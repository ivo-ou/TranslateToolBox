#include "tablewidget.h"
#include <QHeaderView>
#include <QEvent>
#include <QApplication>
#include <QLabel>
#include <QClipboard>
#include <QToolTip>
#include "sharefunction.h"
#include "seltlwidget.h"

#define c1 2 // 每列比例
#define c2 3
#define c3 5

TableWidget::TableWidget( QWidget* parent )
    : QTableWidget( parent )
    , m_first( true )
{
    setColumnCount( 3 );
    QStringList label;
    label << "来源"
          << "原文"
          << "翻译";
    this->setHorizontalHeaderLabels( label );
    this->horizontalHeader()->setFixedHeight( 30 );
    this->horizontalHeader()->setStretchLastSection( true );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff ); // 不显示横向滚动条
    setEditTriggers( QAbstractItemView::NoEditTriggers );
    verticalHeader()->setVisible( false );
    setSelectionBehavior( QAbstractItemView::SelectRows ); // 按行选择
    setRowCount( 0 );
    setMouseTracking( true );
    connect( this, &QTableWidget::entered, this, &TableWidget::ShowTooltip );
    connect( this, &QTableWidget::itemClicked, this, &TableWidget::CopyText );
    connect( this, &QTableWidget::itemDoubleClicked, this, &TableWidget::EditText );
}

bool TableWidget::event( QEvent* e )
{
    if ( e->type() == QEvent::Show && m_first )
    {
        int width = this->width();
        double p_width = width / 10;
        setColumnWidth( 0, int( p_width * c1 ) );
        setColumnWidth( 1, int( p_width * c2 ) );
        setColumnWidth( 2, int( p_width * c3 ) );
        update();
    }
    return QTableWidget::event( e );
}

void TableWidget::InsetRowItem( QString unitName, QString source, QStringList translation_list, QString lang, QVariant var )
{
    int row = rowCount();
    setRowCount( row + 1 );
    setItem( row, 0, new QTableWidgetItem( unitName ) );
    setItem( row, 1, new QTableWidgetItem( source ) );
    if ( translation_list.size() == 0 )
        setItem( row, 2, new QTableWidgetItem( "" ) );
    else
        setItem( row, 2, new QTableWidgetItem( translation_list.at( 0 ) ) ); // 选中的翻译
    item( row, 2 )->setData( Qt::UserRole, translation_list );               // 翻译候选列表
    item( row, 2 )->setData( Qt::UserRole + 1, lang );                       // 语言
    item( row, 2 )->setData( Qt::UserRole + 2, var );                        // 节点
    update();
    qApp->processEvents();
}

void TableWidget::SetBackGroundColor( int row, int col, QColor color )
{
    if( row == -1 )
        row = rowCount() - 1;
    if( col == -1 )
        col = columnCount() - 1;
    if( item( row, col ) )
        item( row, col)->setData( Qt::BackgroundRole, color );
}

void TableWidget::CopyText( QTableWidgetItem* item )
{
    QString txt = item->text();
    if ( txt.isEmpty() )
        return;
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText( txt );
    QToolTip::showText( QCursor().pos(), "已复制:" + txt, this );
}

void TableWidget::EditText( QTableWidgetItem* item )
{
    if ( item->column() != 2 )
        return;
    QString translate = item->text(); // 选中的翻译
    QString source;             // 原文
    if ( this->item( item->row(), 1 ) )
        source = this->item( item->row(), 1 )->text();
    QStringList candidate_list = item->data( Qt::UserRole ).toStringList(); // 候选的翻译
    candidate_list.removeAll("");
    QString langto = item->data( Qt::UserRole + 1 ).toString();

    // 翻译选中弹窗
    SelTLWidget dlg( source, translate, langto, candidate_list );
    if ( dlg.exec() == QDialog::Rejected )
        return;
    QPair< QString, QStringList > ret = dlg.getTranslate();
    item->setData( Qt::UserRole, ret.second ); // 候选翻译列表
    item->setText( ret.first );                // 选中的翻译

    //  刷新ts文件
    auto xml_node = item->data( Qt::UserRole + 2 ).value< tinyxml2::XMLElement* >();
    xml_node->SetText( ret.first.toUtf8().data() );
    if ( ret.first.isEmpty() )
        xml_node->SetAttribute( "type", "unfinished" );
    else
        xml_node->DeleteAttribute( "type" );
    emit editFinished();
}

void TableWidget::ShowTooltip( QModelIndex index )
{
    QToolTip::showText( QCursor::pos(), index.data().toString(), this );
    return;
}
