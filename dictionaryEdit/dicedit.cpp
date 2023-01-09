#include "dicedit.h"
#include "ui_dicedit.h"
#include "xlsxdocument.h"

#include <QFileDialog>
//#include <select

DicEdit::DicEdit( QWidget* parent )
    : QWidget( parent )
    , ui( new Ui::DicEdit )
{
    ui->setupUi( this );
    this->setWindowTitle( "翻译字典维护" );
    m_model = new QStandardItemModel( this );
    ui->tableView->setSelectionBehavior( QTableView::SelectRows );
    ui->tableView->verticalHeader()->setVisible( false );
    ui->widget_func->setEnabled( false );
    ui->pushButton_save->setEnabled( false );
    ui->pushButton_restore->setEnabled( false );
    ui->pushButton_redo->setEnabled( false );
    ui->pushButton_import->setEnabled( false );
    connect( ui->pushButton_dir_sel, &QPushButton::clicked, this, &DicEdit::Seldir );
    connect( ui->pushButton_import, &QPushButton::clicked, this, &DicEdit::ImportFromTS );
    connect( ui->pushButton_output_model, &QPushButton::clicked, this, &DicEdit::OutputModel );
    connect( ui->pushButton_save, &QPushButton::clicked, this, &DicEdit::OutputToXlsx );
    connect( ui->lineEdit_dir, &QLineEdit::editingFinished, this, &DicEdit::initModel );

    ui->pushButton_search->setEnabled( false );
    ui->pushButton_redo->setEnabled( false );
    ui->pushButton_restore->setEnabled( false );
}

DicEdit::~DicEdit()
{
    delete ui;
}

void DicEdit::initModel()
{
    ui->lineEdit_dir->blockSignals( true );
    ui->lineEdit_dir->clearFocus();
    ui->lineEdit_dir->blockSignals( false );
    ui->label_tip->clear();
    m_model->clear();
    QString file_path = ui->lineEdit_dir->text();
    QFileInfo fileInfo( file_path );
    ui->widget_func->setEnabled( true );
    ui->pushButton_save->setEnabled( true );
    ui->pushButton_restore->setEnabled( true );
    ui->pushButton_redo->setEnabled( true );
    ui->pushButton_import->setEnabled( true );
    if ( !fileInfo.isFile() )
    {
        ui->label_tip->setText( "文件不存在，请检查目录！" );
        ui->widget_func->setEnabled( false );
        ui->pushButton_save->setEnabled( false );
        ui->pushButton_restore->setEnabled( false );
        ui->pushButton_redo->setEnabled( false );
        ui->pushButton_import->setEnabled( false );
        return;
    }
    QTableView* table = ui->tableView;
    table->setModel( m_model );
    //获取表格，选择表单，获取大小
    QXlsx::Document xlsx( file_path );
    if ( xlsx.sheetNames().size() == 0 )
    {
        ui->label_tip->setText( "文件打开失败，请重新选择!" );
        return;
    }
    xlsx.selectSheet( xlsx.sheetNames().at( 0 ) );
    QXlsx::CellRange range = xlsx.dimension();

    //设置model大小
    m_model->setColumnCount( range.columnCount() );
    m_model->setRowCount( range.rowCount() - 1 ); //去掉表头一行

    //插入表头
    for ( int i = 0; i < m_model->columnCount(); ++i )
        if ( xlsx.cellAt( 1, i + 1 ) )
        {
            QString header = xlsx.cellAt( 1, i + 1 )->value().toString();
            m_model->setHeaderData( i, Qt::Horizontal, header );
            m_header_map[ header ] = i;
        }

    //插入内容
    for ( int row = 0; row < m_model->rowCount(); ++row )
        for ( int col = 0; col < m_model->columnCount(); ++col )
        {
            qApp->processEvents( QEventLoop::ExcludeUserInputEvents );
            if ( xlsx.cellAt( row + 2, col + 1 ) )
                m_model->setData( m_model->index( row, col ), xlsx.cellAt( row + 2, col + 1 )->value().toString() );
        }
}

void DicEdit::Seldir()
{
    QString file_path = QFileDialog::getOpenFileName( this, "选择字典", qApp->applicationDirPath(), "Excel(*.xlsx)" );
    if ( file_path.isEmpty() )
        return;
    ui->lineEdit_dir->setText( file_path );
    initModel();
}

void DicEdit::ImportFromTS()
{
    if ( m_header_map[ "序号" ] != 0 || m_header_map[ "中文" ] != 1 )
    {
        ui->label_tip->setText( "请检查字典格式，第一列为“序号”，第二列为“中文”" );
        return;
    }
    QString file = QFileDialog::getOpenFileName( this, "选择TS文件", qApp->applicationDirPath(), "*.ts" );
    if ( file.isEmpty() )
        return;
    tinyxml2::XMLDocument* doc = new tinyxml2::XMLDocument();
    if ( !loadTS( doc, file ) )
    {
        ui->label_tip->setText( QString( "%1文件打开失败！" ).arg( file.split( "/" ).back() ) );
        return;
    }
    qRegisterMetaType< std::function< void( TLUnit ) > >( "std::function< void( TLUnit ) >" );
    RorWXML( doc, std::bind( &DicEdit::InsetUnit, this, std::placeholders::_1 ) );
    ui->label_tip->setText( "导入完成" );
}

/*
 * 原文不存在 - 插入尾行
 * 原文存在，翻译不同 - 插入尾行
 * 原文存在，翻译相同 - 跳过插入
 * 原文存在，翻译为空 - 插入至当前行
 */
void DicEdit::InsetUnit( TLUnit unit )
{
    QMap< QString, QString > lang_zh = Config::getInstance().getLang_zh();
    if ( !lang_zh.contains( unit.Language ) )
    {
        ui->label_tip->setText( "无法匹配" + unit.Language + "语言，请检查配置文件" );
        qDebug() << "配置文件不存在当前语言" << unit.Language;
        return;
    }
    QString lang = lang_zh[ unit.Language ];
    if ( !m_header_map.contains( lang ) )
    {
        // 字典中不存在导入的语言，新增列
        m_model->setHeaderData( m_model->columnCount() + 1, Qt::Horizontal, lang );
        m_model->setColumnCount( m_model->columnCount() + 1 );
        m_header_map.insert( lang, m_header_map.size() );
        qDebug() << "字典不存在语言" << lang << "新增列";
    }
    int column = m_header_map[ lang ];
    qDebug() << "unit language" << lang << unit.Language;
    for ( auto source : unit.Content.keys() )
    {
        QString translate = unit.Content.value( source )->GetText();
        auto list = m_model->findItems( source, Qt::MatchExactly, 1 );
        QVector< int > index_vec; // 中文所在行数索引
        bool exist = false;       // 是否存在一致的翻译
        for ( auto l : list )
        {
            if ( !l )
                continue;
            if ( l->text() == source )
                index_vec.push_back( l->row() );
        }
        for ( auto index : index_vec )
        {
            auto item = m_model->item( index, column ); // 表格中对应的翻译
            // 有中文，但翻译为空，直接插入
            if ( item->text().isEmpty() )
            {
                m_model->item( index, column )->setText( translate );
                qDebug() << "有中文，但翻译为空，插入:" << index << m_model->item( index, m_header_map[ "中文" ] )->text() << translate;
                exist = true;
                break;
            }
            if ( item->text() == translate )
            {
                exist = true;
                break;
            }
        }
        if ( !exist )
        {
            QList< QStandardItem* > items;
            const int col = m_model->columnCount();
            for ( int i = 0; i < col; i++ )
                items.append( new QStandardItem() );
            items[ 0 ]->setText( QString::number( m_model->rowCount() + 1 ) ); // 序号
            auto last_item = m_model->item( m_model->rowCount() );
            if ( last_item )
                items[ 0 ]->setText( QString::number( last_item->text().toInt() + 1 ) );
            items[ m_header_map[ "中文" ] ]->setText( source ); // 中文
            items[ column ]->setText( translate );              // 翻译
            m_model->appendRow( items );
            qDebug() << "插入:" << items[ 0 ]->text() << source << translate;
            continue;
        }
        qDebug() << "已存在相同原文与翻译，跳过" << source << translate;
    }
}

void DicEdit::OutputToXlsx()
{
    QXlsx::Document xlsx;
    int rowCount = m_model->rowCount();
    int colCount = m_model->columnCount();
    // 导出表头
    for ( int i = 0; i < colCount; i++ )
    {
        if ( !m_model->horizontalHeaderItem( i ) )
            continue;
        QString header = m_model->horizontalHeaderItem( i )->text();
        xlsx.write( 1, 1 + i, header );
    }
    for ( auto row = 0; row < rowCount; row++ )
    {
        for ( int col = 0; col < colCount; col++ )
        {
            if ( !m_model->item( row, col ) )
                continue;
            xlsx.write( row + 2, col + 1, m_model->item( row, col )->text() ); //从第二行开始写
        }
    }
    bool success = false;
    threadExec( [&] { success = xlsx.saveAs( ui->lineEdit_dir->text() ); } );
    if ( success )
    {
        qDebug() << "字典保存成功！";
        ui->label_tip->setText( "字典保存成功!" );
    }
    else
    {
        qDebug() << "字典保存失败！";
        ui->label_tip->setText( "字典保存失败!" );
    }
}

void DicEdit::OutputModel()
{
    QStringList header;
    QString file_path = QFileDialog::getExistingDirectory( this, "选择导出路径", qApp->applicationDirPath() );
    if ( file_path.isEmpty() )
        return;
    QMap< QString, QString > lang_map = Config::getInstance().getLang_zh();
    header << "序号"
           << "中文";
    for ( auto lang : lang_map )
    {
        if( lang =="中文")
            continue;
        header << lang + "最大字符";
        header << lang;
        header << lang + "全写";
    }
    QXlsx::Document xlsx;
    bool success = false;
    // 导出表头
    for ( int i = 0; i < header.size(); i++ )
    {
        xlsx.write( 1, 1 + i, header.at( i ) );
    }
    threadExec( [&] { success = xlsx.saveAs( file_path+"/model.xlsx" ); } );
    if ( success )
    {
        qDebug() << "字典模板保存成功！";
        ui->label_tip->setText( "字典模板保存成功!" );
    }
    else
    {
        qDebug() << "字典模板保存失败！";
        ui->label_tip->setText( "字典模板保存失败!" );
    }
}

void DicEdit::on_pushButton_del_clicked()
{
    QItemSelectionModel* select_model = ui->tableView->selectionModel();
    if ( !select_model->hasSelection() )
        return;
    QList< int > sel_rows;
    for ( auto sel : select_model->selectedRows() )
    {
        sel_rows.append( sel.row() );
    }
    std::sort( sel_rows.begin(), sel_rows.end(), []( int a, int b ) { return a > b; } ); // 降序排序
    while ( sel_rows.size() )
    {
        m_model->removeRow( sel_rows[ 0 ] );
        sel_rows.pop_front();
    }
    qDebug() << "删除行";
}
