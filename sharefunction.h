#ifndef SHAREFUNCTION_H
#define SHAREFUNCTION_H

#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QThread>
#include <mutex>
#include <QProcess>
#include <QApplication>
#include <QTextEdit>
#include <QSettings>
#include <tinyxml2.h>

#define noChineseReg QRegExp( "^[A-Za-z0-9^%&*（）()+-.',;=?/-—~μ$x22℃↓↑!]+$" )
#define ChineseReg QRegExp( "[\u4e00-\u9fa5]+" )

#define minBtn QSize( 46, 28 )
#define normalBtn QSize( 64, 32 )
#define fitBtn QSize( 80, 42 )
#define bigBtn QSize( 96, 42 )

Q_DECLARE_METATYPE( tinyxml2::XMLElement* );
struct TLUnit
{
    QString UnitName;                      // 翻译单元名称
    QString Language;                      // 语言
    QHash< QString, tinyxml2::XMLElement* > Content; // < 原文， 翻译节点>

    TLUnit()
    {
        UnitName.clear();
        Content.clear();
    }

    TLUnit( const TLUnit& unit )
    {
        UnitName = unit.UnitName;
        Language = unit.Language;
        Content = unit.Content;
    }
    bool empty()
    {
        return UnitName.isEmpty() && Content.size() == 0;
    }
};

enum LogLevel
{
    normal = 0,
    info,
    process,
    error,
    success
};

class Config
{
public:
    static Config& getInstance()
    {
        static Config instance;
        return instance;
    }
    explicit Config()
    {
        ReadConfig();
    }
    void ReadConfig()
    {
        //配置config.ini
        const QString iniFilePath = QCoreApplication::applicationDirPath() + "/config.ini";

        if ( !QFileInfo( iniFilePath ).exists() )
        {
            qDebug() << "config.ini文件不存在，自动新建";
            // 配置文件不存在则自动新建
            QSettings setting( "config.ini", QSettings::IniFormat );
            setting.setIniCodec( "utf-8" );
            setting.setValue( "BaiduFanyi/baidu_translate_id", "" );
            setting.setValue( "BaiduFanyi/baidu_translate_key", "" );
            setting.setValue( "Cache/FilterWords", "" );

            setting.setValue( "Language/zh_CN", "zh|中文" );
            setting.setValue( "Language/en_US", "en|英语" );
            setting.setValue( "Language/es_ES", "spa|西语" );
            setting.setValue( "Language/fr_FR", "fra|法语" );
            setting.setValue( "Language/pt_BR", "pt|葡语" );
            setting.setValue( "Language/ru_RU", "ru|俄语" );
        }
        QSettings setting( "config.ini", QSettings::IniFormat );
        setting.setIniCodec( "utf-8" );
        m_baidu_translate_id = setting.value( "BaiduFanyi/baidu_translate_id" ).toString();
        m_baidu_translate_key = setting.value( "BaiduFanyi/baidu_translate_key" ).toString();
        m_FilterWords = setting.value( "Cache/FilterWords" ).toString();
        // 生成语言map
        setting.beginGroup( "Language" );
        //遍历该组的键
        foreach ( QString key, setting.childKeys() )
        {
            QStringList lan_list = setting.value( key ).toString().split( "|" );
            lang[ key ] = lan_list[ 0 ];
            if ( lan_list.size() > 1 )
                lang_zh[ key ] = lan_list[ 1 ];
        }
        setting.endGroup();
    }
    QString getBaidu_id() { return m_baidu_translate_id; }
    QString getBaidu_key() { return m_baidu_translate_key; }
    QString getFilterWords() { return m_FilterWords; }
    void setFilterWords( QString filter )
    {
        QSettings setting( "config.ini", QSettings::IniFormat );
        setting.setIniCodec( "utf-8" );
        setting.setValue( "Cache/FilterWords", filter );
    }
    QMap< QString, QString > getLang() { return lang; }
    QMap< QString, QString > getLang_zh() { return lang_zh; }

private:
    QString m_baidu_translate_id;
    QString m_baidu_translate_key;
    QString m_FilterWords;
    QMap< QString, QString > lang;    /* = {
        { "en_US", "en" },
        { "es_ES", "spa" },
        { "fr_FR", "fra" },
        { "pt_BR", "pt" },
        { "ru_RU", "ru" },
    };*/
    QMap< QString, QString > lang_zh; /* = {
        { "en_US", "英语" },
        { "es_ES", "西语" },
        { "fr_FR", "法语" },
        { "pt_BR", "葡语" },
        { "ru_RU", "俄语" },
    };*/
};

inline void setLog( QTextEdit* w, QString log, LogLevel level )
{
    switch ( level )
    {
    case normal:
        w->setTextColor( QColor( Qt::black ) );
        break;
    case info:
        w->setTextColor( QColor( Qt::darkGray ) );
        break;
    case process:
        w->setTextColor( QColor( Qt::darkMagenta ) );
        break;
    case error:
        w->setTextColor( QColor( Qt::red ) );
        break;
    case success:
        w->setTextColor( QColor( Qt::darkGreen ) );
        break;
    }
    w->append( log );
}

template < typename L >
inline void threadExec( L lambdaExecFun )
{
    bool isFinish = false;
    std::mutex mtx_subjects;

    std::thread thr( [&]() {
        lambdaExecFun();

        std::lock_guard< std::mutex > lck( mtx_subjects );
        isFinish = true;
    } );

    while ( true )
    {
        {
            std::lock_guard< std::mutex > lck( mtx_subjects );
            if ( isFinish )
                break;
        }
        qApp->processEvents();
    }

    if ( thr.joinable() )
        thr.join();

    return;
}

inline bool loadTS( tinyxml2::XMLDocument* doc, QString path )
{
    auto ret = doc->LoadFile( path.toLocal8Bit() );
    if ( ret != tinyxml2::XMLError::XML_SUCCESS )
    {
        qDebug() << "文件打开失败：" << path.toLocal8Bit() << ret;
        return false;
    }
    return true;
}

inline QString GetLanguage( tinyxml2::XMLDocument* doc )
{
    if ( !doc )
        return "";
    tinyxml2::XMLElement* root = doc->RootElement(); //返回根节点
    QString language = root->Attribute( "language" );
    return language;
}

inline QString RorWXML( tinyxml2::XMLDocument* doc, std::function< void( TLUnit ) > func )
{
    //    XMLDocument *doc = new XMLDocument();
    //    if ( !loadTS( doc, path) )
    //        return false;
    /* 该函数不包含文件打开操作，请调用时先用上面注释代码打开文件*/
    if ( !doc )
        return "";
    tinyxml2::XMLElement* root = doc->RootElement(); //返回根节点
    QString language = root->Attribute( "language" );
    tinyxml2::XMLElement* context = root->FirstChildElement( "context" ); //返回 context 节点
    TLUnit unit;
    while ( context )
    {
        qApp->processEvents();
        if ( context )
        {

            QString curFileName;
            tinyxml2::XMLElement* node = context->FirstChildElement();
            while ( node )
            {
                if ( QString::fromUtf8( node->Name() ) == "message" )
                {
                    QString curSource;
                    if ( !node->FirstChildElement( "source" ) )
                        goto nextone;
                    curSource = node->FirstChildElement( "source" )->GetText();
                    if ( !node->FirstChildElement( "translation" ) )
                        goto nextone;
                    if ( !curSource.isEmpty() && curSource.contains( QRegExp(ChineseReg)))
                    {
                        // 不处理原文中不含中文的字符
                        unit.Content[ curSource ] = node->FirstChildElement( "translation" );
                        qDebug() << curFileName << curSource << node->FirstChildElement( "translation" )->GetText();
                    }
                    else
                        qDebug() << "no chinese " << curSource;
                }
                else if ( QString::fromUtf8( node->Name() ) == "name" )
                {
                    {
                        if ( !unit.empty() )
                            func( unit ); // 回调函数处理翻译节点
                        curFileName = node->GetText();
                        unit.UnitName = curFileName;
                        unit.Language = language;
                        unit.Content = QHash< QString, tinyxml2::XMLElement* >();
                    }
                }
            nextone:
                node = node->NextSiblingElement();
            }
        }
        context = context->NextSiblingElement();
    }
    if ( !unit.empty() )
        func( unit ); // 回调函数处理翻译节点
    return language;
}


#endif // SHAREFUNCTION_H
