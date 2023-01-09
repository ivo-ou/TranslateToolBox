#include "toolboxui.h"
#include "ui_toolboxui.h"
#include "tsoutput.h"
#include "dicedit.h"
#include "tsfileedit.h"

#include <QDebug>
#include <QButtonGroup>
#include <QResource>

ToolBoxUI::ToolBoxUI(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ToolBoxUI),
    btnGroup_func( nullptr )
{
    ui->setupUi(this);
    btnGroup_func = new QButtonGroup( this );
    connect(btnGroup_func, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled),
          [=](int id, bool checked){
                if( checked && id < ui->stackedWidget->count() )
                {
                    ui->stackedWidget->setCurrentIndex( id );
                    ui->label_title->setText( btnGroup_func->button( id )->text() );
                }});
    // 初始化界面
    Config::getInstance();
    initUI();
}

ToolBoxUI::~ToolBoxUI()
{
    delete ui;
    delete  btnGroup_func;
}

// 添加功能窗口
template<typename _Widget, class... _Types >
void ToolBoxUI::addFunc( _Types&&... _Args )
{
    _Widget* widget = nullptr;
     widget = new _Widget( std::forward< _Types >( _Args )... );
     int index = ui->stackedWidget->addWidget( widget );
     qDebug() << index << widget->windowTitle();
     QPushButton *btn_func = new QPushButton( widget->windowTitle() ,this );
     btn_func->setCheckable( true );
     ui->widget_btn->layout()->addWidget( btn_func );
     btnGroup_func->addButton( btn_func, index );
}

void ToolBoxUI::initUI()
{
    // 清空界面
    while ( ui->stackedWidget->count() ) {
        auto w = ui->stackedWidget->currentWidget();
        w->setParent( nullptr );
        ui->stackedWidget->removeWidget( w );
        delete w;
    }
    while ( ui->widget_btn->layout()->count() ) {
        auto w = ui->widget_btn->layout()->itemAt( 0 )->widget();
        w->setParent( nullptr );
        ui->widget_btn->layout()->removeWidget( w );
    }

    // 添加功能
    addFunc< TSOUTPUT >( WorkMode::qm2ts, this );
    addFunc< TSOUTPUT >( WorkMode::t2xlsx, this );
    addFunc< TsFileEdit >( this );
    addFunc< DicEdit >( this );

    // 选中第一个按钮
    QAbstractButton *firstBtn = btnGroup_func->button( 0 );
    if( firstBtn )
        firstBtn->setChecked( true );


}

