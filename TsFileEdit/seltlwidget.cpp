#include "seltlwidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QButtonGroup>
#include "sharefunction.h"
#include "baidutranslateapi.h"


SelTLWidget::SelTLWidget( QString source, QString translate, QString langto, QStringList TL, QWidget* parent )
    : QDialog( parent )
    , m_source( source )
    , m_langto( langto )
{
    setMinimumSize( 300, 200 );
    QVBoxLayout* vlayout = new QVBoxLayout( this );
    vlayout->setContentsMargins( 16, 40, 16, 16 );
    vlayout->setSpacing( 20 );
    QLabel* label_source = new QLabel( source, this );
    label_source->setAlignment( Qt::AlignCenter );
    vlayout->addWidget( label_source );

    int index = 0;
    m_linE_map.clear();
    QGridLayout* glayout = new QGridLayout();
    m_btnGroup = new QButtonGroup( this );
    m_btnGroup->setExclusive( true );
    glayout->setHorizontalSpacing( 5 );
    vlayout->addLayout( glayout );
    while ( index < TL.size() )
    {
        QCheckBox* chk = new QCheckBox;
        chk->setFixedHeight( 32 );
        m_btnGroup->addButton( chk, index );
        QLineEdit* lineEdit = new QLineEdit( TL.at( index ) );
        lineEdit->setFixedHeight( 32 );
        m_linE_map.insert( index, lineEdit );
        glayout->addWidget( chk, index, 0 );
        glayout->addWidget( lineEdit, index, 1 );
        index++;
    }
    // 最后增加一行空白行，用作翻译或当选中翻译为空时
    QCheckBox* chk = new QCheckBox;
    m_btnGroup->addButton( chk, index );
    QLineEdit* lineEdit = new QLineEdit( "" );
    chk->setFixedHeight( 32 );
    lineEdit->setFixedHeight( 32 );
    m_linE_map.insert( index, lineEdit );
    glayout->addWidget( chk, index, 0 );
    glayout->addWidget( lineEdit, index, 1 );
    // 在末尾增加翻译按钮
    if ( !Config::getInstance().getBaidu_id().isEmpty() && !Config::getInstance().getBaidu_key().isEmpty() )
    {
        QPushButton* btn = new QPushButton( "翻译", this );
        btn->setProperty( "ID", index );
        btn->setProperty( "class", "mini" );
        btn->setFixedHeight( 32 );
        connect( btn, &QPushButton::clicked, this, &SelTLWidget::Translate );
        glayout->addWidget( btn, index, 2 );
    }
    if ( TL.contains( translate ) )
        m_btnGroup->button( TL.indexOf( translate ) )->setChecked( true );
    else
        m_btnGroup->button( index )->setChecked( true );
    vlayout->addSpacerItem( new QSpacerItem( 10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding ) );
    // 取消 确定 按钮
    QHBoxLayout* hlayout = new QHBoxLayout();
    QPushButton* btn_cancel = new QPushButton( "取消", this );
    QPushButton* btn_ok = new QPushButton( "确定", this );
    btn_cancel->setFixedSize( normalBtn );
    btn_ok->setFixedSize( normalBtn );
    hlayout->addSpacerItem( new QSpacerItem( 10, 10, QSizePolicy::Expanding ) );
    hlayout->addWidget( btn_cancel );
    hlayout->addWidget( btn_ok );
    vlayout->addLayout( hlayout );
    connect( btn_cancel, &QPushButton::clicked, this, &QDialog::reject );
    connect( btn_ok, &QPushButton::clicked, this, &QDialog::accept );
}

void SelTLWidget::Translate()
{
    QPushButton* btn = static_cast< QPushButton* >( sender() );
    if ( !btn )
        return;
    QLineEdit* linE = m_linE_map[ btn->property( "ID" ).toInt() ];
    if ( !linE )
        return;
    linE->setText( BaiduTranslateAPI::getInstance().BaiduTranslate( m_source, m_langto ) );
}

QPair< QString, QStringList > SelTLWidget::getTranslate()
{
    QStringList translate_list;
    QString cur_translate;

    for ( auto line : m_linE_map )
    {
        const QString tmp = line->text();
        if ( tmp.isEmpty() || translate_list.contains( tmp ) )
            continue;
        translate_list.append( tmp );
    }

    // 获取选中翻译
    QLineEdit* linE = m_linE_map[ m_btnGroup->checkedId() ];
    if ( !linE )
        return qMakePair( cur_translate, translate_list );
    cur_translate = linE->text();
    return qMakePair( cur_translate, translate_list );
}
