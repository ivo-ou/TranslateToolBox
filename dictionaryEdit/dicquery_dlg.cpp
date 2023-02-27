#include "dicquery_dlg.h"
#include "ui_dicquery_dlg.h"

DicQuery_Dlg::DicQuery_Dlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DicQuery_Dlg)
{
    ui->setupUi(this);
    this->setWindowFlag( Qt::Tool );
}

DicQuery_Dlg::~DicQuery_Dlg()
{
    delete ui;
}

void DicQuery_Dlg::on_lineEdit_content_editingFinished()
{
    emit query( ui->lineEdit_content->text(), 0 );
}

void DicQuery_Dlg::on_pushButton_pre_clicked()
{
    emit query( "", -1 );
}

void DicQuery_Dlg::on_pushButton_next_clicked()
{
    emit query( "", 1 );
}
