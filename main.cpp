#include "toolboxui.h"
#include <QApplication>
#include <QResource>
#include <QDebug>
#include <QFile>

int main( int argc, char* argv[] )
{
    QApplication a( argc, argv );
//#ifdef QT_NO_DEBUG
    qDebug() << QResource::registerResource( a.applicationDirPath() + "/resources.rcc" );
//#else
    QFile qss( ":/file/stylesheet.qss" );
    qss.open( QFile::ReadOnly );
    QByteArray qssBuf = qss.readAll();
    a.setStyleSheet( qssBuf );
//#endif
    ToolBoxUI w;
    w.show();

    return a.exec();
}
