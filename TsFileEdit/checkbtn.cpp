#include "checkbtn.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QButtonGroup>

CheckBtn::CheckBtn( QWidget* parent )
    : QWidget( parent )
    , m_btn( nullptr )
    , m_chk( nullptr )
{
    CheckBtn( 0, "", parent );
}
CheckBtn::CheckBtn( int id, QString name, QWidget* parent )
    : QWidget( parent )
    , m_btn( nullptr )
    , m_chk( nullptr )
    , m_id( id )
{
    m_btn = new QPushButton( name, this );
    m_btn->setCheckable( true );
    m_btn->setAutoExclusive( true );
    if( !name.isEmpty() )
        m_btn->setToolTip( name );
    m_btn->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect( m_btn, &QPushButton::clicked, this, [=] { emit clicked( m_id ); } );
    m_chk = new QCheckBox( "", this );
    QHBoxLayout* hlayout = new QHBoxLayout( this );
    hlayout->setContentsMargins( 0, 0, 0, 0 );
    hlayout->setSpacing( 3 );
    hlayout->addWidget( m_chk );
    hlayout->addWidget( m_btn );
}

bool CheckBtn::isChecked()
{
    return m_chk->isChecked();
}

void CheckBtn::setChkEnabled( bool enabled)
{
    m_chk->setEnabled( enabled );
}

void CheckBtn::setData( QVariant data )
{
    m_data = data;
}

QVariant CheckBtn::getData()
{
    return m_data;
}

int CheckBtn::getID()
{
    return m_id;
}

void CheckBtn::setButtonGroup( QButtonGroup* g)
{
    g->addButton( m_btn );
}
