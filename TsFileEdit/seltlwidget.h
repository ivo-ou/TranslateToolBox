#ifndef SELTLWIDGET_H
#define SELTLWIDGET_H

#include <QDialog>
#include <QLineEdit>

class QButtonGroup;
class QCheckBox;

class SelTLWidget : public QDialog
{
    Q_OBJECT
public:
    SelTLWidget(QString source, QString translate, QString langto, QStringList TL = QStringList(),  QWidget* parent = nullptr );
    QPair<QString, QStringList> getTranslate();

private slots:
    void Translate();

private:
    QMap< int, QLineEdit*> m_linE_map;
    QButtonGroup* m_btnGroup;
    QString m_source, m_langto;
};

#endif // SELTLWIDGET_H
