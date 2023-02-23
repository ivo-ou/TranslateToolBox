#include "tsoutput.h"
#include "ui_tsoutput.h"

#include "xlsxdocument.h"
#include "baidutranslateapi.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

#define log ui->textEdit_log

TSOUTPUT::TSOUTPUT( WorkMode workMode, QWidget* parent )
    : QWidget( parent )
    , ui( new Ui::TSOUTPUT )
    , m_workMode( workMode )
{
    ui->setupUi( this );
    initUI();
    connect( ui->lineEdit_dir, &QLineEdit::editingFinished, this, &TSOUTPUT::ParseDir );
    connect( ui->pushButton_dir_sel, &QPushButton::clicked, this, &TSOUTPUT::Seldir );
}

TSOUTPUT::~TSOUTPUT()
{
    delete ui;
}

void TSOUTPUT::initUI()
{
    if ( m_workMode == qm2ts )
    {
        setWindowTitle( "QM转TS" );
        ui->widget_func->hide();
    }
    else
        setWindowTitle( "QM/TS导出" );
    ui->textEdit_log->clear();
    ui->lineEdit_dir->clear();
    cleanWidgetLang();
    if ( Config::getInstance().getBaidu_id().isEmpty() || Config::getInstance().getBaidu_key().isEmpty() )
    {
        ui->checkBox_translate->setEnabled( false );
        ui->checkBox_translate->setChecked( false );
        ui->checkBox_translate->setToolTip( "百度翻译API的ID或KEY为空，无法使用" );
    }
    else
    {
        ui->checkBox_translate->setEnabled( true );
        ui->checkBox_translate->setToolTip( "" );
    }
}

// 清空左侧文件栏
void TSOUTPUT::cleanWidgetLang()
{
    while ( ui->widget_lang->layout()->count() )
    {
        auto w = ui->widget_lang->layout()->itemAt( 0 )->widget();
        w->setParent( nullptr );
        ui->widget_lang->layout()->removeWidget( w );
    }
}

// 选择ts文件目录
void TSOUTPUT::Seldir()
{
    ui->textEdit_log->clear();
    QString dir_path = QApplication::applicationDirPath();
    if ( !ui->lineEdit_dir->text().isEmpty() )
        dir_path = ui->lineEdit_dir->text();
    dir_path = QFileDialog::getExistingDirectory( this, tr( "选择ts文件目录" ),
                                                  dir_path,
                                                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
    if ( dir_path.isEmpty() )
        return;
    ui->lineEdit_dir->setText( dir_path );
    ParseDir();
}

// 解析目录中的ts文件名
void TSOUTPUT::ParseDir()
{
    ui->lineEdit_dir->blockSignals( true );
    ui->lineEdit_dir->clearFocus();
    ui->lineEdit_dir->blockSignals( false );
    cleanWidgetLang();
    QString dir_path = ui->lineEdit_dir->text();
    if ( !dir_path.isEmpty() )
    {
        ui->lineEdit_dir->setText( dir_path );
        QDir dir( dir_path );
        QStringList nameFilters, files;
        nameFilters << "*.qm";
        if ( m_workMode == WorkMode::t2xlsx )
            nameFilters << "*.ts";
        files = dir.entryList( nameFilters, QDir::Files | QDir::Readable, QDir::Name );
        if ( files.size() == 0 )
        {
            setLog( log, "未识别到文件，请检查目录！", LogLevel::error );
        }
        else
        {
            QSet< QString > name_set;
            // 生成左侧按钮
            for ( auto name : files )
            {
                QString btn_name = name.left( name.size() - 3 );
                if ( m_workMode == WorkMode::t2xlsx )
                {
                    if ( name_set.find( btn_name ) != name_set.end() )
                    {
                        // 有重复名字的翻译文件，优先选择ts文件排除qm文件
                        if ( name.endsWith( "ts" ) )
                        {
                            QCheckBox* chb = ui->widget_lang->findChild< QCheckBox* >( btn_name );
                            if ( !chb )
                                continue;
                            setLog( log, QString( "识别到重复文件%1和%2，优先使用%2" ).arg( chb->property( "fileName" ).toString() ).arg( name ), LogLevel::info );
                            chb->setProperty( "fileName", name );
                        }
                        else
                        {
                            QCheckBox* chb = ui->widget_lang->findChild< QCheckBox* >( btn_name );
                            if ( !chb )
                                continue;
                            setLog( log, QString( "识别到重复文件%1和%2，优先使用%2" ).arg( name ).arg( chb->property( "fileName" ).toString() ), LogLevel::info );
                        }
                        continue;
                    }
                }
                QCheckBox* chb = new QCheckBox( btn_name, ui->widget_lang );
                chb->setProperty( "fileName", name );
                chb->setObjectName( btn_name );
                chb->setToolTip( btn_name );
                ui->widget_lang->layout()->addWidget( chb );
                name_set.insert( btn_name );
            }
            setLog( log, QString( "识别到%1个文件" ).arg( name_set.size() ), LogLevel::success );
        }
    }
}

// 转换qm格式为ts格式
QStringList TSOUTPUT::Transform()
{
    QStringList fileName_list;
    QList< QCheckBox* > chb_list = ui->widget_lang->findChildren< QCheckBox* >();
    QList< QCheckBox* >::iterator iter = chb_list.begin();
    while ( iter < chb_list.end() )
    {
        if ( !( *iter )->isChecked() )
        {
            iter++;
            continue;
        }
        QString fileName = ( *iter )->property( "fileName" ).toString();
        qDebug() << fileName;
        if ( fileName.endsWith( ".qm" ) )
        {
            setLog( log, QString( "正在转换%1为TS格式" ).arg( fileName ), LogLevel::process );
            QString erro;
            threadExec( [&]() {
                QProcess p;
                p.setWorkingDirectory( QApplication::applicationDirPath() );
                p.setProgram( "lconvert.exe" ); // 调用qt的lconvert.exe进行转换
                QStringList args;
                args << ui->lineEdit_dir->text() + "/" + fileName
                     << "-o"
                     << ( *iter )->text() + ".ts";
                p.setArguments( args );
                p.start();
                p.waitForFinished();
                erro = p.readAllStandardError();
            } );
            if ( !erro.isEmpty() )
                setLog( log, ( *iter )->text() + "语言转换为ts文件失败：", LogLevel::error );
            else
            {
                setLog( log, ( *iter )->text() + "语言转换为ts文件成功", LogLevel::success );
                fileName = fileName.replace( "qm", "ts" ); // 转换成功后将checkbobox按钮中保存的文件后缀名改为ts
                ( *iter )->setProperty( "fileName", fileName );
                // 导出功能，选中qm文件，先转成ts文件，再进行导出操作，导出完成后需删除转换的ts文件
                if ( m_workMode == WorkMode::t2xlsx )
                {
                    // 标记qm转ts文件，导出成功后删除转换的ts文件
                    ( *iter )->setProperty( "mark", "del" );
                }
            }
        }
        else if ( fileName.endsWith( ".ts" ) )
        {
            setLog( log, QString( "%1已为TS格式，无需转换" ).arg( fileName ), LogLevel::normal );
        }
        fileName_list.append( fileName );
        iter++;
    }
    return fileName_list;
}

void TSOUTPUT::Output( QStringList fileList )
{
    m_file_lang.clear();
    m_dic_list.clear();
    m_doc_list.clear();
    QString base_path = ui->lineEdit_dir->text() + "/";
    for ( QString fileName : fileList )
    {
        setLog( log, QString( "正在读取 %1 文件" ).arg( fileName ), LogLevel::process );
        tinyxml2::XMLDocument* doc = new tinyxml2::XMLDocument();
        if ( !loadTS( doc, base_path + fileName ) )
        {
            setLog( log, QString( "%1文件打开失败！" ).arg( fileName ), LogLevel::error );
            continue;
        }
        m_doc_list.append( doc );
        m_dic_list.push_front( QHash< int, TLUnit >() );
        qRegisterMetaType< std::function< void( TLUnit ) > >( "std::function< void( TLUnit ) >" );
        QString language = RorWXML( doc, std::bind( &TSOUTPUT::ReadTranslate, this, std::placeholders::_1 ) ); // 读取xml文件
        m_file_lang.push_front( qMakePair( fileName, language ) );                                             // m_file_lang存放文件名与语言
        setLog( log, "\n", LogLevel::normal );
    }

    // 合并导出
    // 判断多语言文件翻译数量与随机抽取原文是否一致，若一致则合并导出，不一致则分开导出
    if ( !ui->checkBox_indiv->isChecked() )
    {
        if ( m_dic_list.size() == 0 )
        {
            setLog( log, "没有需要导出的文件", LogLevel::error );
            return;
        }
        // 判断相邻文件翻译数量是否一致，随机抽取3个位置文件名比对是否一致
        auto isSame = [&]( QHash< int, TLUnit > map, QHash< int, TLUnit > map_next ) -> bool {
            if ( map.size() == 0 || map_next.size() == 0 )
                return false;
            srand( uint( QTime::currentTime().msec() ) );
            int rand_num = rand() % map.size();     // 生成随机数
            bool b = map.size() == map_next.size(); // 比较两个map尺寸是否一致
            qDebug() << QString( "单元数量 f:%1 - n:%2" ).arg( map.size() ).arg( map_next.size() );
            if ( !b )
            {
                for ( auto i = 0; i < qMax( map.size(), map_next.size() ); i++ )
                {
                    qDebug() << QString( "单元：%1 %2  -   %3" ).arg( i + 1 ).arg( map.size() < i ? "" : map[ i ].UnitName ).arg( map_next.size() < i ? "" : map_next[ i ].UnitName );
                }
                return b;
            }
            b = b && map[ rand_num ].UnitName == map_next[ rand_num ].UnitName;
            b = b && map[ rand_num / 2 ].UnitName == map_next[ rand_num / 2 ].UnitName;
            b = b && map[ rand_num / 3 ].UnitName == map_next[ rand_num / 3 ].UnitName;
            qDebug() << QString( "f_s:%1 - n_s:%2" ).arg( map[ rand_num ].UnitName ).arg( map_next[ rand_num ].UnitName );
            qDebug() << QString( "f_s:%1 - n_s:%2" ).arg( map[ rand_num / 2 ].UnitName ).arg( map_next[ rand_num / 2 ].UnitName );
            qDebug() << QString( "f_s:%1 - n_s:%2" ).arg( map[ rand_num / 3 ].UnitName ).arg( map_next[ rand_num / 3 ].UnitName );
            return b;
        };
        setLog( log, "正在进行合并导出...", LogLevel::process );
        for ( int i = 0; i < m_dic_list.size() - 1; i++ )
        {

            if ( !isSame( m_dic_list[ i ], m_dic_list[ i + 1 ] ) )
            {
                if ( QMessageBox::question( this, "错误", QString( "%1文件翻译数量与%2文件不一致，是否全部单独导出？" ).arg( m_file_lang[ i ].first ).arg( m_file_lang[ i + 1 ].first ) ) == QMessageBox::No )
                {
                    setLog( log, "合并导出失败!", LogLevel::info );
                    return;
                }
                setLog( log, "合并导出失败，正在单独导出文件!!!", LogLevel::error );
                IndivOutput();
                return;
            }
        }

        MergeOutput(); // 合并导出
    }
    else
    {
        IndivOutput();
    }
}

// 读取xml文件回调函数
void TSOUTPUT::ReadTranslate( TLUnit unit )
{
    // 处理已删除的节点 <translation type="vanished">
    // 处理不知道啥的节点 <translation type="obsolete">
    // vanished为true则代码单元内没有有效的翻译，均是已删除的翻译，忽略该单元
    if ( unit.Content.size() == 0 )
        return;
    bool vanished = true;
    QHash< QString, tinyxml2::XMLElement* >::iterator iter = unit.Content.begin();
    while ( iter != unit.Content.end() )
    {
        vanished = QString::fromUtf8( ( *iter.value() ).Attribute( "type" ) ) == "vanished" || QString::fromUtf8( ( *iter.value() ).Attribute( "type" ) ) == "obsolete";
        if ( !vanished )
            break;
        iter++;
    }
    if ( vanished )
    {
        qDebug() << "丢弃" << unit.UnitName << "无效单元";
        setLog( log, QString( "丢弃%1无效单元" ).arg( unit.UnitName ), LogLevel::info );
        qApp->processEvents( QEventLoop::ExcludeUserInputEvents );
        return;
    }

    QHash< int, TLUnit >& dic_map = m_dic_list.first();
    dic_map[ dic_map.size() ] = unit;
}

void TSOUTPUT::on_pushButton_start_clicked()
{
    setButtonStatus( false );
    QStringList file_list = Transform();  // 转换文件
    if ( m_workMode == WorkMode::t2xlsx ) // 模式为导出时，进行导出
        // 导出为xlsx
        Output( file_list );
    setButtonStatus( true );
}

// 合并导出
void TSOUTPUT::MergeOutput()
{
    QXlsx::Document xlsx;
    QXlsx::Format format;
    format.setHorizontalAlignment( QXlsx::Format::HorizontalAlignment::AlignHCenter );
    format.setVerticalAlignment( QXlsx::Format::VerticalAlignment::AlignVCenter );
    format.setBorderColor( QColor( Qt::black ) );
    format.setBorderStyle( QXlsx::Format::BorderThin );
    format.setPatternBackgroundColor( QColor( Qt::lightGray ) );

    QMap< QString, QString > lang = Config::getInstance().getLang();

    /* 格式
     * 序号 | 来源 | 原文 | A语言 | B语言 | C语言
     * 序号 | 来源 | 原文 | A语言 | 百度翻译 | B语言 | 百度翻译 | C语言 | 百度翻译
     */
    // 表头
    int col = 1;
    xlsx.write( 1, col, "序号", format );
    xlsx.write( 1, ++col, "来源", format );
    xlsx.write( 1, ++col, "原文", format );
    for ( auto fileLang : m_file_lang )
    {
        xlsx.write( 1, ++col, fileLang.second, format ); // 语言
        if ( ui->checkBox_translate->isChecked() )
            xlsx.write( 1, ++col, "百度翻译", format ); // 百度翻译
    }
    int row = 2;
    format.setPatternBackgroundColor( QColor() );
    // 遍历写入每行
    // 选取m_dic_list[ 0 ].size()是因为在此之前做过判断，所有语言的size都是一致的，故选取第一个size即可
    for ( int index = 0; index < m_dic_list[ 0 ].size(); index++ )
    {
        qApp->processEvents( QEventLoop::ExcludeUserInputEvents );
        int col = 1;
        xlsx.write( row, col, index, format );                               // 序号
        xlsx.write( row, ++col, m_dic_list[ 0 ][ index ].UnitName, format ); // 来源
        QSet< QString > all_source;                                          // 所有翻译文件中相同单元内的全部翻译汇总
        for ( auto dic : m_dic_list )
        {
            QHash< QString, tinyxml2::XMLElement* >::iterator iter = dic[ index ].Content.begin();
            while ( iter != dic[ index ].Content.end() )
            {
                all_source.insert( iter.key() ); // 单元内所有的翻译原文汇总
                iter++;
            }
        }

        xlsx.mergeCells( QXlsx::CellRange( row, 1, row + all_source.size() - 1, 1 ), format ); // 合并 “序号” 列单元格
        xlsx.mergeCells( QXlsx::CellRange( row, 2, row + all_source.size() - 1, 2 ), format ); // 合并 “来源” 列单元格
        for ( auto source : all_source )
        {
            // 写入单元内所有翻译内容列
            col = 3;
            xlsx.write( row, col, source, format ); // 原文

            for ( auto dic : m_dic_list )
            {
                // 写入每行翻译内容
                const QHash< QString, tinyxml2::XMLElement* > Content = dic[ index ].Content; // 单元内翻译内容
                QXlsx::Format format1( format );
                QString translate = "";
                qDebug() << index << dic[ index ].Language << dic[ index ].UnitName << source;
                if ( Content.contains( source ) && Content.value( source ) )
                    translate = Content.value( source )->GetText(); // 获取翻译
                if ( ui->checkBox_mark_undone->isChecked() && translate.isEmpty() )
                    format1.setPatternBackgroundColor( QColor( 255, 255, 0 ) ); // 标记未翻译
                if ( ui->checkBox_mark_dot->isChecked() && ChineseReg.exactMatch( translate ) )
                    format1.setPatternBackgroundColor( QColor( 255, 170, 0 ) ); // 标记中文字符
                xlsx.write( row, ++col, translate, format1 );                   // 翻译文本

                // 百度翻译
                if ( ui->checkBox_translate->isChecked() )
                {
                    // 获取的翻译为空，且原文中含中文（非全英文或数字组合），则翻译
                    if ( translate.isEmpty() && !noChineseReg.exactMatch( source ) )
                    {
                        setLog( log, QString( "正在通过API获取%1的翻译" ).arg( source ), LogLevel::process );
                        xlsx.write( row, ++col, BaiduTranslateAPI::getInstance().BaiduTranslate( source, lang[ dic[ index ].Language ] ), format ); // 获取百度翻译
                    }
                }
            }
            row++;
        }
    }
    // xlsx文件属性
    xlsx.setDocumentProperty( "description", "Created with TranslateToolBox Design by RD1537" );
    xlsx.setDocumentProperty( "creator", "TranslateToolBox" );
    setLog( log, "合并文件成功，正在保存文件...", LogLevel::process );
    bool save = false;
    QString file_path = ui->lineEdit_dir->text();
    QString file_name = m_file_lang[ 0 ].first.split( "_" )[ 0 ] + "merge_output.xlsx";
    threadExec( [&] { save = xlsx.saveAs( file_path + "/" + file_name ); } );
    if ( save )
        setLog( log, QString( "文件合并导出保存成功！%1\n\n\n" ).arg( file_name ), LogLevel::success );
    else
        setLog( log, "文件合并导出保存失败！\n\n\n", LogLevel::error );
}

// 单独导出
void TSOUTPUT::IndivOutput()
{
    QMap< QString, QString > lang = Config::getInstance().getLang();
    for ( int i = 0; i < m_dic_list.size(); i++ )
    {
        QXlsx::Document xlsx;
        QXlsx::Format format;
        format.setHorizontalAlignment( QXlsx::Format::HorizontalAlignment::AlignHCenter );
        format.setVerticalAlignment( QXlsx::Format::VerticalAlignment::AlignVCenter );
        format.setBorderColor( QColor( Qt::black ) );
        format.setBorderStyle( QXlsx::Format::BorderThin );
        format.setPatternBackgroundColor( QColor( Qt::lightGray ) );

        /* 格式
         * 序号 | 来源 | 原文 | A语言
         * 序号 | 来源 | 原文 | A语言 | 百度翻译
         */
        // 表头
        int col = 1;
        xlsx.write( 1, col, "序号", format );
        xlsx.write( 1, ++col, "来源", format );
        xlsx.write( 1, ++col, "原文", format );
        xlsx.write( 1, ++col, m_file_lang[ i ].second, format ); // 语言
        if ( ui->checkBox_translate->isChecked() )
            xlsx.write( 1, ++col, "百度翻译", format ); // 百度翻译
        format.setPatternBackgroundColor( QColor() );
        // 从xlsx第二行开始写入
        int row = 2;
        bool undone = ui->checkBox_only_undone->isChecked(); // 只导出未完成
        // 遍历写入每行
        for ( int index = 0; index < m_dic_list[ i ].size(); index++ )
        {
            qApp->processEvents( QEventLoop::ExcludeUserInputEvents );
            // 判断单元内是否存在未完成的翻译
            if ( undone )
            {
                bool exist_undone = false;
                for ( auto t : m_dic_list[ i ][ index ].Content )
                {
                    exist_undone = QString::fromUtf8( t->Attribute( "type" ) ) == "unfinished" || QString( t->GetText() ).isEmpty();
                    if ( exist_undone )
                        break;
                }
                if ( !exist_undone )
                {
                    continue;
                }
            }
            int col = 1;
            TLUnit unit = m_dic_list[ i ][ index ];
            int first_row = row;
            xlsx.write( row, col, index, format );           // 序号
            xlsx.write( row, ++col, unit.UnitName, format ); // 来源
            for ( auto source : unit.Content.keys() )
            {
                // 写入每行翻译内容
                const QHash< QString, tinyxml2::XMLElement* > Content = unit.Content; // 单元内翻译内容
                QXlsx::Format format1( format );
                QString translate = "";
                qDebug() << index << unit.Language << unit.UnitName << source;
                if ( Content.contains( source ) && Content.value( source ) )
                    translate = Content.value( source )->GetText();
                QString type = QString::fromUtf8( Content.value( source )->Attribute( "type" ) );
                // 只导出未完成，且获取的翻译不为空，则continue
                if ( undone && !translate.isEmpty() && QString::fromUtf8( Content.value( source )->Attribute( "type" ) ) != "unfinished" )
                {
                    continue;
                }
                if ( ui->checkBox_mark_undone->isChecked() && translate.isEmpty() )
                    format1.setPatternBackgroundColor( QColor( 255, 255, 0 ) ); // 标记未翻译
                if ( ui->checkBox_mark_dot->isChecked() && ChineseReg.exactMatch( translate ) )
                    format1.setPatternBackgroundColor( QColor( 255, 170, 0 ) ); // 标记中文字符
                // 写入单元内所有翻译内容列
                col = 3;
                xlsx.write( row, col, source, format );       // 原文
                xlsx.write( row, ++col, translate, format1 ); // 翻译文本

                if ( ui->checkBox_translate->isChecked() )
                {
                    if ( translate.isEmpty() && !noChineseReg.exactMatch( source ) )
                    {
                        setLog( log, QString( "正在通过API获取%1的翻译" ).arg( source ), LogLevel::process );
                        xlsx.write( row, ++col, BaiduTranslateAPI::getInstance().BaiduTranslate( source, lang[ unit.Language ] ), format ); // 获取百度翻译
                    }
                }
                row++;
            }
            xlsx.mergeCells( QXlsx::CellRange( first_row, 1, row - 1, 1 ), format ); // 合并 “序号” 列单元格
            xlsx.mergeCells( QXlsx::CellRange( first_row, 2, row - 1, 2 ), format ); // 合并 “来源” 列单元格
        }
        // xlsx属性
        xlsx.setDocumentProperty( "description", "Created with TranslateToolBox Design by RD1537" );
        xlsx.setDocumentProperty( "creator", "TranslateToolBox" );
        bool save = false;
        QString file_path = ui->lineEdit_dir->text();
        QString fileName = m_file_lang[ i ].first.replace( ".ts", undone ? "_undone_output.xlsx" : "_output.xlsx" );
        setLog( log, QString( "生成%1文件成功，正在保存文件..." ).arg( fileName ), LogLevel::process );
        threadExec( [&] { save = xlsx.saveAs( file_path + "/" + fileName ); } );
        if ( save )
            setLog( log, QString( "%1文件导出保存成功！\n\n" ).arg( fileName ), LogLevel::success );
        else
            setLog( log, QString( "%1文件导出保存失败！\n\n" ).arg( fileName ), LogLevel::error );
    }
    setLog( log, "\n", LogLevel::info );
}

void TSOUTPUT::setButtonStatus( bool enabled )
{
    ui->widget_lang->setEnabled( enabled );
    ui->widget_func->setEnabled( enabled );
    ui->widget->setEnabled( enabled );
    ui->pushButton_start->setEnabled( enabled );
}

void TSOUTPUT::on_checkBox_indiv_toggled( bool checked )
{
    ui->checkBox_only_undone->setEnabled( checked );
    if ( checked )
        ui->checkBox_only_undone->setChecked( false );

    static bool isFirst = true;
    if ( isFirst )
    {
        isFirst = false;
        setLog( log, "\t\t合并导入注意事项：\n1. 合并导入会排除文件中失效的翻译（标记为vanished的）\n2. 要求勾选文件翻译单元数量一致，且对应位置的单元名称一致，否则无法合并\n3. 建议合并前先更新一次翻译文件再进行导出\n", LogLevel::info );
    }
}
