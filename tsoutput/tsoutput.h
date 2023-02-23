#ifndef TSOUTPUT_H
#define TSOUTPUT_H

#include <QWidget>
#include "sharefunction.h"

namespace Ui
{
class TSOUTPUT;
}

enum WorkMode
{
    qm2ts,
    t2xlsx
};

class TSOUTPUT : public QWidget
{
    Q_OBJECT

public:
    explicit TSOUTPUT( WorkMode workMode = WorkMode::qm2ts, QWidget* parent = nullptr );
    ~TSOUTPUT();

private:
    void initUI();
    void cleanWidgetLang(); // 清除左侧语言widget
    void MergeOutput(); // 合并导出
    void IndivOutput(); // 单独导出
    void setButtonStatus( bool enabled );

private slots:
    void Seldir();                       // 路径选择
    void ParseDir();                     // 解析路径
    QStringList Transform();             // QM转换TS
    void Output( QStringList fileList ); // 导出
    void ReadTranslate(TLUnit unit );   // 读取xml回调函数

    void on_pushButton_start_clicked();

    void on_checkBox_indiv_toggled(bool checked);

private:
    Ui::TSOUTPUT* ui;
    WorkMode m_workMode;
    QList< tinyxml2::XMLDocument* > m_doc_list;
    QList< QHash< int, TLUnit > > m_dic_list;
    QVector< QPair< QString, QString>> m_file_lang;
};

#endif // TSOUTPUT_H
