# TranslateToolBox
___

## 简介
---
TranslateToolBox是一个QT多语言开发的翻译维护工具，可用于维护管理多个项目，根据维护字典可快速生成新项目的翻译，保持多个项目翻译的统一性；并且能够将多个语言翻译导出为excel进行对比查错
## 功能
---
1. **QM文件转TS文件**
2. **TS/QM文件导出EXCEL文件**
	- 合并导出多个TS文件为一个EXCEL
	- 标记未完成的翻译
	- 标记翻译中的中文标点
	- 只进行导出未完成的翻译
	- 调用百度翻译对未完成的翻译进行翻译
3. **TS文件翻译快速匹配生成**
	- 根据翻译字典匹配中文生成对应翻译，并更新到TS文件中
	- 一个中文可匹配多个翻译，可在界面中选择使用哪个
	- 未能匹配的中文，可调用百度翻译API进行翻译
4. **翻译字典维护**
	- 可通过TS文件导入生成翻译字典，一个中文可有多个翻译
	- 可在界面中导入、删除、修改
	
## 使用
---
+ 本程序使用QT5.9.7+msvc2015编译，编译运行后会在（/bin）目录下生成config.ini文件，请根据需要修改ini配置文件
![config](/img/config.png)


## 界面
---
![qm2ts](/img/qm2ts.png)
![tsOutput](/img/tsOutput.png)
![tsEdit](/img/tsEdit.png)
![tsEdit_sel](/img/tsEdit_sel.png)
![dicEdit](/img/dicEdit.png)

