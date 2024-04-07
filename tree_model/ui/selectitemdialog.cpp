#include "selectitemdialog.h"
#include "ui_selectitemdialog.h"

using namespace arcirk::tree_widget;

SelectItemDialog::SelectItemDialog(tree_model::TreeItemModel* model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectItemDialog)
{
    ui->setupUi(this);

    treeView = new arcirk::tree_widget::TreeViewWidget(this);
    treeView->setModel(model);

    for (int i = 1; i < treeView->colorCount(); ++i) {
        treeView->hideColumn(i);
    }

    ui->verticalLayout->addWidget(treeView);

}

SelectItemDialog::~SelectItemDialog()
{
    delete ui;
}

void SelectItemDialog::accept()
{
    auto index = treeView->current_index();
    if(!index.isValid())
        return;
    //auto sort_model = (arcirk::tree_model::TreeSortModel*)treeView->model();
    //auto model = (arcirk::tree_model::TreeItemModel *)sort_model->sourceModel();
    m_result = m_model->to_object(index);

    return QDialog::accept();
}

json SelectItemDialog::result()
{
    return m_result;
}
