#ifndef PLUGINPROPERTYDIALOG_H
#define PLUGINPROPERTYDIALOG_H

#include <QDialog>
#include <ui/treeviewwidget.h>

namespace Ui {
class PluginPropertyDialog;
}

using namespace arcirk::tree_widget;
using namespace arcirk::tree_model;

class QPath{

public:
    explicit QPath(){}
    explicit QPath(const QString& p){ m_path = p;}
    ~QPath(){}

    QString path() const {return m_path;}

    void operator /=(const QString& p){
        m_path.append(QDir::separator());
        m_path.append(p);
        //return m_path;
    }

    bool exists(){return true;}
    bool mkpath(){return true;}


private:
    QString m_path;

};

namespace arcirk::plugins {
    class PluginPropertyDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit PluginPropertyDialog(const json& table, QWidget *parent = nullptr);
        ~PluginPropertyDialog();

        json result() const;

    private:
        Ui::PluginPropertyDialog *ui;
        TreeViewWidget* m_tree;
        TableToolBar* m_toolbar;

    };
}
#endif // PLUGINPROPERTYDIALOG_H
