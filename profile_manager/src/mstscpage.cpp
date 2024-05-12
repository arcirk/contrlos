//
// Created by admin on 11.05.2024.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MstscPage.h" resolved

#include "../include/mstscpage.h"
#include "../ui/ui_mstscpage.h"
#include <itree.hpp>

using namespace arcirk::profile_manager;
using namespace arcirk::database;

MstscPage::MstscPage(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::MstscPage)
{
    ui->setupUi(this);

    m_tool_bar = new widgets::TableToolBar(this);
    m_tool_bar->setHierarchyState(true);
    ui->verticalLayout->addWidget(m_tool_bar);
    m_tree = new widgets::TreeViewWidget(this);
    ui->verticalLayout->addWidget(m_tree);

    init();
}

MstscPage::~MstscPage() {
    delete ui;
}

void MstscPage::init() {

    auto model = new ITree<mstsc_item>(this);
    auto aliases = AliasesMap({
          qMakePair("address", "Адрес"),
          qMakePair("name", "Наименование"),
          qMakePair("port", "Порт"),
          qMakePair("def_port", "Порт по умолчанию"),
          qMakePair("not_full_window", "Во весь экран"),
          qMakePair("width", "Ширина"),
          qMakePair("height", "Высота"),
          qMakePair("reset_user", "Сбросить пользователя"),
          qMakePair("user", "Пользователь"),
    });
    model->get_conf()->set_attribute_use(QList<QString>{
            "address", "port", "def_port", "not_full_window", "width","height", "reset_user", "user"
    }, attribute_use::for_element);

    model->set_columns_aliases(aliases);
    model->reorder_columns(QList<QString>{"name", "address", "port", "user"});
    m_tree->setModel(model);
    m_tree->hide_not_ordered_columns();

    m_tree->setTableToolBar(m_tool_bar);
}

