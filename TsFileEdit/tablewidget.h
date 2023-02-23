#ifndef TABLEWIDGET_H
#define TABLEWIDGET_H

#include <QTableWidget>

class TableWidget : public QTableWidget
{
    Q_OBJECT
public:
    TableWidget( QWidget* parent = nullptr );
    QTableWidgetItem *InsetRowItem( QString unitName, QString source, QStringList translation_list, QString lang, QVariant var );
    void SetBackGroundColor(int row = -1, int col = -1, QString color = "#FFFFFF" );

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
