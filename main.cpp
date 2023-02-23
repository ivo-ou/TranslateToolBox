#include "toolboxui.h"
#include <QApplication>
#include <QResource>
#include <QDebug>
#include <QFile>

int main( int argc, char* argv[] )
{
    QApplication a( argc, argv );
    qDebug() << QResource::registerResource( a.applicationDirPath() + "/resources.dll" );
    QFile qss( ":/file/stylesheet.qss" );
    qss.open( QFile::ReadOnly );
    QByteArray qssBuf = qss.readAll();
    a.setStyleSheet( qssBuf );
    ToolBoxUI w;
    w.show();

    return a.exec();
}
