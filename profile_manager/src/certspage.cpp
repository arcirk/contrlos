//
// Created by admin on 19.05.2024.
//

// You may need to build the project (run Qt uic code generator) to get "ui_CertsPage.h" resolved

#include "../include/certspage.h"
#include "../ui/ui_CertsPage.h"
#include "../include/itable.hpp"

using namespace arcirk::profile_manager;
using namespace arcirk::database;

CertsPage::CertsPage(std::shared_ptr<ProfilesConf> &conf, QSqlDatabase &db, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::CertsPage) ,
        m_conf(conf),
        m_db(db)
{
    ui->setupUi(this);

    m_tool_bar = new widgets::TableToolBar(this);
    m_tool_bar->setButtonVisible(arcirk::enum_synonym(table_toolbar_buttons::table_options).c_str(), true);
    m_table = new widgets::TableWidget(this);
    m_table->setTableToolBar(m_tool_bar);
    ui->verticalLayout->addWidget(m_tool_bar);
    ui->verticalLayout->addWidget(m_table);

    auto model = new ITable<certificates>(this);
    model->get_conf()->reorder_columns(QList<QString>{"subject", "issuer", "not_valid_before", "not_valid_after", "private_key"});
    auto aliases = AliasesMap({
                                      qMakePair("name", "Имя"),
                                      qMakePair("synonym", "Представление"),
                                      qMakePair("private_key", "Контейнер"),
                                      qMakePair("subject", "Кому выдан"),
                                      qMakePair("issuer", "Кем выдан"),
                                      qMakePair("not_valid_before", "Начало действия"),
                                      qMakePair("not_valid_after", "Окончание действия"),
                                      qMakePair("parent_user", "Владелец"),
                                      qMakePair("serial", "Серийный номер"),
                              });
    model->set_columns_aliases(aliases);
    model->display_icons(true);
    model->get_conf()->set_default_icon(QIcon("://res/img/cert16NoKey.png"));
    m_table->setModel(model);
    m_table->hide_default_columns();
    m_table->hide_not_ordered_columns();

    auto certs = m_conf->cert_user()->getCertificates(false);
    model->from_json(certs, false);
}

CertsPage::~CertsPage() {
    delete ui;
}
