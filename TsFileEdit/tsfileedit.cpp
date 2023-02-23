#include "tsfileedit.h"
#include "ui_tsfileedit.h"
#include "baidutranslateapi.h"
#include "QtXlsx/xlsxdocument.h"

#include <QFileDialog>
#include <QMessageBox>

TsFileEdit::TsFileEdit( QWidget* parent )
    : QWidget( parent )
    , ui( new Ui::TsFileEdit )
    , m_dic_vec( nullptr )
{
    ui->setupUi( this );
    setWindowTitle( "TS编辑" );
    InitUI();
    connect( ui->pushButton_sel_dic, &QPushButton::clicked, this, &TsFileEdit::SelDicdir );
    connect( ui->lineEdit_dir_dic, &QLineEdit::editingFinished, this, &TsFileEdit::LoadDic );
    connect( ui->pushButton_sel_ts, &QPushButton::clicked, this, &TsFileEdit::SelTsdir );
    connect( ui->lineEdit_dir_ts, &QLineEdit::editingFinished, this, &TsFileEdit::ParseTsDir );
    connect( ui->pushButton_ok, &QPushButton::clicked, this, &TsFileEdit::LoadTs2Table );
}

TsFileEdit::~TsFileEdit()
{
    Config::getInstance().setFilterWords( ui->lineEdit_exclude_words->text() ); // 更新过滤词
    delete ui;
}

void TsFileEdit::InitUI()
{
    ui->label_tip->clear();
    m_stackedWidget = new QStackedWidget( ui->widget_5 );
    ui->verticalLayout_2->addWidget( m_stackedWidget );
    ui->lineEdit_exclude_words->setText( Config::getInstance().getFilterWords() );
    ClearWidget();
    //    SetWidgetState( false );
}

// 加载字典
// 格式   序号 | 中文 | A语言 | B语言 | C语言 | D语言
void TsFileEdit::LoadDic()
{
    ui->label_tip->clear();
    m_header_index.clear();
    m_dic_vec = new QMultiHash< QString, QVector< QString > >();
    const QString file_path = ui->lineEdit_dir_dic->text();
    const QFileInfo fileInfo( file_path );
    if ( !fileInfo.isFile() )
    {
        ui->label_tip->setText( "字典文件不存在，请检查目录！" );
        return;
    }
    //获取表格，选择表单，获取大小
    QXlsx::Document xlsx( file_path );
    if ( xlsx.sheetNames().size() == 0 )
    {
        ui->label_tip->setText( "字典文件打开失败，请重新选择!" );
        return;
    }
    xlsx.selectSheet( xlsx.sheetNames().at( 0 ) );
    const QXlsx::CellRange range = xlsx.dimension();
    const int rowCount = range.rowCount();
    const int colCount = range.columnCount();
    const QMap< QString, QString > lang = Config::getInstance().getLang();
    const QMap< QString, QString > lang_zh = Config::getInstance().getLang_zh();
    // 若表格第二列不为中文，提示格式错误;
    if ( !xlsx.cellAt( 1, 2 ) || xlsx.cellAt( 1, 2 )->value().toString() != lang_zh[ "zh_CN" ] )
    {
        qDebug() << xlsx.cellAt( 1, 2 )->value().toString() << lang_zh[ "zh_CN" ];
        ui->label_tip->setText( "字典文件格式错误，请检查！" );
        QMessageBox::information( this, "错误", QString( "%1字典文件格式错误，请检查！" ).arg( file_path ) );
        return;
    }
    // 读取表头
    for ( int col = 1; col <= colCount; col++ )
    {
        if ( xlsx.cellAt( 1, col ) )
            for ( auto l : lang_zh )
                if ( l.compare( xlsx.cellAt( 1, col )->value().toString() ) == 0 )
                {
                    qDebug() << "识别到语言：" << l << m_header_index.size();
                    m_header_index.insert( m_header_index.size(), qMakePair( l, col ) );
                }
    }
    if ( m_header_index.size() == 0 )
    {
        ui->label_tip->setText( "字典文件格式错误，请检查！" );
        return;
    }
    for ( int row = 2; row <= rowCount; row++ )
    {
        qApp->processEvents();
        QString source;
        if ( !xlsx.cellAt( row, 2 ) ) // 判断中文是否存在
            continue;
        source = xlsx.cellAt( row, 2 )->value().toString();
        qDebug() << "字典： " << source;
        auto iter = m_dic_vec->insert( source, QVector< QString >() ); //新增原文
        // 遍历插入翻译
        for ( auto pair : m_header_index )
        {
            if ( xlsx.cellAt( row, pair.second ) )
                iter.value().push_back( xlsx.cellAt( row, pair.second )->value().toString() );
        }
    }
    ui->label_tip->setText( "字典加载完成！" );
}

// 生成左侧按钮与右侧对于翻译列表
void TsFileEdit::InitLangList( QStringList files )
{
    int id = 0;
    QButtonGroup* group = new QButtonGroup( this );
    group->setExclusive( true );
    for ( auto name : files )
    {
        QString btn_name = name;
        CheckBtn* btn = new CheckBtn( id, btn_name.replace( ".ts", "" ), ui->widget_container );
        btn->setData( ui->lineEdit_dir_ts->text() + "/" + name );
        btn->setButtonGroup( group );
        connect( btn, &CheckBtn::clicked, m_stackedWidget, &QStackedWidget::setCurrentIndex );
        m_chkBtn_vec.push_back( btn );
        ui->verticalLayout_lang_btn->addWidget( btn );
        TableWidget* tbl = new TableWidget( m_stackedWidget );
        // 表格修改后自动刷新ts文件
        connect( tbl, &TableWidget::editFinished, this, [&] {
            // 保存修改
            int id = m_stackedWidget->currentIndex();
            qDebug() << "current stackedwidget id: " << id;
            if ( !m_xmlDoc_map.contains( id ) )
                return;
            if ( m_xmlDoc_map[ id ]->SaveFile( m_chkBtn_vec[ id ]->getData().toString().toLocal8Bit().toStdString().c_str() ) == tinyxml2::XMLError::XML_SUCCESS )
            {
                qDebug() << "更新成功" << m_chkBtn_vec[ id ]->getData().toString();
                if ( m_tblWidget_vec[ id ] )
                {
                    QModelIndex index = m_tblWidget_vec[ id ]->currentIndex();
                    if ( index.row() == -1 )
                        return;
                    m_tblWidget_vec[ id ]->SetBackGroundColor( index.row(), 2 );
                }
            }
            else
                qDebug() << "更新失败" << m_chkBtn_vec[ id ]->getData().toString();
        } );
        m_tblWidget_vec.push_back( tbl );
        m_stackedWidget->insertWidget( id, tbl );
        id++;
    }

    ui->verticalLayout_lang_btn->addSpacerItem( new QSpacerItem( 10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding ) );
}

// 解析目录
void TsFileEdit::ParseTsDir()
{
    ui->lineEdit_dir_ts->blockSignals( true );
    ui->lineEdit_dir_ts->clearFocus();
    ui->lineEdit_dir_ts->blockSignals( false );
    ClearWidget();
    QString dir_path = ui->lineEdit_dir_ts->text();
    if ( !dir_path.isEmpty() )
    {
        ui->lineEdit_dir_ts->setText( dir_path );
        QDir dir( dir_path );
        QStringList nameFilters;
        nameFilters << "*.ts";
        QStringList files = dir.entryList( nameFilters, QDir::Files | QDir::Readable, QDir::Name );
        if ( files.size() == 0 )
        {
            ui->label_tip->setText( "未识别到文件，请检查目录！" );
        }
        else
        {
            ui->label_tip->setText( QString( "识别到%1个文件" ).arg( files.size() ) );
            // 生成左侧按钮与右侧对于翻译列表
            InitLangList( files );
        }
    }
    qApp->postEvent( this, new QEvent( QEvent::Resize ) );
}

void TsFileEdit::ClearWidget()
{
    //清空布局内的所有元素
    m_chkBtn_vec.clear();
    m_tblWidget_vec.clear();
    m_xmlDoc_map.clear();
    QLayoutItem* child;
    while ( ( child = ui->verticalLayout_lang_btn->takeAt( 0 ) ) != nullptr )
    {
        //setParent为NULL，防止删除之后界面不消失
        if ( child->widget() )
        {
            child->widget()->setParent( nullptr );
            delete child->widget(); //释放
        }

        delete child;
    }
    ui->verticalLayout_2->removeWidget( m_stackedWidget );
    m_stackedWidget = new QStackedWidget( ui->widget_5 );
    ui->verticalLayout_2->addWidget( m_stackedWidget );
}

void TsFileEdit::SetWidgetState( bool start )
{
    ui->pushButton_ok->setEnabled( !start );
    ui->widget_func->setEnabled( !start );
    ui->widget_dir->setEnabled( !start );
    for ( auto btn : m_chkBtn_vec )
    {
        btn->setChkEnabled( !start );
    }
    for ( auto tbl : m_tblWidget_vec )
        tbl->blockSignals( start );
}

// 读取xml回调函数
void TsFileEdit::ReadUnit( TLUnit unit )
{
    QStringList ign_words = ui->lineEdit_exclude_words->text().split( "//" ); // 过滤词
    for ( auto source : unit.Content.keys() )
    {
        QStringList translation_list; // 存放候选翻译

        // 获取TS中本身的翻译
        if ( unit.Content.value( source ) )
        {
            if ( !QString( unit.Content.value( source )->GetText() ).isEmpty() )
                translation_list << unit.Content.value( source )->GetText();
        }

        match_result result;
        // （翻译为空或不忽略已存在翻译）并且不在忽略词中，则进行翻译匹配
        if ( ( translation_list.size() == 0 || !ui->checkBox_exist_ign->isChecked() ) && !ign_words.contains( source ) )
        {
            qDebug() << " ";
            result = TranslateMatch( Config::getInstance().getLang()[ unit.Language ], source, translation_list, true );
            qDebug() << " ";

            if ( translation_list.size() )
            {
                unit.Content.value( source )->SetText( translation_list.at( 0 ).toUtf8().data() );
                //                unit.Content.value( source )->SetText("");        // 清空翻译
            }
            // 不是完全匹配，则标记unfinished
            if ( result.ret != match_ret::Success && result.ret != match_ret::Ignore )
            {
                unit.Content.value( source )->SetAttribute( "type", "unfinished" );
                if ( result.ret != match_ret::Fail )
                {
                    tinyxml2::XMLElement* comment = unit.Content.value( source )->Parent()->FirstChildElement( "translatorcomment" );
                    if ( !comment )
                    {
                        comment = unit.Content.value( source )->GetDocument()->NewElement( "translatorcomment" );
                        unit.Content.value( source )->Parent()->InsertEndChild( comment );
                    }
                    comment->SetText( ( QString( "请核对翻译!中文：%1\n翻译：%2" ).arg( result.match_str.first ).arg( result.match_str.second ) ).toStdString().c_str() );
                }
            }
            else if ( unit.Content.value( source )->Attribute( "type" ) )
                unit.Content.value( source )->DeleteAttribute( "type" );
        }
        QVariant var;
        var.setValue( unit.Content.value( source ) );
        m_tblWidget_vec[ m_cur_lang ]->InsetRowItem( unit.UnitName, source, translation_list, Config::getInstance().getLang()[ unit.Language ], var ); // 插入行
        if ( result.ret != match_ret::Success && result.ret != match_ret::Ignore )
            m_tblWidget_vec[ m_cur_lang ]->SetBackGroundColor( -1, 2, "#ffc107" );
        qApp->processEvents();
    }
}

match_result TsFileEdit::TranslateMatch( QString langto, QString source, QStringList& translate_list, bool original )
{
    // 原文中不包含中文
    if ( noChineseReg.exactMatch( source ) )
        return match_result();
    if ( m_dic_vec->contains( source ) )
    {
        auto match_list = m_dic_vec->values( source );

        // 获取字典中的翻译
        for ( auto r : match_list )
        {
            // 匹配到的翻译插入列表中
            if ( translate_list.contains( r.at( m_cur_lang_index ) ) )
                continue;
            translate_list << r.at( m_cur_lang_index );
        }
        return match_result( qMakePair( source, match_list.first().at( m_cur_lang_index ) ), match_ret::Success );
    }
    else
    {
        qDebug() << "匹配翻译中 " + source;
        if ( source.startsWith( " " ) )
        {
            // 以空格开头，去除头部空格匹配，若成功，在翻译头部添加空格
            QString s = source;
            QStringList list;
            match_result result = TranslateMatch( langto, s.remove( 0, 1 ), list, false );
            if ( result.ret == match_ret::Success )
            {
                for ( QString& s : list )
                    s.insert( 0, " " );
            }
            translate_list.append( list );
            if ( result.ret != match_ret::Fail )
            {
                result.ret = match_ret::PartMatch;
                return result;
            }
        }
        if ( source.endsWith( " " ) )
        {
            // 以空格结尾，去除尾部空格匹配，若成功，在翻译尾部添加空格
            QString s = source;
            QStringList list;
            match_result result = TranslateMatch( langto, s.left( s.size() - 1 ), list, false );
            if ( result.ret == match_ret::Success )
                for ( QString& s : list )
                    s.append( " " );
            translate_list.append( list );
            if ( result.ret != match_ret::Fail )
            {
                result.ret = match_ret::PartMatch;
                return result;
            }
        }
        if ( source.contains( "  " ) )
        {
            // 字符中间有多个空格，格式化空格，去除头尾空格，并将多个空格格式化为1个
            QString s = source;
            qDebug() << "中间有多个空格 " + s.simplified();
            match_result result = TranslateMatch( langto, s.simplified(), translate_list, false );
            if ( result.ret != match_ret::Fail )
            {
                result.ret = match_ret::PartMatch;
                return result;
            }
        }
        if ( source.contains( "  " ) )
        {
            // 字符中间有空格，去除空格
            QString s = source;
            qDebug() << "中间有空格 " + s.replace( " ", "" );
            match_result result = TranslateMatch( langto, s.replace( " ", "" ), translate_list, false );
            if ( result.ret != match_ret::Fail )
            {
                result.ret = match_ret::PartMatch;
                return result;
            }
        }
        if ( original )
        {
            // 原文去除尾部最后一个字符匹配翻译（通常最后一个为标点符号）
            QString s = source;
            match_result result = TranslateMatch( langto, s.left( s.size() - 1 ), translate_list, false );
            if ( result.ret != match_ret::Fail )
            {
                result.ret = match_ret::PartMatch;
                return result;
            }
        }
    }

    // 若翻译为空，且选中了自动翻译，则调用百度翻译进行翻译
    if ( translate_list.size() == 0 && ui->checkBox_auto_translate->isChecked() )
    {
        qDebug() << "字典中未找到翻译，调用百度翻译：" << source;
        ui->label_tip->setText( QString( "字典中未找到翻译，调用百度翻译:%1" ).arg( source ) );
        translate_list << BaiduTranslateAPI::getInstance().BaiduTranslate( source, Config::getInstance().getLang()[ langto ] );
    }

    qDebug() << "无法匹配翻译:" << source;
    ui->label_tip->setText( QString( "无法匹配翻译:%1" ).arg( source ) );
    return match_result( match_ret::Fail );
}

void TsFileEdit::LoadTs2Table()
{
    if ( m_dic_vec == nullptr )
    {
        ui->label_tip->setText( "请先加载字典文件！" );
        return;
    }
    SetWidgetState( true );
    m_xmlDoc_map.clear();
    for ( auto lang_btn : m_chkBtn_vec )
    {
        if ( !lang_btn->isChecked() )
            continue;
        QString file = lang_btn->getData().toString().replace( " ", "\ " );
        m_cur_lang = lang_btn->getID();
        m_tblWidget_vec[ m_cur_lang ]->clearContents();
        m_tblWidget_vec[ m_cur_lang ]->setRowCount( 0 );
        m_cur_lang_index = 0;
        tinyxml2::XMLDocument* doc = new tinyxml2::XMLDocument();
        m_xmlDoc_map.insert( m_cur_lang, doc );
        if ( !loadTS( doc, file ) )
        {
            ui->label_tip->setText( QString( "%1文件打开失败！" ).arg( file.split( "/" ).back() ) );
            continue;
        }
        for ( auto key : m_header_index.keys() )
        {
            if ( m_header_index.value( key ).first == Config::getInstance().getLang_zh()[ GetLanguage( doc ) ] )
                m_cur_lang_index = key;
        }
        if ( m_cur_lang_index == 0 )
        {
            qDebug() << "语言匹配失败" << Config::getInstance().getLang_zh()[ GetLanguage( doc ) ];
            ui->label_tip->setText( QString( "语言匹配失败！%1" ).arg( Config::getInstance().getLang_zh()[ GetLanguage( doc ) ] ) );
            SetWidgetState( false );
            return;
        }
        ui->label_tip->setText( QString( "正在读取%1文件！" ).arg( file.split( "/" ).back() ) );
        qRegisterMetaType< std::function< void( TLUnit ) > >( "std::function< void( TLUnit ) >" );
        RorWXML( doc, std::bind( &TsFileEdit::ReadUnit, this, std::placeholders::_1 ) ); // 读取ts文件
        ui->label_tip->setText( QString( "正在保存%1文件！" ).arg( file.split( "/" ).back() ) );
        if ( tinyxml2::XMLError::XML_SUCCESS == doc->SaveFile( file.toLocal8Bit().toStdString().c_str() ) )
            ui->label_tip->setText( QString( "保存%1文件成功！" ).arg( file.split( "/" ).back() ) );
        else
            ui->label_tip->setText( QString( "保存%1文件失败！" ).arg( file.split( "/" ).back() ) );
    }
    SetWidgetState( false );
}

void TsFileEdit::SelDicdir()
{
    QString file_path = QFileDialog::getOpenFileName( this, "选择字典", qApp->applicationDirPath(), "Excel(*.xlsx)" );
    if ( file_path.isEmpty() )
        return;
    ui->lineEdit_dir_dic->setText( file_path );
    LoadDic(); // 加载字典
}

void TsFileEdit::SelTsdir()
{
    QString dir_path = QApplication::applicationDirPath();
    if ( !ui->lineEdit_dir_ts->text().isEmpty() )
        dir_path = ui->lineEdit_dir_ts->text();
    dir_path = QFileDialog::getExistingDirectory( this, tr( "选择ts文件目录" ),
                                                  dir_path,
                                                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
    if ( dir_path.isEmpty() )
        return;
    ui->lineEdit_dir_ts->setText( dir_path );
    ParseTsDir();
}
