#ifndef BAIDUTRANSLATEAPI_H
#define BAIDUTRANSLATEAPI_H

#include <QObject>

#include "sharefunction.h"

class QNetworkAccessManager;
class QNetworkReply;

class BaiduTranslateAPI : public QObject
{
    Q_OBJECT
public:
    static BaiduTranslateAPI& getInstance()
    {
        static BaiduTranslateAPI instance( Config::getInstance().getBaidu_id(), Config::getInstance().getBaidu_key() );
        return instance;
    }
    BaiduTranslateAPI( QString id, QString key, QObject* parent = nullptr );
    ~BaiduTranslateAPI();
    void SetLangTo( QString langto );
    QString BaiduTranslate( QString source, QString lan_to );
private slots:
    int replyFinished( QNetworkReply* reply );

private:
    QNetworkAccessManager* m_networkAccessManager;
    QString m_APP_ID;
    QString m_APP_KEY;
    QString m_lang_to;
    QString cResult;
};

#endif // BAIDUTRANSLATEAPI_H
;
