//
// Created by admin on 14.04.2024.
//

#ifndef QUERY_BUILDER_MAINDIALOG_H
#define QUERY_BUILDER_MAINDIALOG_H

#include <QDialog>
//#include "treeitemswidget.h"
#include <QTabWidget>

//using namespace arcirk::widgets;

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainDialog;
}
QT_END_NAMESPACE

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    MainDialog(QWidget *parent = nullptr);
    ~MainDialog();

private:
    Ui::MainDialog *ui;
    //TreeItemsWidget* m_databaseStructure;
    QTabWidget * m_tab;

    void init_form();

};
#endif //QUERY_BUILDER_MAINDIALOG_H
