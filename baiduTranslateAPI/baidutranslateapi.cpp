#include "baidutranslateapi.h"

#include <QtNetwork>
#include <QCryptographicHash>
#include <QTime>

BaiduTranslateAPI::BaiduTranslateAPI( QString id, QString key, QObject* parent )
    : QObject( parent )
    , m_APP_ID( id )
    , m_APP_KEY( key )
    , m_lang_to( "en" )
{
    m_networkAccessManager = new QNetworkAccessManager();
    connect( m_networkAccessManager, &QNetworkAccessManager::finished, this, &BaiduTranslateAPI::replyFinished );
}

BaiduTranslateAPI::~BaiduTranslateAPI()
{
    delete m_networkAccessManager;
}

QString BaiduTranslateAPI::BaiduTranslate(QString source, QString lan_to )
{
    QString baseUrl = QString( "http://api.fanyi.baidu.com/api/trans/vip/translate?" );

    int uuid = rand();
    QString timestamp = QString::number( QDateTime::currentDateTime().toSecsSinceEpoch() );
    QByteArray sign;
    QString lang_to = lan_to.isEmpty() ? m_lang_to:lan_to;
    QString str = m_APP_ID + source + QString::number( uuid ) + m_APP_KEY;

    sign = QCryptographicHash::hash( str.toUtf8(), QCryptographicHash::Md5 );
    QString MD5 = sign.toHex();

    QString sUrl = baseUrl + QString( "q=%1&from=%2&to=%3&appid=%4&salt=%5&sign=%6" )
                                 .arg( source )
                                 .arg( "zh" )
                                 .arg( lang_to )
                                 .arg( m_APP_ID )
                                 .arg( uuid )
                                 .arg( MD5 );
//    qDebug() << sUrl << "：  " << m_APP_ID << "  " << m_APP_KEY;
    cResult.clear();
    QUrl url( sUrl );
    QNetworkRequest request( url );         //create http request header
    m_networkAccessManager->get( request ); //send GET request to get result
    QTimer timer;
    timer.start( 5000 );
    while ( timer.isActive() && cResult.isEmpty() /*超时*/ )
        qApp->processEvents();
    return cResult;
}

int BaiduTranslateAPI::replyFinished( QNetworkReply* reply )
{
    QJsonParseError jsonError;
    QByteArray all = reply->readAll(); //获得api返回值
                                       /* 大概是这样的一个东西
       {"from":"en","to":"zh","trans_result":[{"src":"apple","dst":"\u82f9\u679c"}]}
       你需要解码 */
    QJsonDocument json = QJsonDocument::fromJson( all, &jsonError );
    QJsonObject object = json.object(); //json转码；
    if ( object.contains( "error_code" ) )
    {
        int nResult = object.value( "error_code" ).toInt();

        switch ( nResult )
        {
        case 52001:
            cResult = "/?.,??$52001 请求超时 重试";
            break;
        case 52002:
            cResult = "/?.,??$52002 系统错误 重试";
            break;
        case 54000:
            cResult = "/?.,??$54000 必填参数为空";
            break;
        case 54001:
            cResult = "/?.,??$54001 签名错误";
            break;
        case 54003:
            cResult = "/?.,??$54003 速度过快访问频率受限";
            break;
        case 54004:
            cResult = "/?.,??$54004 账户余额不足";
            break;
        case 54005:
            cResult = "/?.,??$54005 请求频繁";
            break;
        case 58002:
            cResult = "/?.,??$58002 服务关闭";
            break;
        default:
            cResult = "/?.,??$其他错误" + QString::number( nResult );
            break;
        }
    }
    else
    {
        QJsonArray value = object.value( "trans_result" ).toArray(); //一次解码
        /*
             {"from":"en","to":"zh",
               "trans_result":[{"src":"apple","dst":"\u82f9\u679c"}]}
               第一次解码
             */
        QJsonObject object1 = value.at( 0 ).toObject(); //二次解码开【】括号；
        /*
               {"src":"apple","dst":"\u82f9\u679c"}
               第二次解码
             */
        //from=object.value("from").toString();
        cResult = object1.value( "dst" ).toString(); //得到翻译结果
    }

    reply->deleteLater();
    return 1;
}
