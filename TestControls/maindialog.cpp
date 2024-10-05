#include "maindialog.h"
#include "./ui_maindialog.h"
#include "itree.hpp"
#include <treeitemcheckbox.h>
#include <QLabel>
#include <QList>
#include <QTabWidget>
#include <QSpacerItem>
//#include <iface/iface.hpp>
#include <tablemodel.h>
#include <pairmodel.h>

using namespace arcirk::tasks;

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainDialog)
{
    ui->setupUi(this);

    auto lbl = new QLabel(this);
    lbl->setText("TreeItemTextLine");
    ui->gridLayoutCtrl->addWidget(lbl, 0,0);

    text = new TreeItemTextLine(this);
    ui->gridLayoutCtrl->addWidget(text, 0,1);

    auto chk = new TreeItemCheckBox(this);
    chk->setText("Добавить/Удалить кнопку выбора.");
    chk->setObjectName("selectButton");
    chk->setAlignment(Qt::AlignCenter | Qt::AlignLeft);
    ui->gridLayoutCtrl->addWidget(chk, 1,1);
    connect(chk, &TreeItemCheckBox::checkBoxClicked, this, &MainDialog::onCheckBoxClicked);

    chk = new TreeItemCheckBox(this);
    chk->setText("Добавить/Удалить кнопку очистки.");
    chk->setObjectName("eraseButton");
    chk->setAlignment(Qt::AlignCenter | Qt::AlignLeft);
    ui->gridLayoutCtrl->addWidget(chk, 2,1);
    connect(chk, &TreeItemCheckBox::checkBoxClicked, this, &MainDialog::onCheckBoxClicked);

    chk = new TreeItemCheckBox(this);
    chk->setText("Добавить/Удалить кнопку сохранения.");
    chk->setObjectName("saveButton");
    chk->setAlignment(Qt::AlignCenter | Qt::AlignLeft);
    ui->gridLayoutCtrl->addWidget(chk, 3,1);
    connect(chk, &TreeItemCheckBox::checkBoxClicked, this, &MainDialog::onCheckBoxClicked);

    chk = new TreeItemCheckBox(this);
    chk->setText("Добавить/Удалить кнопку выбора типа.");
    chk->setObjectName("selTypeButton");
    chk->setAlignment(Qt::AlignCenter | Qt::AlignLeft);
    ui->gridLayoutCtrl->addWidget(chk, 4,1);
    connect(chk, &TreeItemCheckBox::checkBoxClicked, this, &MainDialog::onCheckBoxClicked);

    chk = new TreeItemCheckBox(this);
    chk->setText("Добавить/Удалить рамку");
    chk->setObjectName("frameVisible");
    chk->setAlignment(Qt::AlignCenter | Qt::AlignLeft);
    connect(chk, &TreeItemCheckBox::checkBoxClicked, this, &MainDialog::onCheckBoxClicked);
    ui->gridLayoutCtrl->addWidget(chk, 5,1);

    chk = new TreeItemCheckBox(this);
    chk->setText("Автоотметка");
    chk->setObjectName("automark");
    chk->setAlignment(Qt::AlignCenter | Qt::AlignLeft);
    connect(chk, &TreeItemCheckBox::checkBoxClicked, this, &MainDialog::onCheckBoxClicked);
    ui->gridLayoutCtrl->addWidget(chk, 6,1);

    lbl = new QLabel(this);
    lbl->setText("Тип элемента");
    ui->gridLayoutCtrl->addWidget(lbl, 7,0);
    combo = new TreeItemComboBox(this);
    combo->setObjectName("ComboRole");
    auto model = new PairModel(combo);
    model->setContent(QList<DataPair>{
            qMakePair(QVariant("widgetText"), QVariant(editor_inner_role::editorText)),
            qMakePair(QVariant("widgetDirectoryPath"), QVariant(editor_inner_role::editorDirectoryPath)),
            qMakePair(QVariant("widgetFilePath"), QVariant(editor_inner_role::editorFilePath))
    });

    //std::cout << QVariant(tree_editor_inner_role::widgetText).typeId() << std::endl;
    combo->setModel(model);
    ui->gridLayoutCtrl->addWidget(combo, 7,1);
    connect(combo, &TreeItemComboBox::valueChanged, this, &MainDialog::onComboBoxValueChanged);
//
    lbl = new QLabel(this);
    lbl->setText("Вариант");
    ui->gridLayoutCtrl->addWidget(lbl, 8,0);

    variant = new TreeItemVariant(this);
    variant->setFrame(true);
    variant->selectType(true);
    ui->gridLayoutCtrl->addWidget(variant, 8,1);

    auto tb = new TableToolBar(this);
    ui->verticalLayout->addWidget(tb);
    treeView = new TreeViewWidget(this);
    treeView->setTableToolBar(tb);
    treeView->set_inners_dialogs(true);
    ui->verticalLayout->addWidget(treeView);

    auto treeModel = new ITree<task_options>(this);
    treeModel->set_hierarchical_list(false);
    auto row = task_options();
    row.name = "Тестовая задача";
    treeModel->add_struct(row);

    treeView->setModel(treeModel);

    tableView = new arcirk::widgets::TableWidget(this);
    ui->verticalLayoutTable->addWidget(tableView);

    auto table_model = new ITable<test_table_struct>(this);
    auto table_row = test_table_struct();
    table_row.bool_field = true;
    table_row.byte_field = arcirk::to_byte(arcirk::to_binary(BJson(), arcirk::subtypeByte));
    table_row.list_field = arcirk::to_byte(arcirk::to_binary(json::array(), arcirk::subtypeArray));
    table_row.number_field = 1;
    table_row.string_field = "test string";
    table_model->add_struct(table_row);
    table_row = test_table_struct();
    table_row.bool_field = false;
    table_row.byte_field = arcirk::to_byte(arcirk::to_binary(BJson(), arcirk::subtypeByte));;
    table_row.list_field = arcirk::to_byte(arcirk::to_binary(json::array(), arcirk::subtypeArray));;
    table_row.number_field = 2;
    table_row.string_field = "test string";
    table_model->add_struct(table_row);
    table_model->set_read_only(false);
    table_model->reorder_columns(QList<QString>{
        "string_field", "number_field"
    });
    table_model->display_icons(true);
    table_model->setData(table_model->index(1,0), QIcon("://itemOver.png"), Qt::DecorationRole);
    tableView->setModel(table_model);

    tableList = new TableListWidget(this, editor_inner_role::editorText);
    ui->verticalLayoutTableList->addWidget(tableList);

    connect(tableView, &TableWidget::itemClicked, this, &MainDialog::onTableItemClicked);
}

void MainDialog::onTableItemClicked(const QModelIndex &index) {

    if(!index.isValid())
        return;

    auto data = index.data(TABLE_DATA);
    auto item = arcirk::widgets::item_data();
    item.from_json(arcirk::from_variant(data));

    std::cout << item.representation() << std::endl;

}

MainDialog::~MainDialog()
{
    delete ui;
}

void MainDialog::onCheckBoxClicked(int row, int col, bool state)
{
    auto chk = qobject_cast<TreeItemCheckBox*>(sender());
    if(chk){
        if(chk->objectName() == "selectButton"){
            text->setButtonVisible(arcirk::widgets::SelectButton, state);
        }else if(chk->objectName() == "frameVisible"){
            text->setFrame(state);
        }else if(chk->objectName() == "automark"){
            text->setAutoMarkIncomplete(state);
        }else if(chk->objectName() == "eraseButton"){
            text->setButtonVisible(arcirk::widgets::EraseButton, state);
        }else if(chk->objectName() == "saveButton"){
            text->setButtonVisible(arcirk::widgets::SaveButton, state);
        }else if(chk->objectName() == "selTypeButton"){
            text->setButtonVisible(arcirk::widgets::SelectTypeButton, state);
        }

    }
}

void MainDialog::onComboBoxValueChanged(int /*row*/, int /*col*/, const QVariant& value) {
    auto ctrl = qobject_cast<TreeItemComboBox*>(sender());
    if(ctrl){
        if(ctrl->objectName() == "ComboRole" && value.isValid()){
            auto role = (editor_inner_role)value.toInt();
            qDebug() << role;
            text->setRole(role);
        }

    }

}
