﻿#ifndef DICEDIT_H
#define DICEDIT_H

#include <QWidget>
#include <QStandardItemModel>
#include "sharefunction.h"

namespace Ui {
class DicEdit;
}

class DicEdit : public QWidget
{
    Q_OBJECT

public:
    explicit DicEdit(QWidget *parent = nullptr);
    ~DicEdit();

private:


private slots:
    void initModel();
    void Seldir();                       // 路径选择
    void ImportFromTS();
    void OutputToXlsx();
    void InsetUnit( TLUnit unit);
    void OutputModel();
    void Query( QString q, int index );

    void on_pushButton_del_clicked();

    void on_pushButton_search_clicked();

private:
    Ui::DicEdit *ui;
    QStandardItemModel* m_model;
    QMap< QString,int> m_header_map;
    QList<QModelIndex> m_query_items;
    int m_cur_index;
};

#endif // DICEDIT_H
