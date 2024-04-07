#ifndef IS_OS_ANDROID
#include "selectgroupdialog.h"
#include "ui_selectgroupdialog.h"
#include "../sort/treesortmodel.h"

using namespace arcirk::tree_widget;

SelectGroupDialog::SelectGroupDialog(arcirk::tree_model::TreeItemModel * model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectGroupDialog)
{
    ui->setupUi(this);

    auto sort_model = new arcirk::tree_model::TreeSortModel(this);
    sort_model->addFilter("is_group", 1);
    sort_model->setSourceModel(model);

    treeView = new arcirk::tree_widget::TreeViewWidget(this);
    treeView->setModel(sort_model);

    ui->verticalLayout->addWidget(treeView);
}

SelectGroupDialog::~SelectGroupDialog()
{
    delete ui;
}

void SelectGroupDialog::accept()
{
    auto index = treeView->currentIndex();
    if(!index.isValid())
        return;
    auto sort_model = (arcirk::tree_model::TreeSortModel*)treeView->model();
    auto model = (arcirk::tree_model::TreeItemModel *)sort_model->sourceModel();
    m_result = model->to_object(sort_model->mapToSource(index));

    return QDialog::accept();
}

json SelectGroupDialog::result() const
{
    return m_result;
}
#endif
