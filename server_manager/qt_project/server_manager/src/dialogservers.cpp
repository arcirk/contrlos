//
// Created by admin on 07.06.2024.
//

// You may need to build the project (run Qt uic code generator) to get "ui_DialogServers.h" resolved

#include "../include/dialogservers.h"
#include "../ui/ui_dialogservers.h"
#include <itable.hpp>
#include "../include/sql/datautils.h"
#include <global.hpp>
#include <sql/query_builder.hpp>
#include <QObject>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace arcirk::widgets;
using namespace arcirk::database;

DialogServers::DialogServers(QSqlDatabase& db, QWidget *parent) :
        QDialog(parent),
        ui(new Ui::DialogServers),
        m_db(db)
{
    ui->setupUi(this);

    m_table = new arcirk::widgets::TableWidget(this);
    m_tool_bar = new arcirk::widgets::TableToolBar(this);
    m_table->setTableToolBar(m_tool_bar);
    ui->verticalLayout->addWidget(m_tool_bar);
    ui->verticalLayout->addWidget(m_table);

    auto empty = servers();
    empty.ref = to_byte(to_binary(QUuid()));
    empty.uuid = to_byte(to_binary(QUuid()));

    auto model = new ITable<servers>(empty, this);
    AliasesMap aliases = {
            qMakePair("name", "Имя сервера"),
            qMakePair("host", "Адрес сервера"),
            qMakePair("user", "Пользователь"),
            qMakePair("hash", "Хеш пользователя"),
            qMakePair("uuid", "Идентификатор")

    };
    model->set_columns_aliases(aliases);
    model->reorder_columns(QList<QString>({
        "name",
        "host",
        "user"
    }));

    model->get_conf()->set_column_role("host", editor_inner_role::editorIpAddress);
    model->get_conf()->header_column("name")->select_type = false;

    m_table->setModel(model);
    m_table->hide_not_ordered_columns();
    m_table->set_inners_dialogs(true);

    init_model();

    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отмена");

    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &DialogServers::buttonBoxClicked);

}

DialogServers::~DialogServers() {
    delete ui;
}

void DialogServers::init_model() {

    if(!m_db.isOpen())
        return;

    auto model = (ITable<servers>*)m_table->get_model();
    auto utils = arcirk::database::builder::query_builder();
    auto rows = utils.get_table_rows<servers, tables>(m_db, tables::tbServers);
    if(!rows.empty()){
        model->set_rows(rows);
    }

}

void DialogServers::buttonBoxClicked(QAbstractButton *button) {
    if(button == ui->buttonBox->button(QDialogButtonBox::Ok)){
        accept();
    }else if(button == ui->buttonBox->button(QDialogButtonBox::Cancel)){
        QDialog::close();
    }
}

void DialogServers::accept() {
    QDialog::accept();
}
