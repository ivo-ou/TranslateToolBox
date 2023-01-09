#ifndef CHECKBTN_H
#define CHECKBTN_H

#include <QWidget>
#include <QVariant>

class QPushButton;
class QCheckBox;
class QButtonGroup;

class CheckBtn : public QWidget
{
    Q_OBJECT
public:
    explicit CheckBtn(QWidget *parent = nullptr);
    explicit CheckBtn(int id, QString name,QWidget *parent = nullptr);
    bool isChecked();
    void setChkEnabled( bool enabled);
    void setData( QVariant data);
    QVariant getData();
    int getID();
    void setButtonGroup( QButtonGroup* g);

signals:
    void clicked( int);

public slots:
private:
    QPushButton *m_btn;
    QCheckBox *m_chk;
    QVariant m_data;
    int m_id;
};

#endif // CHECKBTN_H
