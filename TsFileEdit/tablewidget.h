﻿#ifndef TABLEWIDGET_H
#define TABLEWIDGET_H

#include <QTableWidget>

class TableWidget : public QTableWidget
{
    Q_OBJECT
public:
    TableWidget( QWidget* parent = nullptr );
    void InsetRowItem( QString unitName, QString source, QStringList translation_list, QString lang, QVariant var );

signals:
    void editFinished();
private slots:
    void CopyText( QTableWidgetItem* item );
    void EditText( QTableWidgetItem* item );
    void ShowTooltip( QModelIndex index );

protected:
    bool event( QEvent* e );

private:
    bool m_first;
};

#endif // TABLEWIDGET_H