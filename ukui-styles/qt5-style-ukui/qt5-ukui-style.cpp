/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2020, Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "qt5-ukui-style.h"

#include "qt5-ukui-style-helper.h"

#include "ukui-style-settings.h"
#include "ukui-tabwidget-default-slide-animator.h"

#include <QStyleOption>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>

#include <QPaintDevice>

#include "tab-widget-animation-helper.h"
#include "scrollbar-animation-helper.h"
#include "button-animation-helper.h"
#include "button-animator.h"
#include "box-animation-helper.h"

#include "animator-iface.h"
#include "animation-helper.h"
#include "shadow-helper.h"

#include "highlight-effect.h"

#include <QIcon>
#include <QStyleOptionViewItem>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QTreeView>
#include <QListView>
#include <QMenu>
#include <QToolButton>
#include <QtPlatformHeaders/QXcbWindowFunctions>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTableView>
#include <QTableWidget>
#include <QTreeView>
#include <QTreeWidget>
#include <QListWidget>
#include <QHeaderView>
#include <QEvent>
#include <QDebug>
#include <QPixmapCache>
#include <QStyleOptionButton>
#include <QStyleOptionMenuItem>
#include <QLabel>
#include <QMessageBox>
#include <QLineEdit>
#include <QApplication>

#include <private/qlineedit_p.h>

#define COMMERCIAL_VERSION true
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

//---copy from qcommonstyle
#include <QTextLayout>

#include <private/qtextengine_p.h>

static QSizeF viewItemTextLayout(QTextLayout &textLayout, int lineWidth, int maxHeight = -1, int *lastVisibleLine = nullptr)
{
    if (lastVisibleLine)
        *lastVisibleLine = -1;
    qreal height = 0;
    qreal widthUsed = 0;
    textLayout.beginLayout();
    int i = 0;
    while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            break;
        line.setLineWidth(lineWidth);
        line.setPosition(QPointF(0, height));
        height += line.height();
        widthUsed = qMax(widthUsed, line.naturalTextWidth());
        // we assume that the height of the next line is the same as the current one
        if (maxHeight > 0 && lastVisibleLine && height + line.height() > maxHeight) {
            const QTextLine nextLine = textLayout.createLine();
            *lastVisibleLine = nextLine.isValid() ? i : -1;
            break;
        }
        ++i;
    }
    textLayout.endLayout();
    return QSizeF(widthUsed, height);
}

QString calculateElidedText(const QString &text, const QTextOption &textOption,
                            const QFont &font, const QRect &textRect, const Qt::Alignment valign,
                            Qt::TextElideMode textElideMode, int flags,
                            bool lastVisibleLineShouldBeElided, QPointF *paintStartPosition)
{
    QTextLayout textLayout(text, font);
    textLayout.setTextOption(textOption);

    // In AlignVCenter mode when more than one line is displayed and the height only allows
    // some of the lines it makes no sense to display those. From a users perspective it makes
    // more sense to see the start of the text instead something inbetween.
    const bool vAlignmentOptimization = paintStartPosition && valign.testFlag(Qt::AlignVCenter);

    int lastVisibleLine = -1;
    viewItemTextLayout(textLayout, textRect.width(), vAlignmentOptimization ? textRect.height() : -1, &lastVisibleLine);

    const QRectF boundingRect = textLayout.boundingRect();
    // don't care about LTR/RTL here, only need the height
    const QRect layoutRect = QStyle::alignedRect(Qt::LayoutDirectionAuto, valign,
                                                 boundingRect.size().toSize(), textRect);

    if (paintStartPosition)
        *paintStartPosition = QPointF(textRect.x(), layoutRect.top());

    QString ret;
    qreal height = 0;
    const int lineCount = textLayout.lineCount();
    for (int i = 0; i < lineCount; ++i) {
        const QTextLine line = textLayout.lineAt(i);
        height += line.height();

        // above visible rect
        if (height + layoutRect.top() <= textRect.top()) {
            if (paintStartPosition)
                paintStartPosition->ry() += line.height();
            continue;
        }

        const int start = line.textStart();
        const int length = line.textLength();
        const bool drawElided = line.naturalTextWidth() > textRect.width();
        bool elideLastVisibleLine = lastVisibleLine == i;
        if (!drawElided && i + 1 < lineCount && lastVisibleLineShouldBeElided) {
            const QTextLine nextLine = textLayout.lineAt(i + 1);
            const int nextHeight = height + nextLine.height() / 2;
            // elide when less than the next half line is visible
            if (nextHeight + layoutRect.top() > textRect.height() + textRect.top())
                elideLastVisibleLine = true;
        }

        QString text = textLayout.text().mid(start, length);
        if (drawElided || elideLastVisibleLine) {
            if (elideLastVisibleLine) {
                if (text.endsWith(QChar::LineSeparator))
                    text.chop(1);
                text += QChar(0x2026);
            }
            const QStackTextEngine engine(text, font);
            ret += engine.elidedText(textElideMode, textRect.width(), flags);

            // no newline for the last line (last visible or real)
            // sometimes drawElided is true but no eliding is done so the text ends
            // with QChar::LineSeparator - don't add another one. This happened with
            // arabic text in the testcase for QTBUG-72805
            if (i < lineCount - 1 &&
                    !ret.endsWith(QChar::LineSeparator))
                ret += QChar::LineSeparator;
        } else {
            ret += text;
        }

        // below visible text, can stop
        if ((height + layoutRect.top() >= textRect.bottom()) ||
                (lastVisibleLine >= 0 && lastVisibleLine == i))
            break;
    }
    return ret;
}

QString toolButtonElideText(const QStyleOptionToolButton *option,
                            const QRect &textRect, int flags)
{
//    if (option->fontMetrics.horizontalAdvance(option->text) <= textRect.width())
    if (option->fontMetrics.width(option->text, -1) <= textRect.width())
        return option->text;

    QString text = option->text;
    text.replace('\n', QChar::LineSeparator);
    QTextOption textOption;
    textOption.setWrapMode(QTextOption::ManualWrap);
    textOption.setTextDirection(option->direction);

    return calculateElidedText(text, textOption,
                               option->font, textRect, Qt::AlignTop,
                               Qt::ElideMiddle, flags,
                               false, nullptr);
}

static QWindow *qt_getWindow(const QWidget *widget)
{
    return widget ? widget->window()->windowHandle() : 0;
}



void Qt5UKUIStyle::viewItemDrawText(QPainter *p, const QStyleOptionViewItem *option, const QRect &rect) const
{
    const QWidget *widget = option->widget;
    const int textMargin = proxy()->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, widget) + 1;

    QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0); // remove width padding
    const bool wrapText = option->features & QStyleOptionViewItem::WrapText;
    QTextOption textOption;
    textOption.setWrapMode(wrapText ? QTextOption::WordWrap : QTextOption::ManualWrap);
    textOption.setTextDirection(option->direction);
    textOption.setAlignment(QStyle::visualAlignment(option->direction, option->displayAlignment));

    QPointF paintPosition;
    const QString newText = calculateElidedText(option->text, textOption,
                                                option->font, textRect, option->displayAlignment,
                                                option->textElideMode, 0,
                                                true, &paintPosition);

    QTextLayout textLayout(newText, option->font);
    textLayout.setTextOption(textOption);
    viewItemTextLayout(textLayout, textRect.width());
    textLayout.draw(p, paintPosition);
}
//---copy from qcommonstyle

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
Qt5UKUIStyle::Qt5UKUIStyle(bool dark, bool useDefault) : QFusionStyle()
#else
Qt5UKUIStyle::Qt5UKUIStyle(bool dark, bool useDefault) : QProxyStyle("fusion")
#endif
{
    m_is_default_style = useDefault;
    m_use_dark_palette = dark;
    m_tab_animation_helper = new TabWidgetAnimationHelper(this);
    m_scrollbar_animation_helper = new ScrollBarAnimationHelper(this);
    m_button_animation_helper = new ButtonAnimationHelper(this);
    m_combobox_animation_helper = new BoxAnimationHelper(this);
    m_shadow_helper = new ShadowHelper(this);
}

const QStringList Qt5UKUIStyle::specialList() const
{
    //use dark palette in default style.
    QStringList l;
    l<<"ukui-menu";
    l<<"ukui-panel";
    l<<"ukui-sidebar";
    l<<"ukui-volume-control-applet-qt";
    l<<"ukui-power-manager-tray";
    l<<"kylin-nm";
    l<<"ukui-flash-disk";
    l<<"indicator-china-weather";
    l<<"kylin-video";
//    l<<"ukui-bluetooth";
    l<<"mktip";
    return l;
}

const QStringList Qt5UKUIStyle::useDefaultPalette() const
{
    QStringList l;
    l<<"kylin-assistant";
    l<<"kybackup";
    l<<"biometric-manager";
    return l;
}

bool Qt5UKUIStyle::shouldBeTransparent(const QWidget *w) const
{
    bool should = false;

    if (w->inherits("QComboBoxPrivateContainer"))
        return true;

    if (w->inherits("QTipLabel"))
        return true;

    return should;
}

bool Qt5UKUIStyle::eventFilter(QObject *obj, QEvent *e)
{
    /*!
      \bug
      There is a bug when use fusion as base style when in qt5 assistant's
      HelperView. ScrollBar will not draw with our overrided function correctly,
      and then it will trigger QEvent::StyleAnimationUpdate. by some how it will let
      HelperView be hidden.
      I eat this event to aviod this bug, but the scrollbar in HelperView still
      display with old fusion style, and the animation will be ineffective.

      I don't know why HelperView didn't use our function drawing scrollbar but fusion, that
      makes me stranged.
      */
    if (e->type() == QEvent::StyleAnimationUpdate) {
        return true;
    }

    if (qobject_cast<QPushButton *>(obj) || qobject_cast<QToolButton *>(obj)) {
        if (e->type() == QEvent::Hide) {
            if (QWidget *w = qobject_cast<QWidget *>(obj)) {
                auto animator = m_button_animation_helper->animator(w);
                if (animator) {
                    animator->stopAnimator("SunKen");
                    animator->stopAnimator("MouseOver");
                    animator->setAnimatorCurrentTime("SunKen", 0);
                    animator->setAnimatorCurrentTime("MouseOver", 0);
                }
            }
        }
        return false;
    }
    return false;
}

/*
 * Note there are some widgets can not be set as transparent one in polish.
 * Because it has been created as a rgb window.
 *
 * To reslove this problem, we have to let attribute be setted more ahead.
 * Some styleHint() methods are called in the early creation of a widget.
 * So we can real set them as alpha widgets.
 */
int Qt5UKUIStyle::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    realSetWindowSurfaceFormatAlpha(widget);
    realSetMenuTypeToMenu(widget);

    switch (hint) {
    case SH_ScrollBar_Transient:
        return false;
    case SH_ItemView_ShowDecorationSelected:
        return true;
    case SH_DialogButtonBox_ButtonsHaveIcons:
        return int(true);
    case SH_DialogButtons_DefaultButton:
        return int(true);
    case SH_UnderlineShortcut:
        return true;
    case SH_ComboBox_Popup:
        return false;
    case SH_Table_GridLineColor:
        return option ? option->palette.mid().color().darker().rgb() : 0;
    case SH_ComboBox_AllowWheelScrolling:
        return int(false);

//    case SH_Button_FocusPolicy:
//        return Qt::TabFocus;

    default:
        break;
    }
    return Style::styleHint(hint, option, widget, returnData);
}
void Qt5UKUIStyle::polish(QPalette &palette){
    palette = standardPalette();
    return Style::polish(palette);
}

QPalette Qt5UKUIStyle::standardPalette() const
{
    auto palette = Style::standardPalette();
    //ukui-light
    QColor  window_bg(245 , 245, 245),
            window_no_bg(237 ,237, 237),
            base_bg(255, 255, 255),
            base_no_bg(245, 245, 245),
            font_bg(0,0,0),
            font_br_bg(255,255,255),
            font_di_bg(0, 0, 0, 76),
            button_bg(230, 230, 230),
            button_di_bg(233,233,233),
            highlight_bg(55,144,250),
            highlight_dis(233, 233, 233),
            tip_bg(248,248,248),
            tip_font(22,22,22),
            alternateBase(248,248,248);

        if (!useDefaultPalette().contains(qAppName()) && (qApp->property("preferDark").toBool() || (m_is_default_style && specialList().contains(qAppName())))) {
        //ukui-dark
        window_bg.setRgb(31, 32, 34);
        window_no_bg.setRgb(26 , 26, 26);
        base_bg.setRgb(18, 18, 18);
        base_no_bg.setRgb(28, 28, 28);
        font_bg.setRgb(255,255,255);
        font_bg.setAlphaF(0.9);
        font_br_bg.setRgb(255,255,255);
        font_br_bg.setAlphaF(0.9);
        font_di_bg.setRgb(255,255,255);
        font_di_bg.setAlphaF(0.3);
        button_bg.setRgb(51, 51, 54);
        button_di_bg.setRgb(46, 46, 48);
        highlight_dis.setRgb(71, 71, 71),
        tip_bg.setRgb(61,61,65);
        tip_font.setRgb(232,232,232);
        alternateBase.setRgb(36,35,40);
    }

    palette.setBrush(QPalette::Active, QPalette::Window, window_bg);
    palette.setBrush(QPalette::Inactive, QPalette::Window, window_bg);
    palette.setBrush(QPalette::Disabled, QPalette::Window, window_no_bg);

    palette.setBrush(QPalette::WindowText,font_bg);
    palette.setBrush(QPalette::Active,QPalette::WindowText,font_bg);
    palette.setBrush(QPalette::Inactive,QPalette::WindowText,font_bg);
    palette.setBrush(QPalette::Disabled,QPalette::WindowText,font_di_bg);

    palette.setBrush(QPalette::Active, QPalette::Base, base_bg);
    palette.setBrush(QPalette::Inactive, QPalette::Base, base_bg);
    palette.setBrush(QPalette::Disabled, QPalette::Base, base_no_bg);

    palette.setBrush(QPalette::Text,font_bg);
    palette.setBrush(QPalette::Active,QPalette::Text,font_bg);
    palette.setBrush(QPalette::Disabled,QPalette::Text,font_di_bg);

    //Cursor placeholder
#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
    palette.setBrush(QPalette::PlaceholderText,font_di_bg);
#endif

    palette.setBrush(QPalette::ToolTipBase,tip_bg);
    palette.setBrush(QPalette::ToolTipText,tip_font);

    palette.setBrush(QPalette::Active, QPalette::Highlight, highlight_bg);
    palette.setBrush(QPalette::Inactive, QPalette::Highlight, highlight_bg);
    palette.setBrush(QPalette::Disabled, QPalette::Highlight, highlight_dis);

    palette.setBrush(QPalette::HighlightedText,font_br_bg);

    palette.setBrush(QPalette::BrightText,font_br_bg);
    palette.setBrush(QPalette::Active,QPalette::BrightText,font_br_bg);
    palette.setBrush(QPalette::Inactive,QPalette::BrightText,font_br_bg);
    palette.setBrush(QPalette::Disabled,QPalette::BrightText,font_di_bg);

    palette.setBrush(QPalette::Active, QPalette::Button, button_bg);
    palette.setBrush(QPalette::Inactive, QPalette::Button, button_bg);
    palette.setBrush(QPalette::Disabled, QPalette::Button, button_di_bg);

    palette.setBrush(QPalette::ButtonText,font_bg);
    palette.setBrush(QPalette::Inactive,QPalette::ButtonText,font_bg);
    palette.setBrush(QPalette::Disabled,QPalette::ButtonText,font_di_bg);

    palette.setBrush(QPalette::AlternateBase,alternateBase);
    palette.setBrush(QPalette::Inactive,QPalette::AlternateBase,alternateBase);
    palette.setBrush(QPalette::Disabled,QPalette::AlternateBase,button_di_bg);

    return palette;
}



QColor Qt5UKUIStyle::button_Click() const
{
    if ((qApp->property("preferDark").toBool() || (m_is_default_style && specialList().contains(qAppName())))) {
        return QColor(43, 43, 46);
    } else {
        return QColor(217, 217, 217);
    }
}



QColor Qt5UKUIStyle::button_Hover() const
{
    if ((qApp->property("preferDark").toBool() || (m_is_default_style && specialList().contains(qAppName())))) {
        return QColor(75, 75, 79);
    } else {
        return QColor(235, 235, 235);
    }
}



QColor Qt5UKUIStyle::button_DisableChecked() const
{
    if ((qApp->property("preferDark").toBool() || (m_is_default_style && specialList().contains(qAppName())))) {
        return QColor(61, 61, 64);
    } else {
        return QColor(224, 224, 224);
    }
}



void Qt5UKUIStyle::polish(QWidget *widget)
{
    Style::polish(widget);

    m_shadow_helper->registerWidget(widget);

    if (qobject_cast<QTabWidget*>(widget)) {
        //FIXME: unpolish, extensiable.
        m_tab_animation_helper->registerWidget(widget);
    }

    if (qobject_cast<QScrollBar*>(widget)) {
        widget->setAttribute(Qt::WA_Hover);
        m_scrollbar_animation_helper->registerWidget(widget);
    }

    if (auto v = qobject_cast<QAbstractItemView *>(widget)) {
        v->viewport()->setAttribute(Qt::WA_Hover);
    }

    if(qobject_cast<QToolButton*>(widget))
    {
        m_button_animation_helper->registerWidget(widget);
    }

    if(qobject_cast<QPushButton*>(widget))
    {
        m_button_animation_helper->registerWidget(widget);
    }

    /*!
      \todo QDateTimeEdit widget affected with calendarPopup() draw QComboBox style or QSpinBox style.
       So temporarily haven't added the QDateTimeEdit animation and style.
      */
    if(qobject_cast<QComboBox*>(widget))
    {
        m_combobox_animation_helper->registerWidget(widget);
        m_button_animation_helper->registerWidget(widget);
    }

    if(qobject_cast<QSpinBox*>(widget) || qobject_cast<QDoubleSpinBox*>(widget))
    {
        m_button_animation_helper->registerWidget(widget);
    }

    if(auto tv = qobject_cast<QTableView*>(widget))
    {
       tv->setShowGrid(false);
       tv->setAlternatingRowColors(true);
    }

    if (widget->inherits("QTipLabel")) {
        auto label = qobject_cast<QLabel *>(widget);
        label->setWordWrap(true);
        label->setScaledContents(true);
    }

    if (qobject_cast<QMessageBox *>(widget)) {
        widget->setAutoFillBackground(true);
        widget->setBackgroundRole(QPalette::Base);
    }

    if (qobject_cast<QLineEdit *>(widget) || qobject_cast<QTabBar *>(widget)) {
        widget->setAttribute(Qt::WA_Hover);
    }

    widget->installEventFilter(this);
}

void Qt5UKUIStyle::unpolish(QWidget *widget)
{
    m_shadow_helper->unregisterWidget(widget);

    widget->removeEventFilter(this);

    if (widget->inherits("QTipLabel")) {
        auto label = qobject_cast<QLabel *>(widget);
        label->setWordWrap(false);
    }

    if (qobject_cast<QTabWidget*>(widget)) {
        m_tab_animation_helper->unregisterWidget(widget);
    }

    if (qobject_cast<QScrollBar*>(widget)) {
        widget->setAttribute(Qt::WA_Hover, false);
        m_scrollbar_animation_helper->unregisterWidget(widget);
    }

    if (auto v = qobject_cast<QAbstractItemView *>(widget)) {
        v->viewport()->setAttribute(Qt::WA_Hover);
    }

    if(qobject_cast<QToolButton*>(widget))
    {
        m_button_animation_helper->unregisterWidget(widget);
    }

    if(qobject_cast<QPushButton*>(widget))
    {
        m_button_animation_helper->unregisterWidget(widget);
    }

    if(qobject_cast<QComboBox*>(widget))
    {
        m_combobox_animation_helper->unregisterWidget(widget);
        m_button_animation_helper->unregisterWidget(widget);
    }

    if(qobject_cast<QSpinBox*>(widget) || qobject_cast<QDoubleSpinBox*>(widget))
    {
        m_button_animation_helper->unregisterWidget(widget);
    }

//    if(auto tv = qobject_cast<QTableView*>(widget))
//    {
//       tv->setShowGrid(true);
//       tv->setAlternatingRowColors(false);
//    }

    if (qobject_cast<QLineEdit *>(widget)) {
        widget->setAttribute(Qt::WA_Hover, false);
    }

    Style::unpolish(widget);
}



QIcon Qt5UKUIStyle::standardIcon(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const
{
    if (QApplication::desktopSettingsAware() && !QIcon::themeName().isEmpty()) {
        switch (standardIcon) {
        case SP_FileDialogListView:
            return QIcon::fromTheme(QLatin1String("view-list-symbolic"));
        case SP_FileDialogDetailedView:
            return QIcon::fromTheme(QLatin1String("view-grid-symbolic"));

        default:
            break;
        }
    }
    return Style::standardIcon(standardIcon, option, widget);
}



static inline uint qt_intensity(uint r, uint g, uint b)
{
    // 30% red, 59% green, 11% blue
    return (77 * r + 150 * g + 28 * b) / 255;
}

/*! \reimp */
QPixmap Qt5UKUIStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const
{
    switch (iconMode) {
    case QIcon::Disabled: {
        QStyleOption tmp = *option;
        tmp.state = State_Enabled;
        QPixmap target = HighLightEffect::ordinaryGeneratePixmap(pixmap, &tmp);
        QImage im = target.toImage().convertToFormat(QImage::Format_ARGB32);

        // Create a colortable based on the background (black -> bg -> white)
        QColor bg = option->palette.color(QPalette::Disabled, QPalette::Window);
        int red = bg.red();
        int green = bg.green();
        int blue = bg.blue();
        uchar reds[256], greens[256], blues[256];
        for (int i=0; i<128; ++i) {
            reds[i]   = uchar((red   * (i<<1)) >> 8);
            greens[i] = uchar((green * (i<<1)) >> 8);
            blues[i]  = uchar((blue  * (i<<1)) >> 8);
        }
        for (int i=0; i<128; ++i) {
            reds[i+128]   = uchar(qMin(red   + (i << 1), 255));
            greens[i+128] = uchar(qMin(green + (i << 1), 255));
            blues[i+128]  = uchar(qMin(blue  + (i << 1), 255));
        }

        int intensity = qt_intensity(red, green, blue);
        const int factor = 191;

        // High intensity colors needs dark shifting in the color table, while
        // low intensity colors needs light shifting. This is to increase the
        // perceived contrast.
        if ((red - factor > green && red - factor > blue)
            || (green - factor > red && green - factor > blue)
            || (blue - factor > red && blue - factor > green))
            intensity = qMin(255, intensity + 91);
//        else if (intensity <= 128)
//            intensity -= 51;

        for (int y=0; y<im.height(); ++y) {
            QRgb *scanLine = (QRgb*)im.scanLine(y);
            for (int x=0; x<im.width(); ++x) {
                QRgb pixel = *scanLine;
                // Calculate color table index, taking intensity adjustment
                // and a magic offset into account.
                uint ci = uint(qGray(pixel)/3 + (130 - intensity / 3));
                *scanLine = qRgba(reds[ci], greens[ci], blues[ci], qAlpha(pixel));
                ++scanLine;
            }
        }

        return QPixmap::fromImage(im);
    }

    default:
        break;
    }

    return Style::generatedIconPixmap(iconMode, pixmap, option);
}



void Qt5UKUIStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case QStyle::PE_PanelMenu:
    case QStyle::PE_FrameMenu:
    {
        return drawMenuPrimitive(option, painter, widget);
    }
    case PE_FrameFocusRect: {
        if (qobject_cast<const QAbstractItemView *>(widget))
            return;
        break;
    }
    case PE_IndicatorBranch: {
        if (qobject_cast<const QTreeView *>(widget)) {
            bool isHover = (option->state & State_MouseOver) && (option->state & ~State_Selected);
            bool isSelected = option->state & State_Selected;
            bool enable = option->state & State_Enabled;
            QColor color = option->palette.color(enable? QPalette::Active: QPalette::Disabled,
                                                 QPalette::Highlight);

            QColor color2 = option->palette.color(enable? QPalette::Active: QPalette::Disabled,
                                                  QPalette::HighlightedText);

            if (isSelected || isHover) {
                if (isHover && !isSelected) {
                    int h = color.hsvHue();
                    //int s = color.hsvSaturation();
                    auto base = option->palette.base().color();
                    int v = color.value();
                    color.setHsv(h, base.lightness(), v, 64);
                }
                painter->fillRect(option->rect, color);
                auto vopt = qstyleoption_cast<const QStyleOptionViewItem *>(option);
                QStyleOptionViewItem tmp = *vopt;

                // for now the style paint arrow with highlight text brush when hover
                // we but don't want to highligh the indicator when hover a view item.
                if (isSelected) {
//                    tmp.state.setFlag(QStyle::State_MouseOver);
                    tmp.state |= QStyle::State_MouseOver;
                } else {
//                    tmp.state.setFlag(QStyle::State_MouseOver, false);
                    tmp.state &= ~QStyle::State_MouseOver;
                }

                tmp.palette.setColor(tmp.palette.currentColorGroup(), QPalette::Highlight, color2);
                return Style::drawPrimitive(PE_IndicatorBranch, &tmp, painter, widget);
            }
            break;
        }
        break;
    }
    case PE_PanelItemViewItem: {
        bool isIconView = false;
        auto opt = qstyleoption_cast<const QStyleOptionViewItem *>(option);
        if (!opt)
            return;
        if (opt) {
            isIconView = (opt->decorationPosition & QStyleOptionViewItem::Top);
        }
        bool isHover = (option->state & State_MouseOver) && (option->state & ~State_Selected);
        bool isSelected = option->state & State_Selected;
        bool enable = option->state & State_Enabled;
        QColor color = option->palette.color(enable? QPalette::Active: QPalette::Disabled,
                                             QPalette::Highlight);

        color.setAlpha(0);
        if (isHover && !isSelected) {
            int h = color.hsvHue();
            //int s = color.hsvSaturation();
            auto base = option->palette.base().color();
            int v = color.value();
            color.setHsv(h, base.lightness(), v, 64);
        }
        if (isSelected) {
            color.setAlpha(255);
        }

        if ((qobject_cast<const QListView *>(widget) || qobject_cast<const QListWidget *>(widget))
                && (opt->decorationPosition != QStyleOptionViewItem::Top)) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::transparent);
            painter->setBrush(color);
            painter->drawRoundedRect(option->rect, 4, 4);
            painter->restore();
            return;
        }

        if (!isIconView)
            painter->fillRect(option->rect, color);
        else {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::transparent);
            painter->setBrush(color);
            painter->drawRoundedRect(option->rect, 6, 6);
            painter->restore();
        }
        return;
    }

    case PE_Frame:{
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(QPen(option->palette.color(QPalette::Normal, QPalette::Dark), 1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(option->rect,6,6);
        painter->restore();
        return;
    }

//    case PE_IndicatorHeaderArrow: //Here is the arrow drawing of the table box

//        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
//            painter->save();
//            painter->setRenderHint(QPainter::Antialiasing,true);
//            painter->setBrush(Qt::NoBrush);
//            if(option->state & State_Enabled){
//                painter->setPen(QPen(option->palette.foreground().color(), 1.1));
//                if (option->state & State_MouseOver) {
//                    painter->setPen(QPen(option->palette.color(QPalette::Highlight), 1.1));
//                }
//            }
//            else {
//                painter->setPen(QPen(option->palette.color(QPalette::Text), 1.1));
//            }
//            QPolygon points(4);
//            //Add 8 to center vertically
//            int x = option->rect.x()+8;
//            int y = option->rect.y()+8;

//            int w = 8;
//            int h =  4;
//            x += (option->rect.width() - w) / 2;
//            y += (option->rect.height() - h) / 2;
//            if (header->sortIndicator & QStyleOptionHeader::SortUp) {
//                points[0] = QPoint(x, y);
//                points[1] = QPoint(x + w / 2, y + h);
//                points[2] = QPoint(x + w / 2, y + h);
//                points[3] = QPoint(x + w, y);
//            } else if (header->sortIndicator & QStyleOptionHeader::SortDown) {
//                points[0] = QPoint(x, y + h);
//                points[1] = QPoint(x + w / 2, y);
//                points[2] = QPoint(x + w / 2, y);
//                points[3] = QPoint(x + w, y + h);
//            }
//            painter->drawLine(points[0],  points[1] );
//            painter->drawLine(points[2],  points[3] );
//            painter->restore();
//            return;
//        }

    case PE_IndicatorHeaderArrow:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option))
        {
            QStyleOption opt = *option;
            opt.state = header->state & State_Enabled ? State_Enabled : State_None;
            opt.rect = header->rect;
            if(header->sortIndicator & QStyleOptionHeader::SortUp)
            {
                proxy()->drawPrimitive(PE_IndicatorArrowUp,&opt,painter,widget);
            }
            else if(header->sortIndicator & QStyleOptionHeader::SortDown)
            {
                proxy()->drawPrimitive(PE_IndicatorArrowDown,&opt,painter,widget);
            }
            return;
        }
        break;
    }

    case PE_PanelItemViewRow:
    {
        if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(option))
        {
            QPalette::ColorGroup cg = (widget ? widget->isEnabled() : (vopt->state & QStyle::State_Enabled))
                    ? QPalette::Normal : QPalette::Disabled;
            if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
                cg = QPalette::Inactive;

            if ((vopt->state & QStyle::State_Selected) &&
                    proxy()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, option, widget))
                painter->fillRect(vopt->rect, vopt->palette.brush(cg, QPalette::Highlight));
            else if (vopt->features & QStyleOptionViewItem::Alternate)
                painter->fillRect(vopt->rect, vopt->palette.brush(cg, QPalette::AlternateBase));

            if(qobject_cast<const QTableView*>(widget) || qobject_cast<const QTableWidget*>(widget))
            {
                const int gridHint = proxy()->styleHint(QStyle::SH_Table_GridLineColor, option, widget);
                const QColor gridColor = static_cast<QRgb>(gridHint);
                painter->save();
                painter->setPen(gridColor);
                painter->setBrush(Qt::NoBrush);
//                painter->drawLine(vopt->rect.topRight(),vopt->rect.bottomRight());
                painter->restore();
            }
           return;
        }
        break;
    }

    case PE_PanelButtonCommand:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            auto animator = m_button_animation_helper->animator(widget);
            const bool enable = button->state & State_Enabled;
            const bool hover = button->state & State_MouseOver;
            const bool sunken = button->state & State_Sunken;
            const bool on = button->state & State_On;
            qreal x_Radius = 4;
            qreal y_Radius = 4;
            bool isWindowButton = false;
            bool isWindowColoseButton = false;
            bool isImportant = false;
            bool useButtonPalette = false;
            if (widget) {
                if (widget->property("isWindowButton").isValid()) {
                    if (widget->property("isWindowButton").toInt() == 0x01)
                        isWindowButton = true;
                    if (widget->property("isWindowButton").toInt() == 0x02)
                        isWindowColoseButton = true;
                }
                if (widget->property("isImportant").isValid())
                    isImportant = widget->property("isImportant").toBool();

                if (widget->property("useButtonPalette").isValid())
                    useButtonPalette = widget->property("useButtonPalette").toBool();

                if (qobject_cast<const QComboBox*>(widget) || qobject_cast<const QLineEdit*>(widget)
                        || qobject_cast<const QAbstractSpinBox*>(widget))
                    useButtonPalette = true;
            }

            if (!enable) {
                if (animator) {
                    animator->stopAnimator("SunKen");
                    animator->stopAnimator("MouseOver");
                }
                painter->save();
                painter->setPen(Qt::NoPen);
                if (on)
                    painter->setBrush(button_DisableChecked());
                else if (button->features & QStyleOptionButton::Flat)
                    painter->setBrush(Qt::NoBrush);
                else
                    painter->setBrush(option->palette.brush(QPalette::Disabled, QPalette::Button));
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawRoundedRect(option->rect, x_Radius, y_Radius);
                painter->restore();
                return;
            }

            if (!(button->features & QStyleOptionButton::Flat)) {
                painter->save();
                painter->setPen(Qt::NoPen);
                if (isImportant)
                    painter->setBrush(option->palette.brush(QPalette::Active, QPalette::Highlight));
                else
                    painter->setBrush(option->palette.brush(QPalette::Active, QPalette::Button));
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawRoundedRect(option->rect, x_Radius, y_Radius);
                painter->restore();
            }

            if (animator == nullptr) {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(Qt::NoPen);
                if (sunken || on) {
                    if (isWindowColoseButton) {
                        painter->setBrush(QColor("#E44C50"));
                    } else if (useDefaultPalette().contains(qAppName())) {
                        QColor color = button->palette.color(QPalette::Base);
                        color.setAlphaF(0.15);
                        painter->setBrush(color);
                    } else {
                        if (isImportant)
                            painter->setBrush(highLight_Click());
                        else if (useButtonPalette || isWindowButton)
                            painter->setBrush(button_Click());
                        else
                            painter->setBrush(highLight_Click());
                    }
                } else if (hover) {
                    if (isWindowColoseButton) {
                        painter->setBrush(QColor("#F86458"));
                    } else if (useDefaultPalette().contains(qAppName())) {
                        QColor color = button->palette.color(QPalette::Base);
                        color.setAlphaF(0.1);
                        painter->setBrush(color);
                    } else {
                        if (isImportant)
                            painter->setBrush(highLight_Hover());
                        else if (useButtonPalette || isWindowButton)
                            painter->setBrush(button_Hover());
                        else
                            painter->setBrush(highLight_Hover());
                    }
                }
                painter->drawRoundedRect(button->rect, x_Radius, y_Radius);
                painter->restore();
                return;
            }

            if (sunken || on || animator->isRunning("SunKen") || animator->value("SunKen") == 1.0) {
                double opacity = animator->value("SunKen").toDouble();
                if (sunken || on) {
                    if (opacity == 0.0) {
                        animator->setAnimatorDirectionForward("SunKen", true);
                        animator->startAnimator("SunKen");
                    }
                } else {
                    if (opacity == 1.0) {
                        animator->setAnimatorDirectionForward("SunKen",false);
                        animator->startAnimator("SunKen");
                    }
                }

                QColor hoverColor, sunkenColor;
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setRenderHint(QPainter::Antialiasing,true);
                if (isWindowColoseButton) {
                    hoverColor = QColor("#F86458");
                    sunkenColor = QColor("#E44C50");
                } else if (useDefaultPalette().contains(qAppName())) {
                    hoverColor = option->palette.color(QPalette::Active, QPalette::Base);
                    hoverColor.setAlphaF(0.1);
                    sunkenColor = option->palette.color(QPalette::Active, QPalette::Base);
                    sunkenColor.setAlphaF(0.15);
                } else {
                    if (isImportant) {
                        hoverColor = highLight_Hover();
                        sunkenColor = highLight_Click();
                    } else if (useButtonPalette || isWindowButton) {
                        hoverColor = button_Hover();
                        sunkenColor = button_Click();
                    } else {
                        hoverColor = highLight_Hover();
                        sunkenColor = highLight_Click();
                    }
                }
                painter->setBrush(mixColor(hoverColor, sunkenColor, opacity));
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawRoundedRect(option->rect, x_Radius, y_Radius);
                painter->restore();
                return;
            }

            if (hover || animator->isRunning("MouseOver")
                    || animator->currentAnimatorTime("MouseOver") == animator->totalAnimationDuration("MouseOver")) {
                double opacity = animator->value("MouseOver").toDouble();
                if (hover) {
                    animator->setAnimatorDirectionForward("MouseOver",true);
                    if (opacity == 0.0) {
                        animator->startAnimator("MouseOver");
                    }
                } else {
                    animator->setAnimatorDirectionForward("MouseOver",false);
                    if (opacity == 1.0) {
                        animator->startAnimator("MouseOver");
                    }
                }

                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(Qt::NoPen);
                painter->setOpacity(opacity);
                if (isWindowColoseButton) {
                    painter->setBrush(QColor("#F86458"));
                } else if (useDefaultPalette().contains(qAppName())) {
                    QColor color = option->palette.color(QPalette::Active, QPalette::Base);
                    color.setAlphaF(0.1);
                    painter->setBrush(color);
                } else {
                    if (isImportant)
                        painter->setBrush(highLight_Hover());
                    else if (useDefaultPalette().contains(qAppName()))
                        painter->setBrush(button_Hover());
                    else if (useButtonPalette || isWindowButton)
                        painter->setBrush(button_Hover());
                    else
                        painter->setBrush(highLight_Hover());
                }
                painter->drawRoundedRect(option->rect, x_Radius, y_Radius);
                painter->restore();
                return;
            }
            return;
        }
        break;
    }

    case PE_PanelTipLabel://UKUI Tip  style: Open ground glass
        {
            if (widget && widget->isEnabled()) {
                QStyleOption opt = *option;

                painter->save();
                painter->setRenderHint(QPainter::Antialiasing);
                QPainterPath rectPath;
                rectPath.addRoundedRect(option->rect.adjusted(+3,+3,-3,-3), 4, 4);

                // Draw a black floor
                QPixmap pixmap(option->rect.size());
                pixmap.fill(Qt::transparent);
                QPainter pixmapPainter(&pixmap);
                pixmapPainter.setRenderHint(QPainter::Antialiasing);
                pixmapPainter.setPen(Qt::transparent);
                pixmapPainter.setBrush(Qt::black);
                pixmapPainter.drawPath(rectPath);
                pixmapPainter.end();

                // Blur the black background
                QImage img = pixmap.toImage();
                qt_blurImage(img, 4, false, false);

                // Dig out the center part
                pixmap = QPixmap::fromImage(img);
                QPainter pixmapPainter2(&pixmap);
                pixmapPainter2.setRenderHint(QPainter::Antialiasing);
                pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
                pixmapPainter2.setPen(Qt::transparent);
                pixmapPainter2.setBrush(Qt::transparent);
                pixmapPainter2.drawPath(rectPath);

                // Shadow rendering
                painter->drawPixmap(option->rect, pixmap, pixmap.rect());

                //This is the beginning of drawing the bottom of the prompt box
                auto color = opt.palette.color(QPalette::ToolTipBase);
                if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
                    auto opacity = UKUIStyleSettings::globalInstance()->get("menuTransparency").toInt()/100.0;
                    color.setAlphaF(opacity);
                }

                if (qApp->property("blurEnable").isValid()) {
                    bool blurEnable = qApp->property("blurEnable").toBool();
                    if (!blurEnable) {
                        color.setAlphaF(1);
                    }
                }

                opt.palette.setColor(QPalette::ToolTipBase, color);
                painter->setRenderHint(QPainter::Antialiasing);
                QPen pen(opt.palette.toolTipBase().color().darker(150), 1);
                pen.setCapStyle(Qt::RoundCap);
                pen.setJoinStyle(Qt::RoundJoin);
                painter->setPen(Qt::transparent);
                painter->setBrush(color);

                QPainterPath path;
                auto region = widget->mask();
                if (region.isEmpty()) {
                    path.addRoundedRect(opt.rect.adjusted(+3,+3,-3,-3), 4, 4);
                } else {
                    path.addRegion(region);
                }

                painter->drawPath(path);
                painter->restore();
                return;
            }
            return Style::drawPrimitive(element, option, painter, widget);
        }

    case PE_FrameStatusBar://UKUI Status style
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(option->palette.color(QPalette::Dark));
        painter->drawRect(option->rect);
        painter->restore();
        return;
    }

    case PE_IndicatorButtonDropDown: //UKUI IndicatorButton  style
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(Qt::NoPen);
        if(widget && widget->isEnabled()){
            if (option->state & State_MouseOver) {
                painter->setBrush(option->palette.color(QPalette::Highlight));
                painter->drawRoundedRect(option->rect.left()-4,option->rect.y(),option->rect.width()+4,option->rect.height(),4,4);
            }
        }
        painter->restore();
        return;
    }

    case PE_PanelButtonTool:
    {
        auto animator = m_button_animation_helper->animator(widget);

        bool isWindowColoseButton = false;
        bool isWindowButton = false;
        bool useButtonPalette = false;
        if (widget) {
            if (widget->property("isWindowButton").isValid()) {
                if (widget->property("isWindowButton").toInt() == 0x01)
                    isWindowButton = true;
                if (widget->property("isWindowButton").toInt() == 0x02)
                    isWindowColoseButton = true;
            }
            if (widget->property("useButtonPalette").isValid())
                useButtonPalette = widget->property("useButtonPalette").toBool();
        }

        const bool enable = option->state & State_Enabled;
        const bool raise = option->state & State_AutoRaise;
        const bool sunken = option->state & State_Sunken;
        const bool hover = option->state & State_MouseOver;
        const bool on = option->state & State_On;

        if (!enable) {
            if (animator) {
                animator->stopAnimator("SunKen");
                animator->stopAnimator("MouseOver");
            }
            painter->save();
            painter->setPen(Qt::NoPen);
            if (on)
                painter->setBrush(option->palette.color(QPalette::Disabled, QPalette::Button));
            else if (raise)
                painter->setBrush(Qt::NoBrush);
            else
                painter->setBrush(option->palette.color(QPalette::Disabled, QPalette::Button));
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->drawRoundedRect(option->rect, 4, 4);
            painter->restore();
            return;
        }

        if (!raise) {
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(option->palette.color(QPalette::Active, QPalette::Button));
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->drawRoundedRect(option->rect, 4, 4);
            painter->restore();
        }

        if (animator == nullptr) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setPen(Qt::NoPen);
            if (sunken || on) {
                if (useDefaultPalette().contains(qAppName())) {
                    QColor color = option->palette.color(QPalette::Active, QPalette::Base);
                    color.setAlphaF(0.15);
                    painter->setBrush(color);
                } else if (isWindowColoseButton) {
                    painter->setBrush(QColor("#E44C50"));
                } else if (isWindowButton || useButtonPalette) {
                    painter->setBrush(button_Click());
                } else {
                    painter->setBrush(highLight_Click());
                }
            } else if (hover) {
                if (useDefaultPalette().contains(qAppName())) {
                    QColor color = option->palette.color(QPalette::Active, QPalette::Base);
                    color.setAlphaF(0.1);
                    painter->setBrush(color);
                } else if (isWindowColoseButton) {
                    painter->setBrush(QColor("#F86458"));
                } else if (isWindowButton || useButtonPalette) {
                    painter->setBrush(button_Hover());
                } else {
                    painter->setBrush(highLight_Hover());
                }
            }
            painter->drawRoundedRect(option->rect, 4, 4);
            painter->restore();
            return;
        }

        if (sunken || on || animator->isRunning("SunKen")
                || animator->currentAnimatorTime("SunKen") == animator->totalAnimationDuration("SunKen")) {
            double opacity = animator->value("SunKen").toDouble();
            if (sunken || on) {
                if (opacity == 0.0) {
                    animator->setAnimatorDirectionForward("SunKen",true);
                    animator->startAnimator("SunKen");
                }
            } else {
                if (animator->currentAnimatorTime("SunKen") == animator->totalAnimationDuration("SunKen")) {
                    animator->setAnimatorDirectionForward("SunKen",false);
                    animator->startAnimator("SunKen");
                }
            }
            QColor hoverColor, sunkenColor;
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setRenderHint(QPainter::Antialiasing,true);
            if (useDefaultPalette().contains(qAppName())) {
                hoverColor = option->palette.color(QPalette::Active, QPalette::Base);
                hoverColor.setAlphaF(0.1);
                sunkenColor = option->palette.color(QPalette::Active, QPalette::Base);
                sunkenColor.setAlphaF(0.15);
            } else if (isWindowColoseButton) {
                hoverColor = QColor("#F86458");
                sunkenColor = QColor("#E44C50");
            } else if (isWindowButton || useButtonPalette){
                hoverColor = button_Hover();
                sunkenColor = button_Click();
            } else {
                hoverColor = highLight_Hover();
                sunkenColor = highLight_Click();
            }
            painter->setBrush(mixColor(hoverColor, sunkenColor, opacity));
            painter->drawRoundedRect(option->rect, 4, 4);
            painter->restore();
            return;
        }

        if (hover || animator->isRunning("MouseOver")
                || animator->currentAnimatorTime("MouseOver") == animator->totalAnimationDuration("MouseOver")) {
            double opacity = animator->value("MouseOver").toDouble();
            if (option->state & State_MouseOver) {
                animator->setAnimatorDirectionForward("MouseOver",true);
                if(opacity == 0.0) {
                    animator->startAnimator("MouseOver");
                }
            } else {
                animator->setAnimatorDirectionForward("MouseOver",false);
                if (opacity == 1.0) {
                    animator->startAnimator("MouseOver");
                }
            }
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->setPen(Qt::NoPen);
            painter->setOpacity(opacity);
            if (useDefaultPalette().contains(qAppName())) {
                QColor color = option->palette.color(QPalette::Active, QPalette::Base);
                color.setAlphaF(0.1);
                painter->setBrush(color);
            } else if (isWindowColoseButton) {
                painter->setBrush(QColor("#F86458"));
            } else if (isWindowButton || useButtonPalette){
                painter->setBrush(button_Hover());
            } else {
                painter->setBrush(highLight_Hover());
            }
            painter->drawRoundedRect(option->rect, 4, 4);
            painter->restore();
            return;
        }
        return;
    }

    case PE_IndicatorTabClose:
    {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setBrush(option->palette.brush(QPalette::Active, QPalette::Text));
        if (option->state & (State_On | State_Sunken)) {
            painter->setOpacity(0.15);
        }
        else if (option->state & State_MouseOver) {
            painter->setOpacity(0.1);
        }
        else
            painter->setOpacity(0.0);
        painter->drawEllipse(option->rect);
        painter->restore();
        QIcon icon = QIcon::fromTheme("window-close-symbolic");
        if (!icon.isNull()) {
            int iconSize = proxy()->pixelMetric(QStyle::PM_SmallIconSize, option, widget);
            QPixmap pixmap = icon.pixmap(QSize(iconSize, iconSize), QIcon::Normal, QIcon::On);
            pixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
            proxy()->drawItemPixmap(painter, option->rect, Qt::AlignCenter, pixmap);
        }
        return;
    }

    case PE_FrameTabBarBase:
    {
        if (const QStyleOptionTabBarBase *tbb = qstyleoption_cast<const QStyleOptionTabBarBase *>(option)) {
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(tbb->palette.brush(QPalette::Active, QPalette::Window));
            painter->drawRect(tbb->tabBarRect);
            painter->restore();
            return;
        }
        break;
    }

    case PE_FrameTabWidget: {
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(twf->palette.brush(QPalette::Active, QPalette::Base));
            painter->drawRect(option->rect.adjusted(0, 0, 0, 0));
            painter->restore();
            return;
        }
        break;
    }

    case PE_FrameGroupBox: //UKUI GroupBox style:
    {
        /*
        * Remove the style of the bounding box according to the design
        */
        //painter->save();
        //painter->setRenderHint(QPainter::Antialiasing,true);
        //painter->setPen(option->palette.color(QPalette::Base));
        //painter->setBrush(option->palette.color(QPalette::Base));
        //painter->drawRoundedRect(option->rect,4,4);
        //painter->restore();
        return;
    }

    case PE_PanelLineEdit://UKUI Text edit style
    {
        if (widget) {
            if (QAction *clearAction = widget->findChild<QAction *>(QLatin1String("_q_qlineeditclearaction"))) {
                QStyleOption subOption = *option;
                QColor color = subOption.palette.color(QPalette::Text);
                color.setAlphaF(1.0);
                subOption.palette.setColor(QPalette::Text, color);
                clearAction->setIcon(QIcon(HighLightEffect::ordinaryGeneratePixmap(clearAction->icon().pixmap(16, 16), &subOption, widget, HighLightEffect::BothDefaultAndHighlit)));
            }
        }

        // Conflict with qspinbox and so on, The widget text cannot use this style
        if (widget) {
            if (widget->parentWidget())
                if (widget->parentWidget()->inherits("QDoubleSpinBox")|| widget->parentWidget()->inherits("QSpinBox")
                        || widget->parentWidget()->inherits("QComboBox") || widget->parentWidget()->inherits("QDateTimeEdit"))
                {
                    return;
                }
        }

        if (const QStyleOptionFrame *f = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            const bool enable = f->state & State_Enabled;
            const bool focus = f->state & State_HasFocus;

            if (!enable) {
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(f->palette.brush(QPalette::Disabled, QPalette::Button));
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawRoundedRect(option->rect, 4, 4);
                painter->restore();
                return;
            }

            if (f->state & State_ReadOnly) {
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(f->palette.brush(QPalette::Active, QPalette::Button));
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawRoundedRect(option->rect, 4, 4);
                painter->restore();
                return;
            }

            if (focus) {
                painter->save();
                painter->setPen(QPen(f->palette.brush(QPalette::Active, QPalette::Highlight),
                                     2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                painter->setBrush(option->palette.brush(QPalette::Active, QPalette::Base));
                painter->drawRoundedRect(option->rect.adjusted(1, 1, -1, -1), 4, 4);
                painter->restore();
            } else {
                QStyleOptionButton button;
                button.state = option->state & ~(State_Sunken | State_On);
                button.rect = option->rect;
                proxy()->drawPrimitive(PE_PanelButtonCommand, &button, painter, widget);

                if (f->state & State_MouseOver) {
                    QRectF rect = f->rect;
                    painter->save();
                    painter->setPen(QPen(f->palette.brush(QPalette::Active, QPalette::Highlight),
                                         1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    painter->setBrush(Qt::NoBrush);
                    painter->drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -0.5), 4, 4);
                    painter->restore();
                }
            }
            return;
        }
        break;
    }

    case PE_IndicatorArrowUp:
    {
        QIcon icon = QIcon::fromTheme("ukui-up-symbolic");
        if(!icon.isNull()) {
            int indWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
            int indHight = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
            QSize iconsize(indWidth, indHight);
            QRect drawRect(option->rect.topLeft(), iconsize);
            drawRect.moveCenter(option->rect.center());
            const bool enable(option->state & State_Enabled);
            QIcon::Mode mode =  enable ? QIcon::Normal : QIcon::Disabled;
            QPixmap pixmap = icon.pixmap(iconsize, mode, QIcon::Off);
            QPixmap target = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
            painter->save();
            painter->drawPixmap(drawRect, target);
            painter->restore();
            return;
        }
        break;
    }

    case PE_IndicatorArrowDown:
    {
        QIcon icon = QIcon::fromTheme("ukui-down-symbolic");
        if(!icon.isNull()) {
            int indWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
            int indHight = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
            QSize iconsize(indWidth, indHight);
            QRect drawRect(option->rect.topLeft(), iconsize);
            drawRect.moveCenter(option->rect.center());
            const bool enable(option->state & State_Enabled);
            QIcon::Mode mode =  enable ? QIcon::Normal : QIcon::Disabled;
            QPixmap pixmap = icon.pixmap(iconsize, mode, QIcon::Off);
            QPixmap target = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
            painter->save();
            painter->drawPixmap(drawRect, target);
            painter->restore();
            return;
        }
        break;
    }

    case PE_IndicatorArrowRight:
    {
        QIcon icon = QIcon::fromTheme("ukui-end-symbolic");
        if(!icon.isNull()) {
            int indWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
            int indHight = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
            QSize iconsize(indWidth, indHight);
            QRect drawRect(option->rect.topLeft(), iconsize);
            drawRect.moveCenter(option->rect.center());
            const bool enable(option->state & State_Enabled);
            QIcon::Mode mode =  enable ? QIcon::Normal : QIcon::Disabled;
            QPixmap pixmap = icon.pixmap(iconsize, mode, QIcon::Off);
            QPixmap target = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
            painter->save();
            painter->drawPixmap(drawRect, target);
            painter->restore();
            return;
        }
        break;
    }

    case PE_IndicatorArrowLeft:
    {
        QIcon icon = QIcon::fromTheme("ukui-start-symbolic");
        if(!icon.isNull()) {
            int indWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
            int indHight = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
            QSize iconsize(indWidth, indHight);
            QRect drawRect(option->rect.topLeft(), iconsize);
            drawRect.moveCenter(option->rect.center());
            const bool enable(option->state & State_Enabled);
            QIcon::Mode mode =  enable ? QIcon::Normal : QIcon::Disabled;
            QPixmap pixmap = icon.pixmap(iconsize, mode, QIcon::Off);
            QPixmap target = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
            painter->save();
            painter->drawPixmap(drawRect, target);
            painter->restore();
            return;
        }
        break;
    }

    case PE_IndicatorRadioButton:
    {
        if (const QStyleOptionButton* radiobutton = qstyleoption_cast<const QStyleOptionButton*>(option)) {
            QRectF rect = radiobutton->rect.adjusted(1, 1, -1, -1);

            bool enable = radiobutton->state & State_Enabled;
            bool mouseOver = radiobutton->state & State_MouseOver;
            bool sunKen = radiobutton->state & State_Sunken;
            bool On = radiobutton->state & State_On;

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);
            if (On) {
                if (enable) {
                    if (sunKen) {
                        painter->setPen(QColor(25, 101, 207));
                        painter->setBrush(highLight_Click());
                    } else if (mouseOver) {
                        painter->setPen(QColor(36, 109, 212));
                        painter->setBrush(highLight_Hover());
                    } else {
                        painter->setPen(QColor(36, 109, 212));
                        painter->setBrush(radiobutton->palette.brush(QPalette::Active, QPalette::Highlight));
                    }
                    painter->drawEllipse(rect);
                    QRectF childRect(rect.x(), rect.y(), 6, 6);
                    childRect.moveCenter(rect.center());
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(radiobutton->palette.brush(QPalette::Active, QPalette::HighlightedText));
                    painter->drawEllipse(childRect);
                } else {
                    if (qApp->property("preferDark").toBool() || (m_is_default_style && specialList().contains(qAppName()))) {
                        painter->setPen(QColor(48, 48, 51));
                        painter->setBrush(QColor(28, 28, 30));
                    } else {
                        painter->setPen(QColor(224, 224, 224));
                        painter->setBrush(QColor(233, 233, 233));
                    }
                    painter->drawEllipse(rect);
                    QRectF childRect(rect.x(), rect.y(), 6, 6);
                    childRect.moveCenter(rect.center());
                    painter->setBrush(radiobutton->palette.brush(QPalette::Disabled, QPalette::ButtonText));
                    painter->drawEllipse(childRect);
                }
            } else {
                if (enable) {
                    if (sunKen) {
                        if (qApp->property("preferDark").toBool() || (m_is_default_style && specialList().contains(qAppName()))) {
                            painter->setPen(QColor(36, 109, 212));
                            painter->setBrush(QColor(6, 35, 97));
                        } else {
                            painter->setPen(QColor(36, 109, 212));
                            painter->setBrush(QColor(179, 221, 255));
                        }
                    } else if (mouseOver) {
                        if (qApp->property("preferDark").toBool() || (m_is_default_style && specialList().contains(qAppName()))) {
                            painter->setPen(QColor(55, 144, 250));
                            painter->setBrush(QColor(9, 53, 153));
                        } else {
                            painter->setPen(QColor(97, 173, 255));
                            painter->setBrush(QColor(219, 240, 255));
                        }
                    } else {
                        if (qApp->property("preferDark").toBool() || (m_is_default_style && specialList().contains(qAppName()))) {
                            painter->setPen(QColor(72, 72, 77));
                            painter->setBrush(QColor(48, 48, 51));
                        } else {
                            painter->setPen(QColor(191, 191, 191));
                            painter->setBrush(option->palette.brush(QPalette::Active, QPalette::Base));
                        }
                    }
                } else {
                    if (qApp->property("preferDark").toBool() || (m_is_default_style && specialList().contains(qAppName()))) {
                        painter->setPen(QColor(48, 48, 51));
                        painter->setBrush(QColor(28, 28, 30));
                    } else {
                        painter->setPen(QColor(224, 224, 224));
                        painter->setBrush(QColor(233, 233, 233));
                    }
                }
                painter->drawEllipse(rect);
            }
            painter->restore();
            return;
        }
        break;
    }

    case PE_IndicatorCheckBox:
    {
        if (const QStyleOptionButton *checkbox = qstyleoption_cast<const QStyleOptionButton*>(option)) {
            bool enable = checkbox->state & State_Enabled;
            bool mouseOver = checkbox->state & State_MouseOver;
            bool sunKen = checkbox->state & State_Sunken;
            bool on = checkbox->state & State_On;
            bool noChange = checkbox->state & State_NoChange;

            QRectF rect = checkbox->rect;
            int width = rect.width();
            int heigth = rect.height();
            int x_Radius = 4;
            int y_Radius = 4;

            QPainterPath path;
            if (on) {
                path.moveTo(width/4 + checkbox->rect.left(), heigth/2 + checkbox->rect.top());
                path.lineTo(width*0.45 + checkbox->rect.left(), heigth*3/4 + checkbox->rect.top());
                path.lineTo(width*3/4 + checkbox->rect.left(), heigth/4 + checkbox->rect.top());
            } else if (noChange){
                path.moveTo(rect.left() + width/4, rect.center().y());
                path.lineTo(rect.right() - width/4 , rect.center().y());
            }

            painter->save();
            painter->setClipRect(rect);
            painter->setRenderHint(QPainter::Antialiasing,true);
            if (enable) {
                if (on | noChange) {
                    if (sunKen) {
                        painter->setPen(QColor(25, 101, 207));
                        painter->setBrush(highLight_Click());
                    } else if (mouseOver) {
                        painter->setPen(QColor(36, 109, 212));
                        painter->setBrush(highLight_Hover());
                    } else {
                        painter->setPen(QColor(36, 109, 212));
                        painter->setBrush(checkbox->palette.brush(QPalette::Active, QPalette::Highlight));
                    }
                    painter->drawRoundedRect(rect, x_Radius, y_Radius);

                    painter->setPen(QPen(checkbox->palette.brush(QPalette::Active, QPalette::HighlightedText), 2,
                                         Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    painter->setBrush(Qt::NoBrush);
                    painter->drawPath(path);
                } else {
                    if (sunKen) {
                        if (qApp->property("preferDark").toBool() || (m_is_default_style && specialList().contains(qAppName()))) {
                            painter->setPen(QColor(36, 109, 212));
                            painter->setBrush(QColor(6, 35, 97));
                        } else {
                            painter->setPen(QColor(36, 109, 212));
                            painter->setBrush(QColor(179, 221, 255));
                        }
                    } else if (mouseOver) {
                        if (qApp->property("preferDark").toBool() || (m_is_default_style && specialList().contains(qAppName()))) {
                            painter->setPen(QColor(55, 144, 250));
                            painter->setBrush(QColor(9, 53, 153));
                        } else {
                            painter->setPen(QColor(97, 173, 255));
                            painter->setBrush(QColor(219, 240, 255));
                        }
                    } else {
                        if (qApp->property("preferDark").toBool() || (m_is_default_style && specialList().contains(qAppName()))) {
                            painter->setPen(QColor(72, 72, 77));
                        } else {
                            painter->setPen(QColor(191, 191, 191));
                        }
                        painter->setBrush(Qt::NoBrush);
                    }
                    painter->drawRoundedRect(rect, x_Radius, y_Radius);
                }
            } else {
                if (qApp->property("preferDark").toBool() || (m_is_default_style && specialList().contains(qAppName()))) {
                    painter->setPen(QColor(48, 48, 51));
                    painter->setBrush(QColor(28, 28, 30));
                } else {
                    painter->setPen(QColor(224, 224, 224));
                    painter->setBrush(QColor(233, 233, 233));
                }
                painter->drawRoundedRect(rect, x_Radius, y_Radius);
                if (on | noChange) {
                    painter->setPen(QPen(checkbox->palette.brush(QPalette::Disabled, QPalette::ButtonText), 2,
                                         Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    painter->setBrush(Qt::NoBrush);
                    painter->drawPath(path);
                }
            }
            painter->restore();
            return;
        }
        break;
    }

    case PE_IndicatorTabTearLeft:
        return;

    case PE_IndicatorTabTearRight:
        return;

    case PE_PanelScrollAreaCorner:
    {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(option->palette.brush(QPalette::Active, QPalette::Base));
        painter->drawRect(option->rect);
        painter->restore();
        return;
    }

    default:
        break;
    }

    return Style::drawPrimitive(element, option, painter, widget);
}

void Qt5UKUIStyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    switch (control) {
    case CC_ScrollBar: {
        if (const QStyleOptionSlider *bar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            if (widget && widget->property("drawScrollBarGroove").isValid() && (!widget->property("drawScrollBarGroove").toBool())) {

            } else {
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(bar->palette.brush(QPalette::Active, QPalette::Base));
                painter->drawRect(bar->rect);
                painter->restore();
            }
            QStyleOptionSlider newScrollbar = *bar;
            newScrollbar.rect = proxy()->subControlRect(control, option, SC_ScrollBarSlider, widget);
//            if (!(bar->activeSubControls & SC_ScrollBarSlider)) {
//                newScrollbar.state &= ~(State_Sunken | State_MouseOver);
//            }
            proxy()->drawControl(CE_ScrollBarSlider, &newScrollbar, painter, widget);
            return;
        }
        break;
    }

    case CC_ComboBox:
    {
        if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            const bool enable = comboBox->state & State_Enabled;
            const bool on = comboBox->state & State_On;
            const bool hover = comboBox->state & State_MouseOver;

            if (!enable) {
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(comboBox->palette.brush(QPalette::Disabled, QPalette::Button));
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->drawRoundedRect(option->rect, 4, 4);
                painter->restore();
                return;
            }

            if (comboBox->editable) {
                painter->save();
                if (comboBox->state & (State_HasFocus | State_On)) {
                    painter->setPen(QPen(comboBox->palette.brush(QPalette::Active, QPalette::Highlight),
                                         2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    painter->setBrush(option->palette.brush(QPalette::Active, QPalette::Base));
                } else {
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(option->palette.brush(QPalette::Active, QPalette::Button));
                }
                painter->drawRoundedRect(option->rect.adjusted(1, 1, -1, -1), 4, 4);
                painter->restore();
            } else {
                QStyleOptionButton button;
                button.state = option->state;
                button.rect = option->rect;
                proxy()->drawPrimitive(PE_PanelButtonCommand, &button, painter, widget);
                if (on) {
                    painter->save();
                    painter->setPen(QPen(comboBox->palette.brush(QPalette::Active, QPalette::Highlight),
                                         2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    painter->setBrush(Qt::NoBrush);
                    painter->drawRoundedRect(option->rect.adjusted(1, 1, -1, -1), 4, 4);
                    painter->restore();
                }
            }

            if (hover) {
                QRectF rect = comboBox->rect;
                painter->save();
                painter->setPen(QPen(comboBox->palette.brush(QPalette::Active, QPalette::Highlight),
                                     1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                painter->setBrush(Qt::NoBrush);
                painter->drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -0.5), 4, 4);
                painter->restore();
            }

            return;
        }
        break;
    }

    case CC_SpinBox:
    {
        if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            bool enable = sb->state & State_Enabled;
            bool hover = enable && (sb->state & State_MouseOver);
            bool up = sb->activeSubControls == SC_SpinBoxUp;
            bool down = sb->activeSubControls == SC_SpinBoxDown;
            bool focus = sb->state & State_HasFocus;
            QRect upRect = proxy()->subControlRect(CC_SpinBox, sb, SC_SpinBoxUp, widget);
            QRect downRect = proxy()->subControlRect(CC_SpinBox, sb, SC_SpinBoxDown, widget);

            QStyleOption upOption, downOption;
            upOption.rect = upRect;
            downOption.rect = downRect;
            if (!enable) {
                upOption.state = State_None;
                downOption.state = State_None;
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(sb->palette.brush(QPalette::Disabled, QPalette::Button));
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawRoundedRect(option->rect, 4, 4);
                painter->restore();
            } else if (sb->stepEnabled == QAbstractSpinBox::StepNone) {
                upOption.state = State_Enabled;
                downOption.state = State_Enabled;
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setPen(Qt::NoPen);
                painter->setBrush(sb->palette.brush(QPalette::Active, QPalette::Button));
                painter->drawRoundedRect(option->rect, 4, 4);
                painter->restore();
            } else {
                upOption.state |= State_Enabled;
                downOption.state |= State_Enabled;
                if (focus) {
                    painter->save();
                    painter->setPen(QPen(sb->palette.brush(QPalette::Active, QPalette::Highlight),
                                         2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    painter->setBrush(option->palette.brush(QPalette::Active, QPalette::Base));
                    painter->drawRoundedRect(option->rect.adjusted(1, 1, -1, -1), 4, 4);
                    painter->restore();
                } else {
                    QStyleOptionButton button;
                    button.state = option->state & ~(State_Sunken | State_On);
                    button.rect = option->rect;
                    proxy()->drawPrimitive(PE_PanelButtonCommand, &button, painter, widget);

                    if (hover) {
                        QRectF rect = sb->rect;
                        painter->save();
                        painter->setPen(QPen(sb->palette.brush(QPalette::Active, QPalette::Highlight),
                                             1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                        painter->setBrush(Qt::NoBrush);
                        painter->drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -0.5), 4, 4);
                        painter->restore();
                    }
                }
            }

            int radius = 4;
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(Qt::NoBrush);
            if (sb->subControls & (SC_SpinBoxUp | SC_SpinBoxDown)) {
                if (sb->stepEnabled & QAbstractSpinBox::StepUpEnabled) {
                    QPainterPath upPath;
                    upPath.moveTo(upRect.left(), upRect.top() + radius);
                    upPath.arcTo(upRect.left(), upRect.top(), radius * 2, radius * 2, 180, -90);
                    upPath.lineTo(upRect.right() - radius, upRect.top());
                    upPath.arcTo(upRect.right() + 1 - 2 * radius, upRect.top(), radius * 2, radius * 2, 90, -90);
                    upPath.lineTo(upRect.right() + 1, upRect.bottom());
                    upPath.lineTo(upRect.left(), upRect.bottom());
                    upPath.lineTo(upRect.left(), upRect.top() - radius);
                    upOption.state |= State_Enabled;
                    if (up) {
                        painter->setBrush(sb->palette.brush(QPalette::Active, QPalette::Highlight));
                        upOption.state |= State_MouseOver;
                        if (option->state & State_Sunken) {
                            painter->setBrush(highLight_Click());
                            upOption.state |= State_Sunken;
                        }
                        painter->drawPath(upPath);
                    }
                } else {
                    upOption.state = State_None;
                }

                if (sb->stepEnabled & QAbstractSpinBox::StepDownEnabled) {
                    QPainterPath downPath;
                    downPath.moveTo(downRect.left(), downRect.bottom() - radius);
                    downPath.arcTo(downRect.left(), downRect.bottom() - radius * 2, radius * 2, radius * 2, 180, 90);
                    downPath.lineTo(downRect.right() - radius, downRect.bottom());
                    downPath.arcTo(downRect.right() + 1 - radius * 2, downRect.bottom() - radius * 2, radius * 2, radius * 2, 270, 90);
                    downPath.lineTo(downRect.right() + 1, downRect.top());
                    downPath.lineTo(downRect.left(), downRect.top());
                    downPath.lineTo(downRect.left(), downRect.bottom() - radius);
                    downOption.state |= State_Enabled;
                    painter->setBrush(Qt::NoBrush);
                    painter->setBrush(sb->palette.brush(QPalette::Active, QPalette::Highlight));
                    if (down) {
                        painter->setBrush(sb->palette.brush(QPalette::Active, QPalette::Highlight));
                        downOption.state |= State_MouseOver;
                        if (option->state & State_Sunken) {
                            painter->setBrush(highLight_Click());
                            downOption.state |= State_Sunken;
                        }
                        painter->drawPath(downPath);
                    }
                } else {
                    downOption.state = State_None;
                }

                if (sb->buttonSymbols == QAbstractSpinBox::PlusMinus) {

                } else if (sb->buttonSymbols == QAbstractSpinBox::UpDownArrows) {
                    proxy()->drawPrimitive(PE_IndicatorArrowUp, &upOption, painter, widget);
                    proxy()->drawPrimitive(PE_IndicatorArrowDown, &downOption, painter, widget);
                }
            }
            painter->restore();
            return;
        }
        break;
    }

//    case CC_Slider :
//        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
//            //Size and location of each rectangle used
//            QRectF rect = option->rect;
//            QRectF rectHandle = proxy()->subControlRect(CC_Slider, option, SC_SliderHandle, widget);
//            QRectF rectSliderTickmarks = proxy()->subControlRect(CC_Slider, option, SC_SliderTickmarks, widget);
//            QRect rectGroove = proxy()->subControlRect(CC_Slider, option, SC_SliderGroove, widget);
//            const bool enable = option->state & State_Enabled;
//            const QColor highlight = option->palette.color(QPalette::Highlight);
//            const QColor dis_highlight = option->palette.color(QPalette::Disabled,QPalette::Button).darker(120);
//            QPen pen;
//            //Drawing chute (line)
//            if (option->subControls & SC_SliderGroove) {
//                pen.setStyle(Qt::CustomDashLine);
//                QVector<qreal> dashes;
//                //qreal space = 1.3;
//                qreal space = 0;
//                dashes << 0.1 << space;
//                // dashes << -0.1 << space;
//                pen.setDashPattern(dashes);
//                pen.setWidthF(3);
//                pen.setColor(enable ? highlight.lighter() : dis_highlight);
//                painter->setPen(pen);
//                painter->setRenderHint(QPainter::Antialiasing);

//                if (slider->orientation == Qt::Horizontal) {
//                    painter->drawLine(QPointF(rectGroove.left(), rectHandle.center().y()), QPointF(rectHandle.left(), rectHandle.center().y()));
//                    pen.setColor(option->palette.color(enable ? QPalette::Active : QPalette::Disabled,QPalette::Button));
//                    painter->setPen(pen);
//                    painter->drawLine(QPointF(rectGroove.right(), rectHandle.center().y()), QPointF(rectHandle.right(), rectHandle.center().y()));
//                } else {
//                    painter->drawLine(QPointF(rectGroove.center().x(), rectGroove.bottom()), QPointF(rectGroove.center().x(),  rectHandle.bottom()));
//                    pen.setColor(option->palette.color(enable ? QPalette::Active : QPalette::Disabled,QPalette::Button));
//                    painter->setPen(pen);
//                    painter->drawLine(QPointF(rectGroove.center().x(),  rectGroove.top()), QPointF(rectGroove.center().x(),  rectHandle.top()));
//                }
//            }

//            //Painting slider
//            if (option->subControls & SC_SliderHandle) {
//                pen.setStyle(Qt::SolidLine);
//                painter->setPen(Qt::NoPen);
//                painter->setBrush(enable ? highlight : dis_highlight);
//                painter->drawEllipse(rectHandle);
//            }

//            //Drawing scale
//            if ((option->subControls & SC_SliderTickmarks) && slider->tickInterval) {
//                painter->setPen(option->palette.color(enable ? QPalette::Active : QPalette::Disabled,QPalette::WindowText));
//                int available = proxy()->pixelMetric(PM_SliderSpaceAvailable, slider, widget);
//                int interval = slider->tickInterval;
//                //int tickSize = proxy()->pixelMetric(PM_SliderTickmarkOffset, opt, w);
//                //int ticks = slider->tickPosition;
//                int v = slider->minimum;
//                int len = proxy()->pixelMetric(PM_SliderLength, slider, widget);
//                while (v <= slider->maximum + 1) {
//                    const int v_ = qMin(v, slider->maximum);
//                    int pos = sliderPositionFromValue(slider->minimum, slider->maximum, v_, available) + len / 2;

//                    if (slider->orientation == Qt::Horizontal) {
//                        if (slider->tickPosition == QSlider::TicksBothSides) {
//                            painter->drawLine(pos, int(rect.top()), pos, int(rectHandle.top()));
//                            painter->drawLine(pos, int(rect.bottom()), pos, int(rectHandle.bottom()));
//                        } else {
//                            painter->drawLine(pos, int(rectSliderTickmarks.top()), pos, int(rectSliderTickmarks.bottom()));
//                        }
//                    } else {
//                        if (slider->tickPosition == QSlider::TicksBothSides) {
//                            painter->drawLine(int(rect.left()), pos, int(rectHandle.left()), pos);
//                            painter->drawLine(int(rect.right()), pos, int(rectHandle.right()), pos);
//                        } else {
//                            painter->drawLine(int(rectSliderTickmarks.left()), pos, int(rectSliderTickmarks.right()), pos);
//                        }
//                    }
//                    // in the case where maximum is max int
//                    int nextInterval = v + interval;
//                    if (nextInterval < v)
//                        break;
//                    v = nextInterval;
//                }
//            }
//            return;
//        }

    case CC_Slider:
    {
        if(const QStyleOptionSlider* slider = qstyleoption_cast<const QStyleOptionSlider*>(option))
        {
            QRect groove = proxy()->subControlRect(CC_Slider, option, SC_SliderGroove, widget);
            QRect handle = proxy()->subControlRect(CC_Slider, option, SC_SliderHandle, widget);
            const bool horizontal = slider->orientation == Qt::Horizontal;
            const bool enable = slider->state & State_Enabled;
            const bool ticksAbove = slider->tickPosition & QSlider::TicksAbove;
            const bool ticksBelow = slider->tickPosition & QSlider::TicksBelow;
            const QColor button = slider->palette.color(enable ? QPalette::Active : QPalette::Disabled,QPalette::Button);
            QColor hightligt = slider->palette.color(QPalette::Highlight).lighter();
            if(!enable)
            {
                hightligt = slider->palette.color(QPalette::Disabled,QPalette::Button).darker(120);
            }
            int Slider_GrooveMargin = 4;
            if (horizontal) {
                groove.adjust(0, (groove.height() - Slider_GrooveMargin) / 2, 0, -(groove.height() - Slider_GrooveMargin) /2);
            } else {
                groove.adjust((groove.width() - Slider_GrooveMargin) / 2, 0, -(groove.width() - Slider_GrooveMargin) / 2, 0);
            }

            if(slider->subControls & SC_SliderGroove)
            {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);
                //draw blue groove highlight
                QRect hlRect;
                if(horizontal)
                {
                   if(slider->upsideDown)
                      hlRect.setRect(handle.right(),groove.top(),groove.right() - handle.center().x(),groove.height());
                   else
                      hlRect.setRect(groove.left(),groove.top(),handle.center().x() - groove.left(),groove.height());
                }
                else
                {
                   hlRect.setRect(groove.left(), handle.center().y(), groove.width(), groove.bottom() - handle.center().y());
                }
                painter->setPen(button);
                painter->setBrush(button);
                painter->drawRoundedRect(groove, 2, 2);
                painter->setPen(hightligt);
                painter->setBrush(hightligt);
                painter->drawRoundedRect(hlRect,2,2);
                painter->restore();
            }

            int len = proxy()->pixelMetric(PM_SliderLength, slider, widget);
            int sliderPos = QStyle::sliderPositionFromValue(slider->minimum, slider->maximum,slider->sliderPosition,
                                                    (horizontal ? slider->rect.width(): slider->rect.height()) - len,slider->upsideDown);
            if(slider->subControls & SC_SliderTickmarks)
            {
                int tickOffset = proxy()->pixelMetric(PM_SliderTickmarkOffset, option, widget);
                int tickSize = tickOffset;
//                int available = proxy()->pixelMetric(PM_SliderSpaceAvailable, option, widget);
                int interval = slider->tickInterval;
                if (interval <= 0)
                {
//                    interval = slider->singleStep;
//                    if (QStyle::sliderPositionFromValue(slider->minimum, slider->maximum, interval,available)
//                              - QStyle::sliderPositionFromValue(slider->minimum, slider->maximum,0, available) < 3)
                        interval = slider->pageStep;
                }
                if (interval <= 0)
                    interval = 1;
                int v = slider->minimum;
                int len = proxy()->pixelMetric(PM_SliderLength, slider, widget);
                while (v <= slider->maximum + 1)
                {
                   if (v == slider->maximum + 1 && interval == 1)
                       break;
                   const int v_ = qMin(v, slider->maximum);
                   int pos = QStyle::sliderPositionFromValue(slider->minimum, slider->maximum,v_, (horizontal ? slider->rect.width():
                                                     slider->rect.height()) - len,slider->upsideDown) + len / 2;
                   //int extra = 2 - ((v_ == slider->minimum || v_ == slider->maximum) ? 1 : 0);

                   painter->save();
                   painter->setPen(QPen(slider->palette.color(enable ? QPalette::Active : QPalette::Disabled,QPalette::WindowText),1));
                   painter->setBrush(Qt::NoBrush);
                   if (horizontal)
                   {
                       if(pos <= sliderPos + handle.width()/2)
                       {
                           painter->setPen(QPen(hightligt,1));
                       }
                       if (ticksAbove)
                       {
                           painter->drawLine(pos, groove.top() - tickOffset,pos, groove.top() - tickOffset - tickSize);
                       }
                       if (ticksBelow)
                       {
                           painter->drawLine(pos, groove.bottom() + tickOffset,pos, groove.bottom() + tickOffset + tickSize);
                       }
                   }
                   else
                   {
                       if(pos >= sliderPos + handle.width()/2)
                       {
                           painter->setPen(QPen(hightligt,1));
                       }
                       if (ticksAbove)
                       {
                           painter->drawLine(groove.left() - tickOffset, pos,groove.left() - tickOffset - tickSize, pos);
                       }
                       if (ticksBelow)
                       {
                           painter->drawLine(groove.right() + tickOffset, pos,groove.right() + tickOffset + tickSize, pos);
                       }
                   }
                   // in the case where maximum is max int
                   int nextInterval = v + interval;
                   if (nextInterval < v)
                       break;
                   v = nextInterval;
                   painter->restore();
                }
            }

            if(slider->subControls & SC_SliderHandle)
            {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(Qt::NoPen);
                painter->setBrush(slider->palette.color(QPalette::Highlight).lighter(125));
                if(slider->state & State_Sunken && slider->activeSubControls == SC_SliderHandle)
                {
                    painter->setBrush(slider->palette.color(QPalette::Highlight));
                }
                else if(slider->state & State_MouseOver && slider->activeSubControls == SC_SliderHandle)
                {
                    painter->setBrush(slider->palette.color(QPalette::Highlight).lighter());
                }
                if(!enable)
                {
                    painter->setBrush(hightligt);
                }
                painter->drawEllipse(handle);
                painter->restore();
            }
            return;
        }
        break;
    }

    case CC_Dial:
    {
        if(const QStyleOptionSlider* dial = qstyleoption_cast<const QStyleOptionSlider*>(option))
        {
            const bool enable = dial->state & State_Enabled;
            int tickOffset = proxy()->pixelMetric(PM_SliderTickmarkOffset, option, widget);
            int r = qMin(dial->rect.width(), dial->rect.height())/2;
            int r_ = r - tickOffset*2;
            QRect br(dial->rect.topLeft(), QSize(r_*2, r_*2));
            br.moveCenter(dial->rect.center());
            QColor highlight = enable ? dial->palette.color(QPalette::Highlight) : dial->palette.color(QPalette::Disabled, QPalette::Button).darker(120);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->setPen(dial->palette.color(enable ? QPalette::Active : QPalette::Disabled, QPalette::Button));
            painter->setBrush(dial->palette.color(enable ? QPalette::Active : QPalette::Disabled, QPalette::Button));
            painter->drawEllipse(br);

            int tickHandle = proxy()->pixelMetric(PM_SliderControlThickness, option, widget);
            int handle_r = tickHandle < r_/2 ? tickHandle : r_/2;
            int Circle_r = (r_ + handle_r)/2;
            qreal fist = QT5UKUISTYLE_H::calcRadialPos(dial, dial->minimum);
            qreal dp = QT5UKUISTYLE_H::calcRadialPos(dial, dial->sliderPosition);
            qreal end = QT5UKUISTYLE_H::calcRadialPos(dial, dial->maximum);
            QRectF dialRect(dial->rect.topLeft(), QSizeF(handle_r, handle_r));
            dialRect.moveCenter(dial->rect.center() + QPointF(Circle_r * qCos(dp),-Circle_r * qSin(dp)));
            QRectF GrooveRect(dial->rect.topLeft(), QSizeF(Circle_r*2, Circle_r*2));
            GrooveRect.moveCenter(dial->rect.center());

            painter->setPen(QPen(dial->palette.color(enable ? QPalette::Active : QPalette::Disabled,QPalette::Base),
                                 handle_r/4, Qt::SolidLine, Qt::RoundCap));
            painter->setBrush(Qt::NoBrush);
            painter->drawArc(GrooveRect, fist * 16 * 180 / M_PI, (end - fist) * 16 * 180 / M_PI);
            painter->setPen(QPen(highlight,handle_r/4, Qt::SolidLine, Qt::RoundCap));
            painter->drawArc(GrooveRect, fist * 16 * 180 / M_PI, (dp - fist) * 16 * 180 / M_PI);

            painter->setPen(highlight);
            painter->setBrush(highlight);
            painter->drawEllipse(dialRect);

            if(dial->subControls & SC_DialTickmarks)
            {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(dial->palette.color(enable ? QPalette::Active : QPalette::Disabled, QPalette::WindowText));
                painter->setBrush(dial->palette.color(enable ? QPalette::Active : QPalette::Disabled, QPalette::WindowText));
                painter->drawLines(QT5UKUISTYLE_H::calcLines(dial, tickOffset*2));
                painter->restore();
            }
            painter->restore();
            return;
        }
        break;
    }

    case CC_ToolButton:
    {
        if (qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            proxy()->drawPrimitive(PE_PanelButtonTool, option, painter, widget);
            proxy()->drawControl(CE_ToolButtonLabel, option, painter, widget);
//            if (option->state & State_HasFocus) {
//                painter->save();
//                painter->setPen(QPen(highLight_Click(), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
//                painter->setBrush(Qt::NoBrush);
//                painter->setRenderHint(QPainter::Antialiasing, true);
//                int x_Radius = 4;
//                int y_Radius = 4;
//                painter->drawRoundedRect(option->rect.adjusted(1, 1, -1, -1), x_Radius, y_Radius);
//                painter->restore();
//            }
            return;
        }
        break;
    }

    case CC_GroupBox: //UKUI GroupBox style
    {
        painter->save();
        if (const QStyleOptionGroupBox *groupBox = qstyleoption_cast<const QStyleOptionGroupBox *>(option)) {
            // Draw frame
            QRect textRect = proxy()->subControlRect(CC_GroupBox, option, SC_GroupBoxLabel, widget);
            QRect checkBoxRect = proxy()->subControlRect(CC_GroupBox, option, SC_GroupBoxCheckBox, widget);
            //新加
            QRect groupContents = proxy()->subControlRect(CC_GroupBox, option, SC_GroupBoxContents, widget);

            if (groupBox->subControls & QStyle::SC_GroupBoxFrame) {
                QStyleOptionFrame frame;
                frame.QStyleOption::operator=(*groupBox);
                frame.features = groupBox->features;
                frame.lineWidth = groupBox->lineWidth;
                frame.midLineWidth = groupBox->midLineWidth;
                frame.rect = proxy()->subControlRect(CC_GroupBox, option, SC_GroupBoxFrame, widget);
                proxy()->drawPrimitive(PE_FrameGroupBox, &frame, painter, widget);
            }

            // Draw title
            if ((groupBox->subControls & QStyle::SC_GroupBoxLabel) && !groupBox->text.isEmpty()) {
                // groupBox->textColor gets the incorrect palette here
                painter->setPen(QPen(option->palette.windowText(), 1));
                int alignment = int(groupBox->textAlignment);
                if (!proxy()->styleHint(QStyle::SH_UnderlineShortcut, option, widget))
                    alignment |= Qt::TextHideMnemonic;

                proxy()->drawItemText(painter, textRect,  Qt::TextShowMnemonic | Qt::AlignLeft | alignment,
                                      groupBox->palette, groupBox->state & State_Enabled, groupBox->text, QPalette::NoRole);

                if (groupBox->state & State_HasFocus) {
                    QStyleOptionFocusRect fropt;
                    fropt.QStyleOption::operator=(*groupBox);
                    fropt.rect = textRect.adjusted(-2, -1, 2, 1);
                    proxy()->drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
                }
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(Qt::NoPen);
                painter->setBrush(option->palette.color(QPalette::Base));
                painter->drawRoundedRect(groupContents,4,4);
                painter->restore();

            }

            // Draw checkbox
            if (groupBox->subControls & SC_GroupBoxCheckBox) {
                QStyleOptionButton box;
                box.QStyleOption::operator=(*groupBox);
                box.rect = checkBoxRect;
                proxy()->drawPrimitive(PE_IndicatorCheckBox, &box, painter, widget);
            }
        }
        painter->restore();
        return;
        break;

    }

    default:        return Style::drawComplexControl(control, option, painter, widget);
    }
}

void Qt5UKUIStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case CE_ItemViewItem: {
        auto p = painter;
        auto opt = option;
        if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
            p->save();
            if (p->clipPath().isEmpty())
                p->setClipRect(opt->rect);

            QRect checkRect = proxy()->subElementRect(SE_ItemViewItemCheckIndicator, vopt, widget);
            QRect iconRect = proxy()->subElementRect(SE_ItemViewItemDecoration, vopt, widget);
            QRect textRect = proxy()->subElementRect(SE_ItemViewItemText, vopt, widget);

            // draw the background
            proxy()->drawPrimitive(PE_PanelItemViewItem, opt, p, widget);

            // draw the check mark
            if (vopt->features & QStyleOptionViewItem::HasCheckIndicator) {
                QStyleOptionViewItem option(*vopt);
                option.rect = checkRect;
                option.state = option.state & ~QStyle::State_HasFocus;

                switch (vopt->checkState) {
                case Qt::Unchecked:
                    option.state |= QStyle::State_Off;
                    break;
                case Qt::PartiallyChecked:
                    option.state |= QStyle::State_NoChange;
                    break;
                case Qt::Checked:
                    option.state |= QStyle::State_On;
                    break;
                default:
                    break;
                }
                proxy()->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &option, p, widget);
            }

            // draw the icon
            QIcon::Mode mode = QIcon::Normal;
            if (!(vopt->state & QStyle::State_Enabled))
                mode = QIcon::Disabled;
            else if (vopt->state & QStyle::State_Selected)
                mode = QIcon::Selected;
            QIcon::State state = vopt->state & QStyle::State_Open ? QIcon::On : QIcon::Off;
            //vopt->icon.paint(p, iconRect, vopt->decorationAlignment, mode, state);
            auto pixmap = vopt->icon.pixmap(vopt->decorationSize,
                                            mode,
                                            state);

            QStyle::drawItemPixmap(painter, iconRect, vopt->decorationAlignment, HighLightEffect::generatePixmap(pixmap, vopt, widget));
//            auto target = pixmap;
//            if (widget) {
//                if (widget->property("useIconHighlightEffect").isValid()) {
//                    qDebug()<<widget->property("useIconHighlightEffect").toInt();
//                    HighLightEffect::HighLightMode needHandel = HighLightEffect::HighLightMode(widget->property("useIconHighlightEffect").toInt());
//                    if (needHandel) {
//                        HighLightEffect::EffectMode mode = HighLightEffect::HighlightOnly;
//                        if (widget->property("iconHighlightEffectMode").isValid()) {
//                            auto var = widget->property("iconHighlightEffectMode");
//                            mode = qvariant_cast<HighLightEffect::EffectMode>(var);
//                            target = HighLightEffect::generatePixmap(pixmap, vopt, widget, false, mode);
//                        } else {
//                            target = HighLightEffect::generatePixmap(pixmap, vopt, widget, false);
//                        }
//                    }
//                }
//            }
//            QStyle::drawItemPixmap(painter, iconRect, vopt->decorationAlignment, target);

            // draw the text
            if (!vopt->text.isEmpty()) {
                QPalette::ColorGroup cg = vopt->state & QStyle::State_Enabled
                        ? QPalette::Normal : QPalette::Disabled;
                if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
                    cg = QPalette::Inactive;

                if (vopt->state & QStyle::State_Selected) {
                    p->setPen(vopt->palette.color(cg, QPalette::HighlightedText));
                } else {
                    p->setPen(vopt->palette.color(cg, QPalette::Text));
                }
                if (vopt->state & QStyle::State_Editing) {
                    p->setPen(vopt->palette.color(cg, QPalette::Text));
                    p->drawRect(textRect.adjusted(0, 0, -1, -1));
                }

                viewItemDrawText(p, vopt, textRect);
            }

            // draw the focus rect
            /*
            if (vopt->state & QStyle::State_HasFocus) {
                QStyleOptionFocusRect o;
                o.QStyleOption::operator=(*vopt);
                o.rect = proxy()->subElementRect(SE_ItemViewItemFocusRect, vopt, widget);
                o.state |= QStyle::State_KeyboardFocusChange;
                o.state |= QStyle::State_Item;
                QPalette::ColorGroup cg = (vopt->state & QStyle::State_Enabled)
                        ? QPalette::Normal : QPalette::Disabled;
                o.backgroundColor = vopt->palette.color(cg, (vopt->state & QStyle::State_Selected)
                                                        ? QPalette::Highlight : QPalette::Window);
                proxy()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, p, widget);
            }
            */

            p->restore();
            return;
        }
        break;
    }
//    case CE_ProgressBarGroove:{
//        const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(option);
//        if (!bar)
//            return;
//        painter->save();
//        painter->setRenderHint(QPainter::Antialiasing, true);
//        painter->setPen(Qt::NoPen);
//        painter->setBrush(option->palette.button());
//        int adjustMarignx2 = qMin(option->rect.width(), option->rect.height()) - 16;
//        bool needAdjustBarWidth = adjustMarignx2 > 0;
//        bool vertical = (bar->orientation == Qt::Vertical);
//        auto progressBarGroveRect = option->rect;
//        if (!vertical) {
//            if (needAdjustBarWidth) {
//                progressBarGroveRect.adjust(0, adjustMarignx2/2, 0, -adjustMarignx2/2);
//            }
//        } else {
//            if (needAdjustBarWidth) {
//                progressBarGroveRect.adjust(adjustMarignx2/2, 0, -adjustMarignx2/2, 0);
//            }
//        }
//        painter->drawRoundedRect(progressBarGroveRect, 4, 4);
//        painter->restore();
//        return;
//    }
//    case CE_ProgressBarContents:{
//        painter->save();
//        painter->setRenderHint(QPainter::Antialiasing, true);
//        if (const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
//            //Judgment status
//            painter->setPen(Qt::NoPen);
//            bool vertical = false;
//            bool inverted = false;
//            bool indeterminate = (bar->minimum == 0 && bar->maximum == 0);
//            bool complete = bar->progress == bar->maximum;

//            // Get extra style options if version 2
//            vertical = (bar->orientation == Qt::Vertical);
//            inverted = bar->invertedAppearance;

//            // If the orientation is vertical, we use a transform to rotate
//            // the progress bar 90 degrees clockwise.  This way we can use the
//            // same rendering code for both orientations.
//            int maxWidth = vertical? option->rect.height(): option->rect.width();
//            const auto progress = qMax(bar->progress, bar->minimum); // workaround for bug in QProgressBar
//            const auto totalSteps = qMax(Q_INT64_C(1), qint64(bar->maximum) - bar->minimum);
//            const auto progressSteps = qint64(progress) - bar->minimum;
//            const auto progressBarWidth = progressSteps * maxWidth / totalSteps;
//            int width = indeterminate ? maxWidth : progressBarWidth;

//            QRect progressBar;
//            painter->setPen(Qt::NoPen);


//            int adjustMarignx2 = qMin(option->rect.width(), option->rect.height()) - 16;
//            bool needAdjustBarWidth = adjustMarignx2 > 0;

//            //Positioning
//            progressBar = option->rect;
//            if (!indeterminate) {
//                if (!inverted) {
//                    if (!vertical) {
//                        //progressBar.setRect(option->rect.left()-1, option->rect.top(), width-3,option->rect.height()-7);
//                        progressBar.setRight(width);
//                        if (needAdjustBarWidth) {
//                            progressBar.adjust(0, adjustMarignx2/2, 0, -adjustMarignx2/2);
//                        }
//                    } else {
//                        progressBar.setTop(maxWidth - width);
//                        if (needAdjustBarWidth) {
//                            progressBar.adjust(adjustMarignx2/2, 0, -adjustMarignx2/2, 0);
//                        }
//                    }
//                } else {
//                    if (!vertical) {
//                        progressBar = option->rect;
//                        progressBar.setLeft(maxWidth - width);
//                        if (needAdjustBarWidth) {
//                            progressBar.adjust(0, adjustMarignx2/2, 0, -adjustMarignx2/2);
//                        }
//                    } else {
//                        progressBar = option->rect;
//                        progressBar.setBottom(width);
//                        if (needAdjustBarWidth) {
//                            progressBar.adjust(adjustMarignx2/2, 0, -adjustMarignx2/2, 0);
//                        }
//                    }
//                }
//            }

//            //Brush color
//            if (!indeterminate && width > 0) {
//                painter->save();
//                if (!vertical) {
//                    painter->setPen(Qt::NoPen);

//                    QColor startcolor = option->palette.highlight().color();
//                    QColor endcolor = option->palette.highlight().color().darker(200);
//                    QLinearGradient linearGradient(QPoint(option->rect.bottomRight().x(), option->rect.bottomRight().y()),
//                                                   QPoint(option->rect.bottomLeft().x(), option->rect.bottomLeft().y()));
//                    linearGradient.setColorAt(1,startcolor);
//                    linearGradient.setColorAt(0,endcolor);
//                    painter->setBrush(QBrush(linearGradient));

//                    // painter->setBrush(option->palette.highlight().color());

//                    if (!complete && !indeterminate)
//                        painter->setClipRect(progressBar.adjusted(0, 0, 0, 0));

//                    painter->drawRoundedRect(progressBar, 4, 4);
//                } else {
//                    painter->setPen(Qt::NoPen);

//                    QColor startcolor = option->palette.highlight().color();
//                    QColor endcolor = option->palette.highlight().color().darker(200);
//                    QLinearGradient linearGradient(QPoint(option->rect.topLeft()),
//                                                   QPoint(option->rect.bottomLeft()));
//                    linearGradient.setColorAt(0,startcolor);
//                    linearGradient.setColorAt(1,endcolor);
//                    painter->setBrush(QBrush(linearGradient));

//                    if (!complete && !indeterminate)
//                        painter->setClipRect(progressBar.adjusted(0, 0, 0, 0));

//                    painter->drawRoundedRect(progressBar, 4, 4);
//                }
//                painter->restore();
//            } else {
//                //FIXME: implement waiting animation.
//                //painter->fillRect(option->rect, Qt::red);
//            }
//        }
//        painter->restore();
//        return;
//    }
//    case CE_ProgressBarLabel:{
//        return;
//    }

    case CE_ProgressBarGroove:
    {
        const bool enable = option->state &State_Enabled;
        QRect rect = proxy()->subElementRect(SE_ProgressBarGroove,option,widget);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(option->palette.color(enable ? QPalette::Active : QPalette::Disabled,QPalette::Button));
        painter->setBrush(option->palette.color(enable ? QPalette::Active : QPalette::Disabled,QPalette::Button));
        painter->drawRoundedRect(rect,4,4);
        painter->restore();
        return;
    }

    case CE_ProgressBarContents:
    {
       if(const QStyleOptionProgressBar* bar = qstyleoption_cast<const QStyleOptionProgressBar*>(option))
       {
            QRect rect = proxy()->subElementRect(SE_ProgressBarContents,option,widget);
            const bool vertical = bar->orientation == Qt::Vertical;
            const bool inverted = bar->invertedAppearance;
            qint64 minimum = qint64(bar->minimum);
            qint64 maximum = qint64(bar->maximum);
            qint64 progress = qint64(bar->progress);
            qint64 totalSteps = qMax(Q_INT64_C(1), maximum - minimum);
            const bool indeterminate = (minimum == maximum);
            const bool complete = (progress == minimum);

            bool reverse = (!vertical && (bar->direction == Qt::RightToLeft)) || vertical;
            if(inverted)
               reverse = !reverse;
            if(complete)
               return;

            QColor startColor = option->palette.color(QPalette::Highlight).lighter(125);
            QColor middleColor = option->palette.color(QPalette::Highlight);
            QColor endColor = option->palette.color(QPalette::Highlight).darker(110);
            QRect progressRect = rect;
            QLinearGradient linearGradient;
            if(!vertical)
            {
                qint64 progressBarWidth = (progress - bar->minimum) * rect.width() / totalSteps;
                int width = indeterminate ? rect.width() : progressBarWidth;
                if(!reverse)
                {
                    progressRect.setRect(rect.left(),rect.top(),width,rect.height());
                    linearGradient.setStart(progressRect.topLeft());
                    linearGradient.setFinalStop(progressRect.topRight());
                }
                else
                {
                    progressRect.setRect(indeterminate ? rect.left() : rect.right() - width, rect.top(),width, rect.height());
                    linearGradient.setStart(progressRect.topRight());
                    linearGradient.setFinalStop(progressRect.topLeft());
                }
            }
            else
            {
                qint64 progressBarWidth = (progress - minimum) * rect.height() / totalSteps;
                int width = indeterminate ? rect.height() : progressBarWidth;
                if(!inverted)
                {
                    progressRect.setRect(rect.left(), indeterminate ? rect.top() : rect.bottom() - width, rect.width(), width);
                    linearGradient.setStart(progressRect.bottomLeft());
                    linearGradient.setFinalStop(progressRect.topLeft());
                }
                else
                {
                    progressRect.setRect(rect.left(),rect.top(),rect.width(),width);
                    linearGradient.setStart(progressRect.topLeft());
                    linearGradient.setFinalStop(progressRect.bottomLeft());
                }
            }

            linearGradient.setColorAt(0,startColor);
            linearGradient.setColorAt(0.6,middleColor);
            linearGradient.setColorAt(1,endColor);
            painter->save();
            painter->setRenderHints(QPainter::Antialiasing,true);
            painter->setPen(Qt::NoPen);
            painter->setBrush(linearGradient);
            painter->drawRoundedRect(progressRect,4,4);
            painter->restore();
            return;
        }
        break;
    }

    case CE_ProgressBarLabel:
    {
        return;
    }

    case CE_ScrollBarSlider:
    {
        auto animator = m_scrollbar_animation_helper->animator(widget);
        if (!animator) {
            return Style::drawControl(element, option, painter, widget);
        }

        if (const QStyleOptionSlider *bar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            const bool enable = bar->state & State_Enabled;
            const bool sunKen = bar->state & State_Sunken;
            const bool mouseOver = bar->state & State_MouseOver;
            const bool horizontal = (bar->orientation == Qt::Horizontal);
            QRect rect = bar->rect;

            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setRenderHint(QPainter::Antialiasing, true);

            animator->setAnimatorDirectionForward("slider_opacity", mouseOver);
            animator->setAnimatorDirectionForward("groove_width", mouseOver);
            if (mouseOver) {
                if (!animator->isRunning("slider_opacity") && animator->currentAnimatorTime("slider_opacity") == 0) {
                    animator->startAnimator("slider_opacity");
                }
                if (!animator->isRunning("groove_width") && animator->currentAnimatorTime("groove_width") == 0) {
                    animator->startAnimator("groove_width");
                }
            } else {
                if (!animator->isRunning("slider_opacity") && animator->currentAnimatorTime("slider_opacity") > 0) {
                    animator->startAnimator("slider_opacity");
                }
                if (!animator->isRunning("groove_width") && animator->currentAnimatorTime("groove_width") > 0) {
                    animator->startAnimator("groove_width");
                }
            }

            if (sunKen) {
                if (animator->currentAnimatorTime("additional_opacity") == 0) {
                    animator->setAnimatorDirectionForward("additional_opacity", sunKen);
                    animator->startAnimator("additional_opacity");
                }
            } else {
                if (animator->currentAnimatorTime("additional_opacity") > 0) {
                    animator->setAnimatorDirectionForward("additional_opacity", sunKen);
                    animator->startAnimator("additional_opacity");
                }
            }

            QRectF drawRect;
            qreal len = animator->value("groove_width").toReal() * 4 + 4;
            qreal m_opacity = animator->value("slider_opacity").toReal();
            qreal s_opacity = animator->value("additional_opacity").toReal();

            if (horizontal) {
                drawRect.setRect(rect.x(), rect.y() + (rect.height() - len) / 2, rect.width(), len);
            } else {
                drawRect.setRect(rect.x() + (rect.width() - len) / 2, rect.y(), len, rect.height());
            }

            painter->setBrush(mixColor(bar->palette.color(QPalette::Active, QPalette::Button),
                                       bar->palette.color(QPalette::Active, QPalette::WindowText), m_opacity + s_opacity));
            if (!enable)
                painter->setBrush(bar->palette.brush(QPalette::Disabled, QPalette::Button));
            painter->drawRoundedRect(drawRect, len / 2, len / 2);
            painter->restore();
            return;
        }
        break;
    }

    case CE_ScrollBarAddLine: {
        return;
    }

    case CE_ScrollBarSubLine: {
        return;
    }

    case CE_PushButton:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            proxy()->drawControl(CE_PushButtonBevel, option, painter, widget);
            QStyleOptionButton subopt = *button;
            subopt.rect = proxy()->subElementRect(SE_PushButtonContents, option, widget);
            proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);
//            if (option->state & State_HasFocus || button->features & QStyleOptionButton::DefaultButton) {
//                painter->save();
//                painter->setPen(QPen(highLight_Click(), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
//                painter->setBrush(Qt::NoBrush);
//                painter->setRenderHint(QPainter::Antialiasing, true);
//                int x_Radius = 4;
//                int y_Radius = 4;
//                painter->drawRoundedRect(button->rect.adjusted(1, 1, -1, -1), x_Radius, y_Radius);
//                painter->restore();
//            }
            return;
        }
        break;
    }

    case CE_PushButtonBevel:
    {
        proxy()->drawPrimitive(PE_PanelButtonCommand, option, painter, widget);
        return;
    }

    case CE_PushButtonLabel:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            const bool enable = button->state & State_Enabled;
            const bool text = !button->text.isNull();
            const bool icon = !button->icon.isNull();

            bool isWindowButton = false;
            bool isWindowColoseButton = false;
            bool isImportant = false;
            bool useButtonPalette = false;
            if (widget) {
                if (widget->property("isWindowButton").isValid()) {
                    if (widget->property("isWindowButton").toInt() == 0x01)
                        isWindowButton = true;
                    if (widget->property("isWindowButton").toInt() == 0x02)
                        isWindowColoseButton = true;
                }
                if (widget->property("isImportant").isValid())
                    isImportant = widget->property("isImportant").toBool();

                if (widget->property("useButtonPalette").isValid())
                    useButtonPalette = widget->property("useButtonPalette").toBool();
            }

            QRect drawRect = button->rect;
            int spacing = 8;
            QStyleOption sub = *option;
            if (isImportant && !(button->features & QStyleOptionButton::Flat))
                sub.state = option->state | State_On;
            else if (isWindowButton || useButtonPalette)
                sub.state = enable ? State_Enabled : State_None;
            else
                sub.state = option->state;

            if (button->features & QStyleOptionButton::HasMenu) {
                QRect arrowRect;
                int indicator = proxy()->pixelMetric(PM_MenuButtonIndicator, option, widget);
                arrowRect.setRect(drawRect.right() - indicator, drawRect.top() + (drawRect.height() - indicator) / 2, indicator, indicator);
                arrowRect = visualRect(option->direction, option->rect, arrowRect);
                if (!text && !icon)
                    spacing = 0;
                drawRect.setWidth(drawRect.width() - indicator - spacing);
                drawRect = visualRect(button->direction, button->rect, drawRect);
                sub.rect = arrowRect;
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &sub, painter, widget);
            }

            int tf = Qt::AlignCenter;
            if (proxy()->styleHint(SH_UnderlineShortcut, button, widget))
                tf |= Qt::TextShowMnemonic;
            QPixmap pixmap;
            if (icon) {
                QIcon::Mode mode = button->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;
                if (mode == QIcon::Normal && button->state & State_HasFocus)
                    mode = QIcon::Active;
                QIcon::State state = QIcon::Off;
                if (button->state & State_On)
                    state = QIcon::On;
                pixmap = button->icon.pixmap(button->iconSize, mode, state);
            }

            QFontMetrics fm = button->fontMetrics;
            int textWidth = fm.boundingRect(option->rect, tf, button->text).width() + 2;
            int iconWidth = icon ? button->iconSize.width() : 0;
            QRect iconRect, textRect;
            if (icon && text) {
                int width = textWidth + spacing + iconWidth;
                if (width > drawRect.width()) {
                    width = drawRect.width();
                    textWidth = width - spacing - iconWidth;
                }
                textRect.setRect(drawRect.x(), drawRect.y(), width, drawRect.height());
                textRect.moveCenter(drawRect.center());
                iconRect.setRect(textRect.left(), textRect.top(), iconWidth, textRect.height());
                textRect.setRect(iconRect.right() + spacing + 1, textRect.y(), textWidth, textRect.height());
                iconRect = visualRect(option->direction, drawRect, iconRect);
                textRect = visualRect(option->direction, drawRect, textRect);
            } else if (icon) {
                iconRect = drawRect;
            } else if (text) {
                textRect = drawRect;
            }

            if (iconRect.isValid()) {
                if (!(HighLightEffect::isWidgetIconUseHighlightEffect(widget))) {
                    if (isImportant || isWindowColoseButton) {
                        pixmap = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, &sub, widget);
                    } else {
                        pixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, &sub, widget);
                    }
                } else {
                    pixmap = HighLightEffect::generatePixmap(pixmap, &sub, widget);
                }
                QStyle::drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
            }

            if (textRect.isValid()) {
                if (enable) {
                    if (isWindowButton || useButtonPalette) {
                        proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::ButtonText);
                    } else {
                        if (isImportant) {
                            if (button->features & QStyleOptionButton::Flat) {
                                proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::ButtonText);
                            } else {
                                proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::HighlightedText);
                            }
                            if (button->state & (State_MouseOver | State_Sunken | State_On)) {
                                proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::HighlightedText);
                            }
                        } else {
                            if (button->state & (State_MouseOver | State_Sunken | State_On)) {
                                proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::HighlightedText);
                            } else {
                                proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::ButtonText);
                            }
                        }
                    }
                } else {
                    proxy()->drawItemText(painter, textRect, tf, button->palette, false, button->text, QPalette::ButtonText);
                }
            }
            return;
        }
        break;
    }

    case CE_ToolButtonLabel:
    {
        if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            bool isWindowColoseButton = false;
            bool isWindowButton = false;
            bool useButtonPalette = false;
            if (widget) {
                if (widget->property("isWindowButton").isValid()) {
                    if (widget->property("isWindowButton").toInt() == 0x01)
                        isWindowButton = true;
                    if (widget->property("isWindowButton").toInt() == 0x02)
                        isWindowColoseButton = true;
                }
                if (widget->property("useButtonPalette").isValid())
                    useButtonPalette = widget->property("useButtonPalette").toBool();
            }

            const bool text = !tb->text.isNull();
            const bool icon = !tb->icon.isNull();
            const bool arrow = tb->features & QStyleOptionToolButton::MenuButtonPopup;
            const bool ha = tb->features & QStyleOptionToolButton::Arrow;
            const bool enable = tb->state & State_Enabled;
            QFontMetrics fm = tb->fontMetrics;
            int Margin_Height = 2;
            int Button_MarginWidth = proxy()->pixelMetric(PM_ButtonMargin, option, widget);

            int iconWidth = (icon || ha) ? tb->iconSize.width() : 0;
            int spacing = 8;
            QRect textRect, iconRect, arrowRect;
            QRect drawRect = tb->rect;
            drawRect.adjust(Button_MarginWidth, Margin_Height, -Button_MarginWidth, -Margin_Height);

            QStyleOption sub = *option;
            sub.state = (isWindowButton || useButtonPalette) ? (enable ? State_Enabled : State_None) : (option->state);
            if (arrow) {
                int mbi = proxy()->pixelMetric(PM_MenuButtonIndicator, option, widget);
                arrowRect.setRect(drawRect.right() - mbi + 1, drawRect.y(), mbi, drawRect.height());
                drawRect.adjust(0, 0, - (mbi + spacing), 0);
                arrowRect = visualRect(option->direction, option->rect, arrowRect);
                drawRect = visualRect(option->direction, option->rect, drawRect);
                sub.rect = arrowRect;
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &sub, painter, widget);
            }

            int alignment = Qt::AlignCenter;
            if (proxy()->styleHint(SH_UnderlineShortcut, option, widget))
                alignment |= Qt::TextShowMnemonic;
            int textDis = fm.boundingRect(option->rect, alignment, tb->text).width() + 2;
            QPixmap pixmap;
            if (icon) {
                QIcon::State state = tb->state & State_On ? QIcon::On : QIcon::Off;
                QIcon::Mode mode;
                if (!enable)
                    mode = QIcon::Disabled;
                else if ((tb->state & State_MouseOver) && (tb->state & State_AutoRaise))
                    mode = QIcon::Active;
                else
                    mode = QIcon::Normal;
                pixmap = tb->icon.pixmap(tb->iconSize, mode, state);
            }

            if (tb->toolButtonStyle == Qt::ToolButtonTextOnly && text) {
                textRect = drawRect;
            } else if (tb->toolButtonStyle == Qt::ToolButtonIconOnly) {
                if (icon || ha) {
                    iconRect = drawRect;
                } else if (text) {
                    textRect = drawRect;
                }
            } else if (tb->toolButtonStyle == Qt::ToolButtonTextBesideIcon) {
                if (text) {
                    int width = iconWidth + spacing + textDis;
                    if (width > drawRect.width()) {
                        width = drawRect.width();
                        textDis = drawRect.width() - iconWidth - spacing;
                    }
                    textRect.setRect(drawRect.x(), drawRect.y(), width, drawRect.height());
                    textRect.moveCenter(drawRect.center());
                    iconRect.setRect(textRect.x(), textRect.y(), iconWidth, textRect.height());
                    textRect.setRect(iconRect.right() + spacing + 1, textRect.y(), textDis, textRect.height());
                    iconRect = visualRect(option->direction, drawRect, iconRect);
                    textRect = visualRect(option->direction, drawRect, textRect);
                } else {
                    iconRect = drawRect;
                }
            } else if (tb->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                if (text) {
                    textDis = qMax(iconWidth, textDis);
                    int height = fm.size(Qt::TextShowMnemonic, tb->text).height() + spacing + iconWidth;
                    if (textDis > drawRect.width())
                        textDis = drawRect.width();
                    textRect.setRect(drawRect.x(), drawRect.y(), textDis, height);
                    textRect.moveCenter(drawRect.center());
                    iconRect.setRect(textRect.x(), textRect.y(), textDis, iconWidth);
                    textRect.setRect(textRect.x(), iconRect.bottom() + spacing + 1, textDis, fm.size(Qt::TextShowMnemonic, tb->text).height());
                } else {
                    iconRect = drawRect;
                }
            }

            if (textRect.isValid()) {
                proxy()->drawItemText(painter, textRect, alignment, tb->palette, enable, tb->text,
                                      (isWindowButton || useButtonPalette) ? QPalette::ButtonText
                                      : (option->state & (State_On | State_Sunken | State_MouseOver) && (option->state & State_Enabled))
                                      ? QPalette::HighlightedText : QPalette::ButtonText);
            }
            if (iconRect.isValid()) {
                sub.rect = iconRect;
                switch (tb->arrowType) {
                case Qt::UpArrow:
                {
                    proxy()->drawPrimitive(PE_IndicatorArrowUp, &sub, painter, widget);
                    return;
                }
                case Qt::DownArrow:
                {
                    proxy()->drawPrimitive(PE_IndicatorArrowDown, &sub, painter, widget);
                    return;
                }
                case Qt::LeftArrow:
                {
                    proxy()->drawPrimitive(PE_IndicatorArrowLeft, &sub, painter, widget);
                    return;
                }
                case Qt::RightArrow:
                {
                    proxy()->drawPrimitive(PE_IndicatorArrowRight, &sub, painter, widget);
                    return;
                }
                default:
                    break;
                }

                if (!(HighLightEffect::isWidgetIconUseHighlightEffect(widget))) {
                    if (isWindowColoseButton) {
                        pixmap = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, &sub, widget);
                    } else {
                        pixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, &sub, widget);
                    }
                } else {
                    pixmap = HighLightEffect::generatePixmap(pixmap, &sub, widget);
                }
                QStyle::drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
            }
            return;
        }
        break;
    }

    case CE_TabBarTab:
    {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            proxy()->drawControl(CE_TabBarTabShape, tab, painter, widget);
            proxy()->drawControl(CE_TabBarTabLabel, tab, painter, widget);
            return;
        }
        break;
    }

    case CE_TabBarTabShape:
    {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            bool enable = tab->state & State_Enabled;
            bool selected = tab->state & State_Selected;
            bool hover = tab->state & State_MouseOver;
            QRect drawRect = option->rect;
            QTransform rotMatrix;
            bool flip = false;
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setRenderHint(QPainter::Antialiasing, true);

            switch (tab->shape) {
            case QTabBar::RoundedNorth:
                break;
            case QTabBar::RoundedSouth:
            {
                rotMatrix.rotate(180);
                rotMatrix.translate(0, -tab->rect.height() + 1);
                rotMatrix.scale(-1, 1);
                painter->setTransform(rotMatrix, true);
                break;
            }
            case QTabBar::RoundedWest:
            {
                flip = true;
                rotMatrix.rotate(180 + 90);
                rotMatrix.scale(-1, 1);
                painter->setTransform(rotMatrix, true);
                break;
            }
            case QTabBar::RoundedEast:
            {
                flip = true;
                rotMatrix.rotate(90);
                rotMatrix.translate(0, - tab->rect.width() + 1);
                painter->setTransform(rotMatrix);
                break;
            }
            default:
                painter->restore();
                QCommonStyle::drawControl(element, tab, painter, widget);
                return;
            }

            if (flip) {
                drawRect.adjust(0, 0, 1, 0);
                QRect tmp = drawRect;
                drawRect = QRect(tmp.y(), tmp.x(), tmp.height(), tmp.width());
            } else {
                drawRect.adjust(0, 0, 0, 1);
            }

            bool rtlHorTabs = (tab->direction == Qt::RightToLeft
                               && (tab->shape == QTabBar::RoundedNorth || tab->shape == QTabBar::RoundedSouth));
            bool fisttab = ((!rtlHorTabs && tab->position == QStyleOptionTab::Beginning)
                            || (rtlHorTabs && tab->position == QStyleOptionTab::End));
            bool lastTab = ((!rtlHorTabs && tab->position == QStyleOptionTab::End)
                            || (rtlHorTabs && tab->position == QStyleOptionTab::Beginning));
            bool onlyOne = tab->position == QStyleOptionTab::OnlyOneTab;

            int tabOverlap = proxy()->pixelMetric(PM_TabBarTabOverlap, option, widget);
            if (selected || hover) {
                if (fisttab || onlyOne) {
                    drawRect.adjust(0, 0, tabOverlap, 0);
                } else if (lastTab) {
                    drawRect.adjust(-tabOverlap, 0, 0, 0);
                } else {
                    drawRect.adjust(-tabOverlap, 0, tabOverlap, 0);
                }

                int TabBarTab_Radius = 6;
                QPainterPath path;
                path.moveTo(drawRect.left() + TabBarTab_Radius, drawRect.top());
                path.arcTo(QRect(drawRect.left(), drawRect.top(), TabBarTab_Radius * 2, TabBarTab_Radius * 2), 90, 90);
                path.lineTo(drawRect.left(), drawRect.bottom() - TabBarTab_Radius);
                path.arcTo(QRect(drawRect.left() - TabBarTab_Radius * 2, drawRect.bottom() - TabBarTab_Radius * 2,
                                 TabBarTab_Radius * 2, TabBarTab_Radius * 2), 0, -90);
                path.lineTo(drawRect.right() + TabBarTab_Radius, drawRect.bottom());
                path.arcTo(QRect(drawRect.right(), drawRect.bottom() - TabBarTab_Radius * 2,
                                 TabBarTab_Radius * 2, TabBarTab_Radius * 2), 270, -90);
                path.lineTo(drawRect.right(), drawRect.top() + TabBarTab_Radius);
                path.arcTo(QRect(drawRect.right() - TabBarTab_Radius * 2, drawRect.top(),
                                 TabBarTab_Radius * 2, TabBarTab_Radius * 2), 0, 90);
                path.lineTo(drawRect.left() + TabBarTab_Radius, drawRect.top());

                painter->setBrush(tab->palette.brush(QPalette::Active, QPalette::Base));
                if (hover && !selected)
                    painter->setBrush(mixColor(tab->palette.color(QPalette::Active, QPalette::Base),
                                               tab->palette.color(QPalette::Active, QPalette::Window), 0.6));
                painter->drawPath(path);
            } else {
                painter->setBrush(tab->palette.brush(QPalette::Active, QPalette::Window));
                painter->drawRect(drawRect);
            }
            painter->restore();
            return;
        }
        break;
    }

    case CE_TabBarTabLabel:
    {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            int iconSize = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
            bool verticalTabs = tab->shape == QTabBar::RoundedEast || tab->shape == QTabBar::RoundedWest
                    || tab->shape == QTabBar::TriangularEast || tab->shape == QTabBar::TriangularWest;
            uint alignment = Qt::AlignLeft | Qt::AlignVCenter;
            if (proxy()->styleHint(SH_UnderlineShortcut, option, widget))
                alignment |= Qt::TextShowMnemonic;
            else
                alignment |= Qt::TextHideMnemonic;

            QRect drawRect = tab->rect;
            QRect iconRect;
            QRect textRect;
            tabLayout(tab, widget, proxy(), &textRect, &iconRect);
            textRect = proxy()->subElementRect(SE_TabBarTabText, option, widget);

            painter->save();
            if (verticalTabs) {
                int newX, newY, newRot;
                if (tab->shape == QTabBar::RoundedEast || tab->shape == QTabBar::TriangularEast) {
                    newX = drawRect.width() + drawRect.x();
                    newY = drawRect.y();
                    newRot = 90;
                } else {
                    newX = drawRect.x();
                    newY = drawRect.y() + drawRect.height();
                    newRot = -90;
                }
                QTransform m;
                m.translate(newX, newY);
                m.rotate(newRot);
                painter->setTransform(m, true);
            }

            if (!tab->icon.isNull()) {
                QPixmap pixmap = tab->icon.pixmap(widget ? widget->window()->windowHandle() : 0, tab->iconSize,
                                                  (tab->state & State_Enabled) ? QIcon::Normal : QIcon::Disabled,
                                                  (tab->state & State_Selected) ? QIcon::On : QIcon::Off);
                QPixmap drawPixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
                painter->drawPixmap(iconRect.x(), iconRect.y(), drawPixmap);
            }
            proxy()->drawItemText(painter, textRect, alignment, tab->palette, tab->state & State_Enabled, tab->text, QPalette::WindowText);
            if (!(tab->state & State_Selected)) {
                int dis = ((verticalTabs ? drawRect.width() : drawRect.height()) - iconSize) / 2;
                painter->save();
                painter->resetTransform();
                painter->setPen(tab->palette.color(QPalette::Active, QPalette::Midlight));
                painter->setBrush(Qt::NoBrush);
                if (verticalTabs) {
                    if (tab->shape == QTabBar::RoundedEast || tab->shape == QTabBar::TriangularEast) {
                        painter->drawLine(drawRect.x() + dis, drawRect.bottom(), drawRect.right() - dis, drawRect.bottom());
                    } else {
                        painter->drawLine(drawRect.x() + dis, drawRect.top(), drawRect.right() - dis, drawRect.top());
                    }
                } else if (tab->direction == Qt::RightToLeft){
                    painter->drawLine(drawRect.x(), drawRect.top() + dis, drawRect.x(), drawRect.bottom() - dis);
                } else {
                    painter->drawLine(drawRect.right(), drawRect.top() + dis, drawRect.right(), drawRect.bottom() - dis);
                }
                painter->restore();
            }
            painter->restore();
            return;
        }
        break;
    }

    case CE_ComboBoxLabel:
    {
        if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            QRect arrowRect = proxy()->subControlRect(CC_ComboBox, comboBox, SC_ComboBoxArrow, widget);
            QRect editRect = proxy()->subControlRect(CC_ComboBox, comboBox, SC_ComboBoxEditField, widget);

            QStyleOption arrow = *option;
            arrow.state = option->state & State_Enabled ? State_Enabled : State_None;
            arrow.rect = arrowRect;
            proxy()->drawPrimitive(PE_IndicatorArrowDown, &arrow, painter, widget);

            painter->save();
            if (!comboBox->currentIcon.isNull()) {
                QIcon::Mode mode = comboBox->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;
                QPixmap pixmap = comboBox->currentIcon.pixmap(comboBox->iconSize, mode);
                QRect iconRect;
                if (comboBox->direction == Qt::RightToLeft) {
                    iconRect.setRect(editRect.right() - comboBox->iconSize.width(), editRect.y(), comboBox->iconSize.width(), editRect.height());
                    editRect.setRect(editRect.x(), editRect.y(), editRect.width() - iconRect.width() - 8, editRect.height());
                } else {
                    iconRect.setRect(editRect.x(), editRect.y(), comboBox->iconSize.width(), editRect.height());
                    editRect.setRect(editRect.x() + iconRect.width() + 8, editRect.y(), editRect.width() - iconRect.width() - 8, editRect.height());
                }
                proxy()->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
            }

            if (!comboBox->currentText.isNull() && !comboBox->editable) {
                proxy()->drawItemText(painter, editRect, visualAlignment(option->direction, Qt::AlignLeft | Qt::AlignVCenter),
                                      option->palette, option->state & State_Enabled, comboBox->currentText);
            }
            painter->restore();
            return;
        }
        break;
    }

    case CE_RadioButton:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            QStyleOptionButton subopt = *button;
            subopt.rect = proxy()->subElementRect(SE_RadioButtonIndicator, option, widget);
            proxy()->drawPrimitive(PE_IndicatorRadioButton, &subopt, painter, widget);
            subopt.rect = proxy()->subElementRect(SE_RadioButtonContents, option, widget);
            proxy()->drawControl(CE_RadioButtonLabel, &subopt, painter, widget);
            return;
        }
        break;
    }

    case CE_CheckBox:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            QStyleOptionButton subopt = *button;
            subopt.rect = proxy()->subElementRect(SE_CheckBoxIndicator, option, widget);
            proxy()->drawPrimitive(PE_IndicatorCheckBox, &subopt, painter, widget);
            subopt.rect = proxy()->subElementRect(SE_CheckBoxContents, option, widget);
            proxy()->drawControl(CE_CheckBoxLabel, &subopt, painter, widget);
            return;
        }
        break;
    }

    case CE_RadioButtonLabel:
    case CE_CheckBoxLabel:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            uint alignment = visualAlignment(button->direction, Qt::AlignLeft | Qt::AlignVCenter);
            const bool enable = button->state & State_Enabled;

            if (!proxy()->styleHint(SH_UnderlineShortcut, button, widget))
                alignment |= Qt::TextHideMnemonic;
            QPixmap pixmap;
            QRect textRect = button->rect;
            if (!button->icon.isNull()) {
                pixmap = button->icon.pixmap(button->iconSize, enable ? QIcon::Normal : QIcon::Disabled);
                proxy()->drawItemPixmap(painter, button->rect, alignment, pixmap);
                int spacing = 8;
                if (button->direction == Qt::RightToLeft)
                    textRect.setRight(textRect.right() - button->iconSize.width() - spacing);
                else
                    textRect.setLeft(textRect.left() + button->iconSize.width() + spacing);
            }
            if (!button->text.isEmpty()){
                proxy()->drawItemText(painter, textRect, alignment | Qt::TextShowMnemonic,
                                      button->palette, button->state & State_Enabled, button->text, QPalette::WindowText);
            }
            return;
        }
        break;
    }

    case CE_Header:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option))
        {
            painter->save();
            painter->setClipRect(option->rect);
            proxy()->drawControl(CE_HeaderSection, header, painter, widget);
            QStyleOptionHeader subopt = *header;
            subopt.rect = proxy()->subElementRect(SE_HeaderLabel, header, widget);
            if (subopt.rect.isValid())
                proxy()->drawControl(CE_HeaderLabel, &subopt, painter, widget);
            if (header->sortIndicator != QStyleOptionHeader::None)
            {
                subopt.rect = proxy()->subElementRect(SE_HeaderArrow, option, widget);
                proxy()->drawPrimitive(PE_IndicatorHeaderArrow, &subopt, painter, widget);
            }
            painter->restore();
            return;
        }
        break;
    }

    case CE_HeaderSection:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)){

            bool mouse = header->state &  QStyle::State_MouseOver;
            bool select = header->state & QStyle::State_Sunken;
            bool on = header->state & QStyle::State_On;
            bool roundedRight = false;
            //角落控件控制
            const bool isCorner( widget && widget->inherits( "QTableCornerButton" ) );
            //控件方向判断
            const bool reverseLayout( option->direction == Qt::LeftToRight );
            //对控件位置判断
            if(header->section == QStyleOptionHeader::Middle){
                roundedRight = true;
            }else if(header->section == QStyleOptionHeader::Beginning){
                roundedRight = true;
            }

            painter->save();
            painter->setPen(Qt::transparent);
            //tree头部表格控制
            const auto view = qobject_cast<const QHeaderView*>(widget);
            if (view) {
                auto treeView = qobject_cast<const QTreeView*>(view->parent());
                if (treeView){
                    painter->setBrush(header->palette.color(QPalette::Base));
                    painter->drawRect(header->rect);
                    if(!isCorner){
                        if(roundedRight){
                            painter->setPen(option->palette.color(QPalette::Button).darker(110));
                            if(reverseLayout){
                                if(header->orientation == Qt::Horizontal){
                                    painter->drawLine(header->rect.right(),header->rect.top()+4,header->rect.right(),header->rect.bottom()-4);
                                }
                            }else{
                                if(header->orientation == Qt::Horizontal){
                                    painter->drawLine(header->rect.left(),header->rect.top()+4,header->rect.left(),header->rect.bottom()-4);
                                }
                            }
                        }
                    }
                    painter->restore();
                    return;
                }
            }

            painter->setBrush(header->palette.color(QPalette::Button).lighter(105));
            painter->drawRect(header->rect);
            if(on){
                painter->setBrush(header->palette.color(QPalette::Button).darker(105));
            }
            if(mouse){
                painter->setBrush(header->palette.color(QPalette::Button));
            }
            if(select){
                painter->setBrush(header->palette.color(QPalette::Button).darker(105));
            }
            QPainterPath path_indicator1;
            path_indicator1.addRoundedRect(option->rect.adjusted(+3,+3,-3,-3),4,4);
            painter->drawPath(path_indicator1);


            //根据不同位置绘制外观
            if(!isCorner){
                if(roundedRight){
                    painter->setPen(option->palette.color(QPalette::Button).darker(110));
                    if(reverseLayout){
                        if(header->orientation == Qt::Horizontal){
                            painter->drawLine(header->rect.right(),header->rect.top()+4,header->rect.right(),header->rect.bottom()-4);
                        }
                    }else{
                        if(header->orientation == Qt::Horizontal){
                            painter->drawLine(header->rect.left(),header->rect.top()+4,header->rect.left(),header->rect.bottom()-4);
                        }
                    }
                }
            }

            painter->restore();
            return;
        }
        break;
    }

    case CE_HeaderEmptyArea:{
        bool enable = option->state & QStyle::State_Enabled;
        painter->fillRect(option->rect,option->palette.color(enable ? QPalette::Active : QPalette::Disabled,
                                                             QPalette::Button).lighter(105));
        const auto view = qobject_cast<const QHeaderView*>(widget);
        if (view) {
            auto treeView = qobject_cast<const QTreeView*>(view->parent());
            if (treeView){
                painter->setPen(Qt::NoPen);
                painter->setBrush(option->palette.color(QPalette::Base));
                painter->drawRect(option->rect);
                painter->restore();
                return;
            }
        }
        break;
    }

    case CE_SizeGrip:
    {
        /*
             * Style is not required here, as required by design
             */
        return;

    }break;


        //Drawing of single menu item of menu bar
    case CE_MenuBarItem:
    {

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        if (const QStyleOptionMenuItem *mbi = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
        {
            QStyleOptionMenuItem item = *mbi;
            item.rect = mbi->rect.adjusted(0, 1, 0, -3);
            painter->fillRect(option->rect, option->palette.window());

            uint alignment = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip| Qt::TextSingleLine;

            if (!proxy()->styleHint(SH_UnderlineShortcut, mbi, widget))
                alignment |= Qt::TextHideMnemonic;

            QPixmap pix = mbi->icon.pixmap(proxy()->pixelMetric(PM_SmallIconSize, option, widget), QIcon::Normal);
            if (!pix.isNull())
                proxy()->drawItemPixmap(painter,mbi->rect, alignment, pix);
            else
                proxy()->drawItemText(painter, mbi->rect, alignment, mbi->palette, mbi->state & State_Enabled,
                                      mbi->text, QPalette::ButtonText);

            bool act = mbi->state & State_Selected && mbi->state & State_Sunken | mbi->state & State_HasFocus;
            bool dis = !(mbi->state & State_Enabled);


            QRect r = option->rect;
            //when hover、click and other state, begin to draw style
            if (act) {
                painter->setBrush(option->palette.highlight().color());
                painter->setPen(Qt::NoPen);
                painter->drawRoundedRect(r.adjusted(0, 0, -1, -1),4,4);

                QPalette::ColorRole textRole = dis ? QPalette::Text : QPalette::HighlightedText;
                uint alignment = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                if (!Style::styleHint(SH_UnderlineShortcut, mbi, widget))
                    alignment |= Qt::TextHideMnemonic;
                proxy()->drawItemText(painter, item.rect, alignment, mbi->palette, mbi->state & State_Enabled, mbi->text, textRole);
            } else {

            }
        }
        painter->restore();

        return;
        break;

    }



    case CE_MenuItem:
    {
        if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            const bool enable(menuItem->state & State_Enabled);
            if (menuItem->menuItemType == QStyleOptionMenuItem::EmptyArea) {
                return;
            }

            QRect drawRect = menuItem->rect;
            if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {
                int SepMenuItem_Margin = 8;
                painter->save();
                QBrush SepBrush = menuItem->palette.brush(QPalette::Active, QPalette::Text);
                painter->setPen(QPen(SepBrush, 1, Qt::SolidLine, Qt::RoundCap));
                painter->setBrush(Qt::NoBrush);
                painter->setOpacity(0.1);
                painter->drawLine(QPointF(drawRect.left() + SepMenuItem_Margin, drawRect.center().y()),
                                  QPointF(drawRect.right() - SepMenuItem_Margin, drawRect.center().y()));
                painter->restore();
                return;
            }

            const bool hover(menuItem->state & State_MouseOver);
            const bool sunken(menuItem->state & State_Sunken);
            const bool selected(menuItem->state & State_Selected);
            const bool layout(menuItem->direction == Qt::LeftToRight);
            int MenuItem_xRadius = 2;
            int MenuItem_yRadius = 2;
            if (enable && (selected | hover | sunken)) {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing);
                painter->setPen(Qt::NoPen);
                painter->setBrush(menuItem->palette.brush(QPalette::Active, QPalette::Highlight));
                painter->drawRoundedRect(drawRect, MenuItem_xRadius, MenuItem_yRadius);
                painter->restore();
            }
            int MenuItem_Spacing = 8;
            int MenuItem_VMargin = 3;
            drawRect = drawRect.adjusted(8, MenuItem_VMargin, -8, -MenuItem_VMargin);//去除item边框

            if (menuItem->menuHasCheckableItems) {
                const bool checked = menuItem->checked;
                if (menuItem->checkType == QStyleOptionMenuItem::Exclusive) {
                    if (checked) {
//                        QStyleOptionButton radioOption;
//                        radioOption.QStyleOption::operator=(*option);
//                        radioOption.rect = drawRect;
//                        radioOption.state |= State_On;
//                        radioOption.rect = proxy()->subElementRect(SE_RadioButtonIndicator, &radioOption, widget);
//                        proxy()->drawPrimitive(PE_IndicatorRadioButton, &radioOption, painter, widget);
                        int iconWidth = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
                        QIcon icon = QIcon::fromTheme("dialog-ok");
                        QIcon::Mode mode = enable ?  QIcon::Active : QIcon::Disabled;
                        QPixmap pixmap = icon.pixmap(iconWidth, iconWidth, mode , QIcon::On);
                        QPixmap drawPixmap = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
                        QRect iconRect(drawRect.x(), drawRect.y() + (drawRect.height() - iconWidth) / 2, iconWidth, iconWidth);
                        iconRect = visualRect(menuItem->direction, drawRect, iconRect);
                        painter->save();
                        painter->setPen(Qt::NoPen);
                        painter->setBrush(Qt::NoBrush);
                        painter->drawPixmap(iconRect, drawPixmap);
                        painter->restore();
                    }
                } else if (menuItem->checkType == QStyleOptionMenuItem::NonExclusive) {
                    if (checked) {
                        int iconWidth = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
                        QIcon icon = QIcon::fromTheme("dialog-ok");
                        QIcon::Mode mode = enable ? (selected ? QIcon::Active : QIcon::Normal) : QIcon::Disabled;
                        QPixmap pixmap = icon.pixmap(iconWidth, iconWidth, mode , QIcon::On);
                        QPixmap drawPixmap = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
                        QRect iconRect(drawRect.x(), drawRect.y() + (drawRect.height() - iconWidth) / 2, iconWidth, iconWidth);
                        iconRect = visualRect(menuItem->direction, drawRect, iconRect);
                        painter->save();
                        painter->setPen(Qt::NoPen);
                        painter->setBrush(Qt::NoBrush);
                        painter->drawPixmap(iconRect, drawPixmap);
                        painter->restore();
                    }
                }
            }

            if (menuItem->maxIconWidth != 0 && menuItem->checkType == QStyleOptionMenuItem::NotCheckable) {
                int smallIconSize = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
                if (!menuItem->icon.isNull()) {
                    QSize iconSize(smallIconSize, smallIconSize);
                    QIcon::Mode mode =  enable ? (selected ? QIcon::Active : QIcon::Normal) : QIcon::Disabled;
                    QIcon::State state = menuItem->checked ? QIcon::On : QIcon::Off;
                    QPixmap pixmap = menuItem->icon.pixmap(iconSize, mode, state);
                    QPixmap target = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
                    QRect iconRect(drawRect.x(), drawRect.y() + (drawRect.height() - smallIconSize)/2, smallIconSize, smallIconSize);
                    iconRect = visualRect(menuItem->direction, drawRect, iconRect);
                    painter->save();
                    painter->setPen(menuItem->palette.color(QPalette::Active, QPalette::Text));
                    painter->setBrush(Qt::NoBrush);
                    painter->drawPixmap(iconRect, target);
                    painter->restore();
                }
            }

            if (menuItem->menuHasCheckableItems || menuItem->maxIconWidth != 0) {
                int iconWidth = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
                drawRect = visualRect(menuItem->direction, drawRect, drawRect.adjusted(iconWidth + MenuItem_Spacing, 0, 0, 0));
            } else {
                drawRect = drawRect.adjusted(4, 0, -4, -0);//去除item边框
            }

            if (menuItem->menuItemType == QStyleOptionMenuItem::SubMenu) {
                int indWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
                int indHight = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
                QRect indRect(drawRect.right() - indWidth, drawRect.y() + (drawRect.height() - indHight)/2, indWidth, indHight);//箭头绘制是居中的，绘制会有写误差。
                indRect = visualRect(menuItem->direction, drawRect, indRect);
                QStyleOption indOption = *option;
                if (selected)
                    indOption.state |= State_MouseOver;
                indOption.rect = indRect;
                proxy()->drawPrimitive(layout ? PE_IndicatorArrowRight : PE_IndicatorArrowLeft, &indOption, painter, widget);
                drawRect = visualRect(menuItem->direction, drawRect, drawRect.adjusted(0, 0, -(indWidth + MenuItem_Spacing), 0));
            }

            if (!menuItem->text.isEmpty()) {
                QString text = menuItem->text;
                painter->save();
                if (enable) {
                    if (selected | hover | sunken) {
                        painter->setPen(menuItem->palette.color(QPalette::Active, QPalette::HighlightedText));
                    } else {
                        painter->setPen(menuItem->palette.color(QPalette::Active, QPalette::Text));
                    }
                } else {
                    painter->setPen(menuItem->palette.color(QPalette::Disabled, QPalette::Text));
                }
                painter->setBrush(Qt::NoBrush);

                int text_flags = Qt::AlignVCenter | Qt::TextDontClip | Qt::TextSingleLine;
                if (proxy()->styleHint(SH_UnderlineShortcut, option, widget))
                    text_flags |= Qt::TextShowMnemonic;
                int textToDraw_flags = text_flags;
                int t = menuItem->text.indexOf(QLatin1Char('\t'));
                if (t >=0) {
                    const QString textToDraw = menuItem->text.mid(t + 1);
                    text = menuItem->text.left(t);
                    if (layout)
                        textToDraw_flags |= Qt::AlignRight;
                    else
                        textToDraw_flags |= Qt::AlignLeft;
                    painter->drawText(drawRect.adjusted(0, 0, 0, -2), textToDraw_flags, textToDraw);
                }

                QFont font = menuItem->font;
                font.setPointSizeF(QFontInfo(menuItem->font).pointSizeF());
                if (menuItem->menuItemType == QStyleOptionMenuItem::DefaultItem)
                    font.setBold(true);
                painter->setFont(font);

                if (layout)
                    text_flags |= Qt::AlignLeft;
                else
                    text_flags |= Qt::AlignRight;
                painter->drawText(drawRect.adjusted(0, 0, 0, -2), text_flags, text);
                painter->restore();
            }
            return;
        }
        break;
    }



        // Draw UKUI ToolBoxTab Style
    case CE_ToolBoxTab:
        if (const QStyleOptionToolBox *tb = qstyleoption_cast<const QStyleOptionToolBox *>(option)) {
            proxy()->drawControl(CE_ToolBoxTabShape, tb, painter, widget);
            proxy()->drawControl(CE_ToolBoxTabLabel, tb, painter, widget);
            return;
        }
        break;

    case CE_ToolBoxTabShape:
        if (const QStyleOptionToolBox *tb = qstyleoption_cast<const QStyleOptionToolBox *>(option)) {
            painter->save();
            painter->setPen(Qt::NoPen);
            if(widget->isEnabled()){
                painter->setBrush(tb->palette.button().color());
                if(option->state & State_MouseOver)
                {
                    painter->setBrush(tb->palette.button().color().darker(110));
                    if(option->state & State_Sunken)
                        painter->setBrush(tb->palette.button().color().darker(110));
                }
            }
            else {
                painter->setBrush(tb->palette.color(QPalette::Disabled,QPalette::Button));
            }

            if (tb->direction != Qt::RightToLeft|tb->direction != Qt::RightToLeft) {
                painter->drawRoundedRect(option->rect,4,4);
            }
            painter->restore();
            return;
        } break;

    case CE_ToolBoxTabLabel:
        if (const QStyleOptionToolBox *tb = qstyleoption_cast<const QStyleOptionToolBox *>(option)) {
            bool enabled = tb->state & State_Enabled;
            bool selected = tb->state & State_Selected;
            int iconExtent = proxy()->pixelMetric(QStyle::PM_SmallIconSize, tb, widget);
            QPixmap pm = tb->icon.pixmap(widget ? widget->window()->windowHandle() : 0, QSize(iconExtent, iconExtent),enabled ? QIcon::Normal : QIcon::Disabled);

            QRect cr = subElementRect(QStyle::SE_ToolBoxTabContents, tb, widget);
            QRect tr, ir;
            int ih = 0;
            if (pm.isNull()) {
                tr = cr;
                tr.adjust(4, 0, -8, 0);
            } else {
                int iw = pm.width() / pm.devicePixelRatio() + 4;
                ih = pm.height()/ pm.devicePixelRatio();
                ir = QRect(cr.left() + 4, cr.top(), iw + 2, ih);
                tr = QRect(ir.right(), cr.top(), cr.width() - ir.right() - 4, cr.height());
            }

            if (selected && proxy()->styleHint(QStyle::SH_ToolBox_SelectedPageTitleBold, tb, widget)) {
                QFont f(painter->font());
                f.setBold(true);
                painter->setFont(f);
            }

            QString txt = tb->fontMetrics.elidedText(tb->text, Qt::ElideRight, tr.width());

            if (ih)
                painter->drawPixmap(ir.left(), (tb->rect.height() - ih) / 2, pm);

            int alignment = Qt::AlignCenter | Qt::AlignVCenter | Qt::TextShowMnemonic;
            if (!proxy()->styleHint(QStyle::SH_UnderlineShortcut, tb, widget))
                alignment |= Qt::TextHideMnemonic;

            // painter->drawText(option->rect,tb->text, QTextOption(Qt::AlignCenter));
            proxy()->drawItemText(painter, tr, alignment, tb->palette, enabled, txt, QPalette::ButtonText);

            if (!txt.isEmpty() && option->state & State_HasFocus) {
                QStyleOptionFocusRect opt;
                opt.rect = tr;
                opt.palette = tb->palette;
                opt.state = QStyle::State_None;
                proxy()->drawPrimitive(QStyle::PE_FrameFocusRect, &opt, painter, widget);
            }
            return;
        } break;


    default:
        return Style::drawControl(element, option, painter, widget);
    }
}

int Qt5UKUIStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case PM_ScrollBarExtent:
        return 16;

    case PM_ScrollBarSliderMin:
        return 26;

    case PM_MaximumDragDistance:
        return -1;

    case PM_ScrollView_ScrollBarOverlap:
        return 0;

    case PM_MenuPanelWidth:
        return 0;
    case PM_MenuHMargin:
        return (4 + 5);
    case PM_MenuVMargin:
        return (4 + 5);

    case PM_SmallIconSize:
        return 16;

    case PM_SubMenuOverlap:return 2;

    case PM_DefaultFrameWidth:
        if (qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            return 4;
        }
        return 2;

    case PM_HeaderMargin:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option))
        {
            return 2;
        }
        return 9;
    }

    case PM_MenuBarItemSpacing:return 16;
    case PM_MenuBarVMargin:return 4;
    case PM_ProgressBarChunkWidth: return 9;
    case PM_ToolTipLabelFrameWidth:return 7;

    case PM_SliderTickmarkOffset:
        return 5;
    case PM_SliderLength:
    case PM_SliderThickness:
    case PM_SliderControlThickness:
    {
        return 16;
    }
    case PM_LayoutLeftMargin:
    case PM_LayoutTopMargin:
    case PM_LayoutRightMargin:
    case PM_LayoutBottomMargin:
        return 8;
    case PM_LayoutHorizontalSpacing:
    case PM_LayoutVerticalSpacing:
        return 8;
    case PM_ToolBarFrameWidth:
    case PM_ToolBarItemMargin:
        return 0;
    case PM_ToolBarItemSpacing:
        return 4;

    case PM_MessageBoxIconSize:
        return 24;

    case PM_TabCloseIndicatorWidth:
    case PM_TabCloseIndicatorHeight:
        return 20;

    case PM_TabBarTabHSpace:
        return 8 * 2;
    case PM_TabBarTabVSpace:
        return 4 * 2;

    case PM_TabBarTabOverlap:
        return 1;

    case PM_TabBarScrollButtonWidth:
        return 16;

    case PM_TabBar_ScrollButtonOverlap:
        return 3;

    case PM_TabBarIconSize:
        return 16;

    case PM_TabBarTabShiftVertical:
        return 0;
    case PM_TabBarTabShiftHorizontal:
        return 0;

    case PM_TabBarBaseHeight:
        return 2;

    case PM_TabBarBaseOverlap:
        return 0;

    case PM_ExclusiveIndicatorWidth:
        return 16;
    case PM_ExclusiveIndicatorHeight:
        return 16;
    case PM_RadioButtonLabelSpacing:
        return 8;
    case PM_IndicatorWidth:
        return 16;
    case PM_IndicatorHeight:
        return 16;

    case PM_ButtonIconSize:
        return 16;
    case PM_ButtonMargin:
        return 4;
    case PM_MenuButtonIndicator:
        return 16;
    case PM_ButtonDefaultIndicator:
        return 0;

    case PM_ComboBoxFrameWidth:
        return 2;

    case PM_SpinBoxFrameWidth:
        return 2;

    default:
        break;
    }
    return Style::pixelMetric(metric, option, widget);
}

QRect Qt5UKUIStyle::subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex *option, QStyle::SubControl subControl, const QWidget *widget) const
{
    switch (control) {
    case CC_ScrollBar: {
        if (const QStyleOptionSlider *bar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            const QRect rect = bar->rect;
            const bool horizontal = bar->orientation == Qt::Horizontal;
            int distance = 4;
            int maxlen = horizontal ? rect.width() - distance: rect.height() - distance;
            int sliderlen = 0;
            if (bar->maximum != bar->minimum) {
                uint range = bar->maximum - bar->minimum;
                sliderlen = (qint64(bar->pageStep) * maxlen) / (range + bar->pageStep);

                int slidermin = proxy()->pixelMetric(PM_ScrollBarSliderMin, bar, widget);
                if (sliderlen < slidermin || range > INT_MAX / 2)
                    sliderlen = slidermin;
                if (sliderlen > maxlen)
                    sliderlen = maxlen;
            } else {
                sliderlen = maxlen;
            }

            int sliderstart = sliderPositionFromValue(bar->minimum, bar->maximum, bar->sliderPosition, maxlen - sliderlen, bar->upsideDown) + distance / 2;
            switch (subControl) {
            case SC_ScrollBarSubLine:
            case SC_ScrollBarAddLine:
            case SC_ScrollBarFirst:
            case SC_ScrollBarLast:
                return QRect();

            case SC_ScrollBarSubPage:
            {
                if (horizontal)
                    return QRect(0, 0, sliderstart, rect.height());
                else
                    return QRect(0, 0, rect.width(), sliderstart);
            }

            case SC_ScrollBarAddPage:
            {
                if (horizontal)
                    return QRect(sliderstart + sliderlen, 0, rect.width() - sliderstart - sliderlen, rect.height());
                else
                    return QRect(0, sliderstart + sliderlen, rect.width(), rect.height() - sliderstart - sliderlen);
            }


            case SC_ScrollBarSlider:
            {
                if (horizontal)
                    return QRect(sliderstart, 0, sliderlen, rect.height());
                else
                    return QRect(0, sliderstart, rect.width(), sliderlen);
            }

            case SC_ScrollBarGroove:
            {
                return rect;
            }

            default:
                break;
            }
        }
        break;
    }

    case CC_Slider:
    {
        if(const QStyleOptionSlider* slider = qstyleoption_cast<const QStyleOptionSlider*>(option))
        {
            const bool horizontal = slider->state & State_Horizontal;
            int tickOffset = proxy()->pixelMetric(PM_SliderTickmarkOffset, slider, widget);
            int tickHandle = proxy()->pixelMetric(PM_SliderControlThickness, slider, widget);
            int len = proxy()->pixelMetric(PM_SliderLength, slider, widget);
            int sliderPos = QStyle::sliderPositionFromValue(slider->minimum, slider->maximum, slider->sliderPosition,
                                                            (horizontal ? slider->rect.width() : slider->rect.height()) - len, slider->upsideDown);
            switch (subControl)
            {
            case SC_SliderGroove:
            {
                QRect GrooveRect = slider->rect;
                if (slider->orientation == Qt::Horizontal)
                {
                    if (slider->tickPosition & QSlider::TicksAbove)
                        GrooveRect.adjust(0, tickOffset, 0, 0);
                    if (slider->tickPosition & QSlider::TicksBelow)
                        GrooveRect.adjust(0, 0, 0, -tickOffset);
                }
                else
                {
                    if (slider->tickPosition & QSlider::TicksLeft)
                        GrooveRect.adjust(tickOffset, 0, 0, 0);
                    if (slider->tickPosition & QSlider::TicksRight)
                        GrooveRect.adjust(0, 0, -tickOffset, 0);
                }
                return GrooveRect;
            }
            case SC_SliderHandle:
            {
                QRect rect = slider->rect;
                rect.setHeight(tickHandle);
                rect.setWidth(tickHandle);
                QRect HandleRect = slider->rect;
                if (slider->orientation == Qt::Horizontal)
                {
                    if (slider->tickPosition & QSlider::TicksAbove)
                        HandleRect.adjust(0, tickOffset, 0, 0);
                    if (slider->tickPosition & QSlider::TicksBelow)
                        HandleRect.adjust(0, 0, 0, -tickOffset);
                    rect.moveCenter(QPoint(sliderPos + rect.center().x(), HandleRect.center().ry()));
                }
                else
                {
                    if (slider->tickPosition & QSlider::TicksLeft)
                        HandleRect.adjust(tickOffset, 0, 0, 0);
                    if (slider->tickPosition & QSlider::TicksRight)
                        HandleRect.adjust(0, 0, -tickOffset, 0);
                    rect.moveCenter(QPoint(HandleRect.center().rx(), sliderPos + rect.center().y()));
                }
                return rect;
            }
            case SC_SliderTickmarks:
            {
                break;
            }
            default:
                break;
            }
        }
        break;
    }

    case CC_ToolButton:
    {
        if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            int mbi = proxy()->pixelMetric(PM_MenuButtonIndicator, tb, widget);
            int Button_MarginWidth = proxy()->pixelMetric(PM_ButtonMargin, option, widget);
            QRect rect = tb->rect;
            switch (subControl) {
            case SC_ToolButton:
            {
                if ((tb->features & (QStyleOptionToolButton::MenuButtonPopup | QStyleOptionToolButton::PopupDelay))
                        == QStyleOptionToolButton::MenuButtonPopup)
                    rect.adjust(0, 0, - (mbi + Button_MarginWidth), 0);
                break;
            }
            case SC_ToolButtonMenu:
            {
                if ((tb->features & (QStyleOptionToolButton::MenuButtonPopup | QStyleOptionToolButton::PopupDelay))
                        == QStyleOptionToolButton::MenuButtonPopup)
                    rect.adjust(rect.width() - (mbi + Button_MarginWidth), 0, 0, 0);
                break;
            }
            default:
                break;
            }
            rect = visualRect(tb->direction, tb->rect, rect);
            return rect;
        }
        break;
    }

    case CC_ComboBox:
    {
        if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            int comboBox_Margin = proxy()->pixelMetric(PM_ComboBoxFrameWidth, option, widget);
            int comboBox_MarginWidth = 8;
            int indicator = proxy()->pixelMetric(PM_MenuButtonIndicator, option, widget);
            QRect rect = option->rect.adjusted(comboBox_MarginWidth, comboBox_Margin, -comboBox_MarginWidth, -comboBox_Margin);

            switch (subControl) {
            case SC_ComboBoxArrow:
            {
                QRect arrowRect(rect.right() - indicator, rect.top(), indicator, rect.height());
                return visualRect(option->direction, rect, arrowRect);
            }

            case SC_ComboBoxEditField:
            {
                QRect textRect = option->rect;
                if (cb->editable) {
                    textRect.setRect(rect.left() - 2, rect.top(), rect.width() - indicator + 4, rect.height());
                } else {
                    textRect.setRect(rect.left(), rect.top(), rect.width() - indicator - 8, rect.height());
                }
                return visualRect(option->direction, option->rect, textRect);
            }

            case SC_ComboBoxListBoxPopup:
            {
                return option->rect.adjusted(0, 0, 0, 4);
            }

            default:
                break;
            }
        }
        break;
    }

    case CC_SpinBox:
    {
        if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            int center = sb->rect.height() / 2;
            const int fw = sb->frame ? proxy()->pixelMetric(PM_SpinBoxFrameWidth, sb, widget) : 0;
            const int buttonWidth = 32;
            QRect rect = option->rect;
            switch (subControl) {
            case SC_SpinBoxUp:
            {
                if (sb->buttonSymbols == QAbstractSpinBox::NoButtons)
                    return QRect();
                rect = QRect(rect.right() + 1 - buttonWidth, rect.top(), buttonWidth, center);
                break;
            }
            case SC_SpinBoxDown:
            {
                if (sb->buttonSymbols == QAbstractSpinBox::NoButtons)
                    return QRect();
                rect = QRect(rect.right() + 1 - buttonWidth, rect.top() + center, buttonWidth, center);
                break;
            }
            case SC_SpinBoxEditField:
            {
                if (sb->buttonSymbols == QAbstractSpinBox::NoButtons) {
                    rect = rect.adjusted(fw, 0, -fw, 0);
                } else {
                    rect = rect.adjusted(fw, 0, -(fw + buttonWidth), 0);
                }
                break;
            }
            case SC_SpinBoxFrame:
                rect = sb->rect;

            default:
                break;
            }

            return visualRect(sb->direction, sb->rect, rect);
        }
    }

    default:
        break;
    }

    return Style::subControlRect(control, option, subControl, widget);
}

QRect Qt5UKUIStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    switch (element) {
    case SE_HeaderArrow:
    {
        const int margin = proxy()->pixelMetric(QStyle::PM_HeaderMargin, option, widget);
        QRect rect;
        const int right = option->rect.right();
        const int height = option->rect.height();
        const int top = option->rect.top();
        const int bottom = option->rect.bottom();
        if(option->state & State_Horizontal)
        {
            rect.setRect(right-height+margin,top+margin,option->rect.height()-2*margin,option->rect.height()-2*margin);
        }
        else
        {
            rect.setRect(option->rect.width()+margin,bottom-option->rect.width()+margin,option->rect.width()-2*margin,option->rect.width()-2*margin);
        }
        rect = visualRect(option->direction,option->rect,rect);
        return rect;
    }

    case SE_ProgressBarGroove:
    case SE_ProgressBarContents:
    case SE_ProgressBarLabel:
    {
        if (const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            QRect rect = pb->rect;
            int cw = proxy()->pixelMetric(QStyle::PM_ProgressBarChunkWidth, option, widget);
            if(pb->orientation == Qt::Vertical)
            {
                rect.setRect(pb->rect.left(), pb->rect.top(), cw * 2, rect.height());
            }
            else
            {
                rect.setRect(pb->rect.left(), pb->rect.top(), rect.width(), cw * 2);
            }
            rect.moveCenter(pb->rect.center());
            rect = visualRect(pb->direction, pb->rect, rect);
            return rect;
        }
        break;
    }

    case SE_TabBarScrollLeftButton:
    {
        const bool verticalTabs = option->rect.width() < option->rect.height();
        const int buttonWidth = qMax(proxy()->pixelMetric(QStyle::PM_TabBarScrollButtonWidth, 0, widget), QApplication::globalStrut().width());
        const int lap = proxy()->pixelMetric(QStyle::PM_TabBar_ScrollButtonOverlap, 0, widget);
        QRect rect = verticalTabs ? QRect(-lap, option->rect.height() - (buttonWidth * 2), option->rect.width() + 2 * lap, buttonWidth)
                                  : QStyle::visualRect(option->direction, option->rect, QRect(option->rect.width() - (buttonWidth * 2), -lap,
                                                                                              buttonWidth, option->rect.height() + 2 * lap));
        return rect;
    }

    case SE_TabBarScrollRightButton:
    {
        const bool verticalTabs = option->rect.width() < option->rect.height();
        const int lap = proxy()->pixelMetric(QStyle::PM_TabBar_ScrollButtonOverlap, 0, widget);
        const int buttonWidth = qMax(proxy()->pixelMetric(QStyle::PM_TabBarScrollButtonWidth, 0, widget), QApplication::globalStrut().width());

        QRect rect = verticalTabs ? QRect(-lap, option->rect.height() - buttonWidth, option->rect.width() + 2 * lap, buttonWidth)
                                  : QStyle::visualRect(option->direction, option->rect,
                                                       QRect(option->rect.width() - buttonWidth, -lap, buttonWidth, option->rect.height() + 2 * lap));
        return rect;
    }

    case SE_TabBarTabLeftButton:
    {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            int hpadding = proxy()->pixelMetric(QStyle::PM_TabBarTabHSpace, option, widget) / 2;
            QRect buttonRect(QPoint(0, 0), tab->leftButtonSize);
            switch (tab->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularNorth:
            case QTabBar::TriangularSouth:
            {
                buttonRect.moveTop((tab->rect.height() - buttonRect.height()) / 2);
                buttonRect.moveLeft(tab->rect.left() + hpadding);
                buttonRect = visualRect(tab->direction, tab->rect, buttonRect);
                break;
            }
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            {
                buttonRect.moveLeft((tab->rect.width() - buttonRect.width()) / 2);
                buttonRect.moveBottom(tab->rect.bottom() - hpadding);
                break;
            }
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            {
                buttonRect.moveLeft((tab->rect.width() - buttonRect.width()) / 2);
                buttonRect.moveTop(tab->rect.top() + hpadding);
                break;
            }
            default:
                break;
            }
            return buttonRect;
        }
        break;
    }

    case SE_TabBarTabRightButton:
    {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            int hpadding = proxy()->pixelMetric(QStyle::PM_TabBarTabHSpace, option, widget) / 2;
            QRect buttonRect(QPoint(0, 0), tab->rightButtonSize);
            switch (tab->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularNorth:
            case QTabBar::TriangularSouth:
            {
                buttonRect.moveTop((tab->rect.height() - buttonRect.height()) / 2);
                buttonRect.moveRight(tab->rect.right() - hpadding);
                buttonRect = visualRect(tab->direction, tab->rect, buttonRect);
                break;
            }
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            {
                buttonRect.moveLeft((tab->rect.width() - buttonRect.width()) / 2);
                buttonRect.moveTop(tab->rect.top() + hpadding);
                break;
            }
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            {
                buttonRect.moveLeft((tab->rect.width() - buttonRect.width()) / 2);
                buttonRect.moveBottom(tab->rect.bottom() - hpadding);
                break;
            }
            default:
                break;
            }
            return buttonRect;
        }
        break;
    }

    case SE_TabBarTabText:
    {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            QRect rect = tab->rect;
            QRect iconRect = tab->rect;
            tabLayout(tab, widget, proxy(), &rect, &iconRect);
            return rect;
        }
        break;
    }

    case SE_TabWidgetTabPane:
    {
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            int overlap = proxy()->pixelMetric(PM_TabBarBaseOverlap, option, widget);
            if (twf->lineWidth == 0)
                overlap = 0;
            const QSize tabBarSize(twf->tabBarSize - QSize(overlap, overlap));
            QRect rect(twf->rect);
            switch (twf->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
                rect.adjust(0, tabBarSize.height(), 0, 0);
                break;

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
                rect.adjust(0, 0, 0, -tabBarSize.height());
                break;

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
                rect.adjust(0, 0, -tabBarSize.width(), 0);
                break;

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
                rect.adjust(tabBarSize.width(), 0, 0, 0);
                break;

            default:
                break;
            }
            return rect;
        }
        break;
    }

    case SE_TabWidgetTabContents:
    {
        if (qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            const QRect rect = proxy()->subElementRect(SE_TabWidgetTabPane, option, widget);
            int TabWidget_Margin = 4;
            return rect.adjusted(TabWidget_Margin, TabWidget_Margin, -TabWidget_Margin, -TabWidget_Margin);
        }
        break;
    }

    case SE_TabWidgetTabBar:
    {
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            QRect rect = QRect(QPoint(0, 0), twf->tabBarSize);
            const uint alingMask = Qt::AlignLeft | Qt::AlignRight | Qt::AlignHCenter;
            switch (twf->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
            {
                rect.setWidth(qMin(rect.width(), twf->rect.width() - twf->leftCornerWidgetSize.width() - twf->rightCornerWidgetSize.width()));
                switch (proxy()->styleHint(SH_TabBar_Alignment, twf, widget) & alingMask) {
                case Qt::AlignLeft:
                {
                    rect.moveLeft(twf->leftCornerWidgetSize.width());
                    break;
                }
                case Qt::AlignHCenter:
                {
                    rect.moveLeft((twf->rect.size() - twf->leftCornerWidgetSize - twf->rightCornerWidgetSize - twf->tabBarSize).width() / 2);
                    break;
                }
                case Qt::AlignRight:
                {
                    rect.moveLeft(twf->rect.width() - twf->tabBarSize.width() - twf->rightCornerWidgetSize.width());
                    break;
                }
                default:
                    break;
                }
                rect = visualRect(twf->direction, twf->rect, rect);
                switch (twf->shape) {
                case QTabBar::RoundedSouth:
                case QTabBar::TriangularSouth:
                {
                    rect.moveTop(twf->rect.height() - twf->tabBarSize.height());
                }
                default:
                    break;
                }
                return rect;
            }

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            {
                rect.setHeight(qMin(rect.height(), twf->rect.height()));
                switch (proxy()->styleHint(SH_TabBar_Alignment, twf, widget) & alingMask) {
                case Qt::AlignLeft:
                {
                    rect.moveTopLeft(QPoint(twf->rect.width() - twf->tabBarSize.width(), 0));
                    break;
                }
                case Qt::AlignHCenter:
                {
                    rect.moveTopLeft(QPoint(twf->rect.width() - twf->tabBarSize.width(), (twf->rect.height() - twf->tabBarSize.height()) / 2));
                    break;
                }
                case Qt::AlignRight:
                {
                    rect.moveTopLeft(QPoint(twf->rect.width() - twf->tabBarSize.width(), twf->rect.height() - twf->tabBarSize.height()));
                    break;
                }
                default:
                    break;
                }
                return rect;
            }

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            {
                rect.setHeight(qMin(rect.height(), twf->rect.height()));
                switch (proxy()->styleHint(SH_TabBar_Alignment, twf, widget) & alingMask) {
                case Qt::AlignLeft:
                {
                    rect.moveTop(0);
                    break;
                }
                case Qt::AlignHCenter:
                {
                    rect.moveTop((twf->rect.height() - twf->tabBarSize.height()) / 2);
                    break;
                }
                case Qt::AlignRight:
                {
                    rect.moveTop(twf->rect.height() - twf->tabBarSize.height());
                    break;
                }
                default:
                    break;
                }
                return rect;
            }
            default:
                break;
            }
        }
        break;
    }

    case SE_TabWidgetLeftCorner:
    {
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            QRect paneRect = proxy()->subElementRect(SE_TabWidgetTabPane, twf, widget);
            QRect rect;
            switch (twf->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
                rect = QRect(QPoint(paneRect.x(), paneRect.y() - twf->leftCornerWidgetSize.height()), twf->leftCornerWidgetSize);
                break;
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
                rect = QRect(QPoint(paneRect.x(), paneRect.height()), twf->leftCornerWidgetSize);
                break;
            default:
                break;
            }
            rect = visualRect(twf->direction, twf->rect, rect);
            return rect;
        }
        break;
    }

    case SE_TabWidgetRightCorner:
    {
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
           QRect paneRect = proxy()->subElementRect(SE_TabWidgetTabPane, twf, widget);
           QRect rect;
           switch (twf->shape) {
           case QTabBar::RoundedNorth:
           case QTabBar::TriangularNorth:
               rect = QRect(QPoint(paneRect.width() - twf->rightCornerWidgetSize.width(),
                                paneRect.y() - twf->rightCornerWidgetSize.height()), twf->rightCornerWidgetSize);
               break;
           case QTabBar::RoundedSouth:
           case QTabBar::TriangularSouth:
               rect = QRect(QPoint(paneRect.width() - twf->rightCornerWidgetSize.width(),
                                paneRect.height()), twf->rightCornerWidgetSize);
               break;
           default:
               break;
           }
           rect = visualRect(twf->direction, twf->rect, rect);
           return rect;
        }
        break;
    }

    case SE_RadioButtonIndicator:
    {
        QRect rect;
        int h = proxy()->pixelMetric(PM_ExclusiveIndicatorHeight, option, widget);
        rect.setRect(option->rect.x(), option->rect.y() + ((option->rect.height() - h) / 2),
                  proxy()->pixelMetric(PM_ExclusiveIndicatorWidth, option, widget), h);
        rect = visualRect(option->direction, option->rect, rect);
        return rect;
    }

    case SE_RadioButtonContents:
    {
        int radioWidth = proxy()->pixelMetric(PM_ExclusiveIndicatorWidth, option, widget);
        int spacing = proxy()->pixelMetric(PM_RadioButtonLabelSpacing, option, widget);
        return visualRect(option->direction, option->rect, option->rect.adjusted(radioWidth + spacing, 0, 0, 0));
    }

    case SE_CheckBoxIndicator:
    {
        QRect rect;
        int h = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
        rect.setRect(option->rect.x(), option->rect.y() + ((option->rect.height() - h) / 2),
                     proxy()->pixelMetric(PM_IndicatorWidth, option, widget), h);
        rect = visualRect(option->direction, option->rect, rect);
        return rect;
    }

    case SE_CheckBoxContents:
    {
        int radioWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
        int spacing = proxy()->pixelMetric(PM_RadioButtonLabelSpacing, option, widget);
        return visualRect(option->direction, option->rect, option->rect.adjusted(radioWidth + spacing, 0, 0, 0));
    }

    case SE_PushButtonContents:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            QRect rect = option->rect;
            int Margin_Height = 2;
            int Button_MarginWidth = proxy()->pixelMetric(PM_ButtonMargin, option, widget);
            rect.adjust(Button_MarginWidth, Margin_Height, -Button_MarginWidth, -Margin_Height);
            if (button->features & (QStyleOptionButton::AutoDefaultButton | QStyleOptionButton::DefaultButton)) {
                int dbw = proxy()->pixelMetric(PM_ButtonDefaultIndicator, option, widget);
                rect.adjust(dbw, dbw, -dbw, -dbw);
            }
            return rect;
        }
        break;
    }

    case SE_LineEditContents:
    {
        if (const QStyleOptionFrame *f = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            bool clear = false;
            if (widget) {
                if (qobject_cast<QComboBox *>(widget->parent()))
                    return option->rect;
                if (widget->findChild<QAction *>(QLatin1String("_q_qlineeditclearaction")))
                    clear = true;
            }

            QRect rect = f->rect;
            if (clear) {
                rect.adjust(f->lineWidth + 4, f->lineWidth, 0, -f->lineWidth);
                rect = visualRect(option->direction, option->rect, rect);
            } else {
                rect.adjust(f->lineWidth + 4, f->lineWidth, -(f->lineWidth + 4), -f->lineWidth);
            }

            return rect;
        }
        break;
    }

    default:
        break;
    }

    return Style::subElementRect(element,option,widget);
}



// change control Qsize
QSize Qt5UKUIStyle::sizeFromContents(ContentsType ct, const QStyleOption *option,
                                     const QSize &size, const QWidget *widget) const
{
    QSize  newSize = size;
    switch (ct) {
    case CT_MenuItem: {
        if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            int w = newSize.width();
            int MenuItem_Spacing = 8;
            if (menuItem->text.contains(QLatin1Char('\t')))
                w += 20;

            switch (menuItem->menuItemType) {
            case QStyleOptionMenuItem::SubMenu:
            case QStyleOptionMenuItem::Normal:
            case QStyleOptionMenuItem::DefaultItem:
            {
                if (menuItem->menuHasCheckableItems || menuItem->maxIconWidth != 0) {
                    int iconWidth = proxy()->pixelMetric(QStyle::PM_SmallIconSize, option, widget);
                    w += iconWidth + MenuItem_Spacing;
                    newSize.setHeight(qMax(iconWidth, newSize.height()));
                } else {
                    w += 8;
                }

                w += proxy()->pixelMetric(PM_IndicatorWidth, option, widget) + MenuItem_Spacing;
                newSize.setHeight(qMax(newSize.height(), proxy()->pixelMetric(PM_IndicatorHeight, option, widget)));

                int MenuItem_HMargin = 12 + 4;
                int MenuItem_VMargin = 3;
                w +=  MenuItem_HMargin;
                newSize.setWidth(qMax(w, 152));
                newSize.setHeight(qMax(newSize.height() + MenuItem_VMargin * 2, 30));
                return newSize;
            }

            case QStyleOptionMenuItem::Separator:
            {
                int SepMenuItem_HMargin = 4;
                newSize.setHeight(SepMenuItem_HMargin * 2 + 1);
                return newSize;
            }

            default:
                break;
            }
            return newSize;
        }
        break;
    }

    case CT_LineEdit:
    {
        if (const QStyleOptionFrame *f = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            newSize += QSize(f->lineWidth * 2 + 8, f->lineWidth * 2);
            newSize.setWidth(qMax(newSize.width(), 140));
            newSize.setHeight(qMax(newSize.height(), 36));
            return newSize;
        }
        break;
    }

    case CT_TabBarTab:
    {
        int padding = 0;
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            if (!tab->icon.isNull())
                padding += 4;

            if (!tab->leftButtonSize.isEmpty() || !tab->rightButtonSize.isEmpty())
                padding += 4;

            if (tab->shape == QTabBar::RoundedWest || tab->shape == QTabBar::RoundedEast
                    || tab->shape == QTabBar::TriangularWest || tab->shape == QTabBar::TriangularEast) {
                newSize.setHeight(qMax(newSize.height() + padding, 168));
                newSize.setWidth(qMax(newSize.width(), 36));
            } else {
                newSize.setWidth(qMax(newSize.width() + padding, 168));
                newSize.setHeight(qMax(newSize.height(), 36));
            }
            return newSize;
        }
        break;
    }

    case CT_RadioButton:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            int w = proxy()->pixelMetric(PM_ExclusiveIndicatorWidth, option, widget);
            int h = proxy()->pixelMetric(PM_ExclusiveIndicatorHeight, option, widget);
            int spacing = proxy()->pixelMetric(PM_RadioButtonLabelSpacing, option, widget);
            if (!button->icon.isNull())
                spacing += 4;
            newSize.setWidth(newSize.width() + w + spacing);
            newSize.setHeight(qMax(qMax(newSize.height(), h), 36));
            return newSize;
        }
        break;
    }

    case CT_CheckBox:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            int w = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
            int h = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
            int spacing = proxy()->pixelMetric(PM_RadioButtonLabelSpacing, option, widget);
            if (!button->icon.isNull())
                spacing += 4;
            newSize.setWidth(newSize.width() + w + spacing);
            newSize.setHeight(qMax(qMax(newSize.height(), h), 36));
            return newSize;
        }
        break;
    }

    case CT_ToolButton:
    {
        if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            const bool icon = !tb->icon.isNull();
            const bool text = !tb->text.isNull();
            int w = size.width();
            int h = size.height();
            int Margin_Height = 2;
            int Button_MarginWidth = proxy()->pixelMetric(PM_ButtonMargin, option, widget);
            w += Button_MarginWidth * 2;
            h += Margin_Height * 2;

            if (tb->toolButtonStyle != Qt::ToolButtonIconOnly) {
                QFontMetrics fm = tb->fontMetrics;
                w -= fm.horizontalAdvance(QLatin1Char(' ')) * 2;
                if (tb->toolButtonStyle == Qt::ToolButtonTextBesideIcon) {
                    if (text && icon)
                        w += 4;
                    else
                        w -= 4;
                } else if (tb->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                    if (text && icon)
                        h += 4;
                    else
                        h -= 4;
                }
            }
            if (tb->features & QStyleOptionToolButton::MenuButtonPopup) {
                w += 8;
                newSize.setWidth(w > 60 ? w : 60);
            } else {
                newSize.setWidth(w > 36 ? w : 36);
            }
            newSize.setHeight(h > 36 ? h : 36);
            return newSize;
        }
        break;
    }

    case CT_PushButton:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            const bool icon = !button->icon.isNull();
            const bool text = !button->text.isNull();
            int w = size.width();
            int h = size.height();
            int Margin_Height = 2;
            int Button_MarginWidth = proxy()->pixelMetric(PM_ButtonMargin, option, widget);
            w += Button_MarginWidth * 2;
            h += Margin_Height * 2;

            int spacing = 0;
            if (text && icon)
                spacing += 4;
            if (!text && icon)
                spacing -= 4;
            if (button->features & QStyleOptionButton::HasMenu) {
                if (icon || text)
                    spacing += 8;
            }
            w += spacing;
            if (button->features & (QStyleOptionButton::AutoDefaultButton | QStyleOptionButton::DefaultButton)) {
                int dbw = proxy()->pixelMetric(PM_ButtonDefaultIndicator, option, widget) * 2;
                w += dbw;
                h += dbw;
            }

            newSize.setWidth(w > 96 ? w : 96);
            newSize.setHeight(h > 36 ? h : 36);
            return newSize;
        }
        break;
    }

    case CT_ComboBox:
    {
        if (qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            QSize newSize = size;
            int indicator = proxy()->pixelMetric(PM_MenuButtonIndicator, option, widget);
            int comboBox_Margin = proxy()->pixelMetric(PM_ComboBoxFrameWidth, option, widget);
            int comboBox_MarginWidth = 8 + 8 + 8 + 4;
            newSize.setWidth(qMax(newSize.width() + indicator + comboBox_MarginWidth, 160));
            newSize.setHeight(qMax(newSize.height() + comboBox_Margin, 36));
            return newSize;
        }
        break;
    }

    case CT_SpinBox:
    {
        if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            const int buttonWidth = (sb->subControls & (QStyle::SC_SpinBoxUp | QStyle::SC_SpinBoxDown)) != 0 ? 32 : 0;
            const int fw = sb->frame ? proxy()->pixelMetric(PM_SpinBoxFrameWidth, sb, widget) : 0;
            newSize += QSize(buttonWidth + 2 * fw, 0);
            newSize.setWidth(qMax(newSize.width(), 140));
            newSize.setHeight(qMax(newSize.height(), 36));
            return newSize;
        }
        break;
    }

    default:
        break;
    }

    return QFusionStyle::sizeFromContents(ct, option, size, widget);;
}



void Qt5UKUIStyle::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const
{
    qreal scale = pixmap.devicePixelRatio();
    QRect aligned = alignedRect(QApplication::layoutDirection(), QFlag(alignment), pixmap.size() / scale, rect);
    QRect inter = aligned.intersected(rect);

    QPixmap target = pixmap;

    auto device = painter->device();
    auto widget = dynamic_cast<QWidget *>(device);
    if (widget) {
        if (HighLightEffect::isWidgetIconUseHighlightEffect(widget)) {
            QStyleOption opt;
            opt.initFrom(widget);
            target = HighLightEffect::generatePixmap(pixmap, &opt, widget);
        }
    }

    painter->drawPixmap(inter.x(), inter.y(), target, inter.x() - aligned.x(), inter.y() - aligned.y(), inter.width() * scale, inter.height() *scale);
}



void Qt5UKUIStyle::realSetWindowSurfaceFormatAlpha(const QWidget *widget) const
{
    if (!widget)
        return;

    if (widget->testAttribute(Qt::WA_WState_Created))
        return;

    if (auto menu = qobject_cast<const QMenu *>(widget)) {
        const_cast<QWidget *>(widget)->setAttribute(Qt::WA_TranslucentBackground);
    }

    if (shouldBeTransparent(widget))
        const_cast<QWidget *>(widget)->setAttribute(Qt::WA_TranslucentBackground);
}

void Qt5UKUIStyle::realSetMenuTypeToMenu(const QWidget *widget) const
{
    if (auto menu = qobject_cast<const QMenu *>(widget)) {
        if (!qobject_cast<const QMenu*>(widget)
                || widget->testAttribute(Qt::WA_X11NetWmWindowTypeMenu)
                || !widget->windowHandle())
            return;

        int wmWindowType = 0;
        if (widget->testAttribute(Qt::WA_X11NetWmWindowTypeDropDownMenu))
            wmWindowType |= QXcbWindowFunctions::DropDownMenu;
        if (widget->testAttribute(Qt::WA_X11NetWmWindowTypePopupMenu))
            wmWindowType |= QXcbWindowFunctions::PopupMenu;
        if (wmWindowType == 0) return;
        QXcbWindowFunctions::setWmWindowType(widget->windowHandle(),
                                             static_cast<QXcbWindowFunctions::WmWindowType>(wmWindowType));
    }
}



QRect  Qt5UKUIStyle::centerRect(const QRect &rect, int width, int height) const
{ return QRect(rect.left() + (rect.width() - width)/2, rect.top() + (rect.height() - height)/2, width, height); }
