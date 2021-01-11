#include <QApplication>
#include <QMessageBox>
#include <QMetaEnum>

#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qputenv("QT_QPA_PLATFORMTHEME", "ukui");

#if 0
    QMessageBox::information(nullptr, "title", "information text", QMessageBox::Ok);
    QMessageBox::information(nullptr, "title", "information text1 "
        "information text2 "
        "information text3 "
        "information text4 "
        "information text5 "
        "information text6 "
        "information text7 "
        "information text8 "
        "information text9 "
        "information text10 "
        "information text11 "
        "information text12 "
        "information text13 "
        "information text14 "
        "information text15 "
        "information text16 "
        "information text17 "
        "information text18 "
        "information text19 "
        "information text20 "
        "information text21 "
        "information text22 "
        "information text23 "
        "information text24 "
        "information text25 "
        "information text26 "
        "information text27 "
        "information text28 "
        "information text29 "
        "information text30 "
        "information text"
        "information text"
        "information text"
        "information text"
        "information text"
        "information text"
        "information text"
        "information text"
        "information text"
        "information text"
        "information text"
        "information text"
        "information text"
        "information text"
        "information text"
        "information text"
        "information text"
        "information text"
        "information text", QMessageBox::Ok);
    QMessageBox::information(nullptr, "title", "information text", QMessageBox::Ok | QMessageBox::Cancel | QMessageBox::Save | QMessageBox::SaveAll);

    //
    QMessageBox::warning(nullptr, "title", "warning text", QMessageBox::Ok);
    QMessageBox::critical(nullptr, "title", "critical text", QMessageBox::Ok);
    QMessageBox::question(nullptr, "title", "question text", QMessageBox::Ok);


    // test button
    QMetaEnum metaEnum = QMetaEnum::fromType<QMessageBox::StandardButtons>();
    for (auto i = 0; i < metaEnum.keyCount(); ++i) {
        if (metaEnum.value(i) < QMessageBox::FirstButton
                || metaEnum.value(i) > QMessageBox::LastButton) {
            continue;
        }

        int ret = QMessageBox::information(nullptr, "title", "测试按钮", metaEnum.value(i));

        qDebug() << "is current button:" << (ret == metaEnum.value(i)) << "  ret:" << ret;
    }
#endif
#if 0

    QMessageBox m;
    m.setText("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    int ret = m.exec();
    qDebug() << (ret == QDialog::Accepted) << " --- " << ret;
#endif

#if 1
    QMessageBox m;
    m.setText("这是中文测试1，中文测试2，中文测试3，中文测试4，中文测试5，中文测试6，中文测试7，中文测试8，中文测试9，中文测试10，中文测试11，中文测试12，中文测试13，中文测试14");
    int ret = m.exec();
    m.setIcon(QMessageBox::Critical);
    m.exec();
    m.setText("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    m.exec();
    qDebug() << (ret == QDialog::Accepted) << " --- " << ret;
#endif
    return a.exec();
}
