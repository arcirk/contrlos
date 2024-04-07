#include "../include/pluginpropertydialog.h"
#include "../ui/ui_pluginpropertydialog.h"
#include <QLabel>
#include "iface/iface.hpp"
#include "../include/bankstatementsplugun.h"

using namespace arcirk::plugins;
using namespace arcirk::tree_model;

PluginPropertyDialog::PluginPropertyDialog(const json& table, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PluginPropertyDialog)
{
    ui->setupUi(this);

    m_toolbar = new TableToolBar(this);
    m_tree = new TreeViewWidget(this);
    auto model = new ITree<plugin_param>(this);
    model->set_column_aliases(QMap<QString, QString>{qMakePair("key", "Параметр"), qMakePair("value", "Значение")});
    model->set_columns_order(QList<QString>{"key", "value"});
    model->set_hierarchical_list(false);
    model->enable_drag_and_drop(false);
    model->set_enable_rows_icons(false);
    model->set_user_role_data("value", tree::WidgetRole, widgetVariantRole);
    model->set_user_role_data("key", tree::NotNullRole, true);
    model->set_table(table);

    m_tree->enable_sort(false);
    m_tree->setTableToolBar(m_toolbar);
    m_tree->set_inners_dialogs(true);
    m_tree->setModel(model);
    m_tree->hide_default_columns();

    ui->verticalLayout->addWidget(m_toolbar);
    ui->verticalLayout->addWidget(m_tree);

    setWindowTitle("Параметры");
}

PluginPropertyDialog::~PluginPropertyDialog()
{
    delete ui;
}

json PluginPropertyDialog::result() const
{
    auto model = m_tree->get_model();
    return model->to_table_model(QModelIndex());
}

