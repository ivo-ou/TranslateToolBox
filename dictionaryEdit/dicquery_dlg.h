#ifndef DICQUERY_DLG_H
#define DICQUERY_DLG_H

#include <QDialog>

namespace Ui {
class DicQuery_Dlg;
}

class DicQuery_Dlg : public QDialog
{
    Q_OBJECT

public:
    explicit DicQuery_Dlg(QWidget *parent = nullptr);
    ~DicQuery_Dlg();

signals:
    void query( QString, int );

private slots:
    void on_lineEdit_content_editingFinished();

    void on_pushButton_pre_clicked();

    void on_pushButton_next_clicked();

private:
    Ui::DicQuery_Dlg *ui;
};

#endif // DICQUERY_DLG_H
