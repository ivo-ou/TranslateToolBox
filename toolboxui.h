#ifndef TOOLBOXUI_H
#define TOOLBOXUI_H

#include <QWidget>

namespace Ui {
class ToolBoxUI;
}

class QButtonGroup;

class ToolBoxUI : public QWidget
{
    Q_OBJECT

public:
    explicit ToolBoxUI(QWidget *parent = nullptr);
    ~ToolBoxUI();

private:
    template<typename _Widget, class... _Types >
    void addFunc( _Types&&... _Args );
    void initUI();

private:
    Ui::ToolBoxUI *ui;
    QButtonGroup *btnGroup_func;
};

#endif // TOOLBOXUI_H
