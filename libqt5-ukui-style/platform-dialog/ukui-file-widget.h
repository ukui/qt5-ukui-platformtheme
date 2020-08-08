#ifndef UKUIFILEWIDGET_H
#define UKUIFILEWIDGET_H



#include <QObject>
#include <QWidget>
#include <QUrl>



class UKUIFileWidgetPrivate;
class UKUIFileWidget : public QWidget
{
    Q_OBJECT
public:
    enum OperationMode {
        Other = 0,
        Opening,
        Saving };
    explicit UKUIFileWidget(const QUrl &startDir, QWidget *parent = nullptr);
    ~UKUIFileWidget() override;
    static QUrl getStartUrl(const QUrl &startDir, QString &recentDirClass, QString &fileName);
    void setOperationMode(OperationMode mode);
    OperationMode operationMode() const;

private:
    friend class UKUIFileWidgetPrivate;
    UKUIFileWidgetPrivate *const d;
    OperationMode operationmode;
};

#endif // UKUIFILEWIDGET_H
