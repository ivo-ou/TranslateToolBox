#ifndef TSFILEEDIT_H
#define TSFILEEDIT_H

#include <QWidget>
#include <QStackedWidget>
#include "tablewidget.h"
#include "checkbtn.h"
#include "sharefunction.h"

namespace Ui {
class TsFileEdit;
}

enum match_result{
    Success  = 0,
    PartMatch,
    Fail,
    Ignore
};

class TsFileEdit : public QWidget
{
    Q_OBJECT

public:
    explicit TsFileEdit(QWidget *parent = nullptr);
    ~TsFileEdit();

private:
    void InitUI();
    void LoadDic();
    void InitLangList(QStringList files);
    void ParseTsDir();
    void ClearWidget();
    void SetWidgetState( bool start );
    void ReadUnit( TLUnit unit );
    match_result TranslateMatch(QString langto, QString source, QStringList &translate_list , bool original);

private slots:
    void LoadTs2Table();
    void SelDicdir();                       // 字典路径选择
    void SelTsdir();                       // TS文件路径选择

private:
    Ui::TsFileEdit *ui;
    QStackedWidget *m_stackedWidget;
    QMap< int, QPair< QString, int>> m_header_index;   // 表格中语言与对应的列<序号<英语，列号>>
    QMultiHash< QString, QVector<QString> > *m_dic_vec;    // 原文，翻译
    QVector< TableWidget* > m_tblWidget_vec;
    QVector< CheckBtn* > m_chkBtn_vec;
    QMap<int, tinyxml2::XMLDocument* > m_xmlDoc_map;
    int m_cur_lang;     //正在处理的id
    uint m_cur_lang_index;     //正在处理的语言所在m_dic_vec中vector中序号
};

#endif // TSFILEEDIT_H
