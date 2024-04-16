#include "../include/querybuilderadditionallywidget.h"
#include "../ui/ui_querybuilderadditionallywidget.h"
#include <QCheckBox>
#include <QRadioButton>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSplitter>
#include <QLabel>
#include "../include/querybuilderfieldexdialog.h"
#include "../include/sql/query_builder.hpp"

using namespace arcirk::widgets;
using namespace arcirk::query_builder_ui;
using namespace arcirk::database;

QueryBuilderAdditionallyWidget::QueryBuilderAdditionallyWidget(QWidget *parent, const QUuid& package_uuid) :
    QWidget(parent),
    ui(new Ui::QueryBuilderAdditionallyWidget)
{
    ui->setupUi(this);
    m_package_uuid = package_uuid;

    connect(ui->chkLimit, &QCheckBox::toggled, this, &QueryBuilderAdditionallyWidget::onChkLimitToggled);
    connect(ui->radioSelect, &QRadioButton::clicked, this, &QueryBuilderAdditionallyWidget::onRatioClicked);
    connect(ui->radioCreateTempTable, &QRadioButton::clicked, this, &QueryBuilderAdditionallyWidget::onRatioClicked);
    connect(ui->radioDeleteTempTable, &QRadioButton::clicked, this, &QueryBuilderAdditionallyWidget::onRatioClicked);
    connect(ui->radioCreateTable, &QRadioButton::clicked, this, &QueryBuilderAdditionallyWidget::onRatioClicked);
    connect(ui->radioDeleteTable, &QRadioButton::clicked, this, &QueryBuilderAdditionallyWidget::onRatioClicked);

    ui->radioSelect->setChecked(true);

    m_toolBarField = new TableToolBar(this);
    m_table = new TableWidget(this);
    m_table->setModel(create_model("qbAdditionalTable"));
    m_table->setTableToolBar(m_toolBarField);
    lblTop = new QLabel(this);
    lblTop->setText("Поля таблицы");
    ui->verticalLayout_5->addWidget(lblTop);
    ui->verticalLayout_5->addWidget(m_toolBarField);
    ui->verticalLayout_5->addWidget(m_table);

    lblBottom = new QLabel(this);
    lblBottom->setText("Индексы таблицы");
    m_toolBarIndices = new TableToolBar(this);
    m_table_indexes = new TableWidget(this);
    m_table_indexes->setModel(create_model("qbAdditionalIndex"));
    m_table_indexes->setTableToolBar(m_toolBarIndices);

    ui->verticalLayout_6->addWidget(lblBottom);
    ui->verticalLayout_6->addWidget(m_toolBarIndices);
    ui->verticalLayout_6->addWidget(m_table_indexes);
//
//    treeView->enable_sort(false);
//    treeViewIndexes->enable_sort(false);
//
    hide_columns(m_table);
    hide_columns(m_table_indexes);

    m_current_type = 0;
    m_radio.insert(0, ui->radioSelect);
    m_radio.insert(1, ui->radioCreateTempTable);
    m_radio.insert(2, ui->radioDeleteTempTable);
    m_radio.insert(3, ui->radioCreateTable);
    m_radio.insert(4, ui->radioDeleteTable);

    auto m_connection = QSqlDatabase::database("private.sqlite");
    auto rc = m_connection.exec(QString("select * from qbPackets where ref = '%1'").arg(package_uuid.toString(QUuid::WithoutBraces)));

    while (rc.next()) {
        ui->txtTenpTableName->setText(rc.value("name").toString());
        m_current_type = rc.value("type").toInt();

        ui->chkDistinct->setChecked(rc.value("use_distinct").toInt() > 0);
        ui->chkLimit->setChecked(rc.value("use_limit").toInt() > 0);
        ui->spinBox->setValue(rc.value("row_limit").toInt());
    }

    if(m_current_type >=0 && m_current_type < m_radio.size()){
        m_radio[m_current_type]->setChecked(true);
        form_control(m_radio[m_current_type]);
    }

    connect(m_table, &TableWidget::toolBarItemClicked, this, &QueryBuilderAdditionallyWidget::onToolBarFieldItemClicked);
    connect(m_table_indexes, &TableWidget::toolBarItemClicked, this, &QueryBuilderAdditionallyWidget::onToolBarIndexItemClicked);

}

QueryBuilderAdditionallyWidget::~QueryBuilderAdditionallyWidget()
{
    delete ui;
}

void QueryBuilderAdditionallyWidget::save_to_database()
{
    //QList<QString> m_btn{"radioSelect", "radioCreateTempTable", "radioDeleteTempTable", "radioCreateTable", "radioDeleteTable"};
    auto m_connection = QSqlDatabase::database("private.sqlite");
    Q_ASSERT(m_connection.isOpen());
    QString m_package = m_package_uuid.toString(QUuid::WithoutBraces);
    auto obj = builder::query_builder::object<query_builder_packet>(m_connection, QString("select * from qbPackets where ref = '%1'").arg(m_package));
    obj.name = ui->txtTenpTableName->text().toStdString();
    obj.row_limit = ui->spinBox->value();
    obj.type = m_current_type;
    obj.use_limit = ui->chkLimit->isChecked();
    obj.use_distinct = ui->chkDistinct->isChecked();
    auto b = builder::query_builder();
    b.use(pre::json::to_json(obj));
    m_connection.exec(b.update("qbPackets", true).where(json{{"ref", obj.ref}}, true).prepare().c_str());
    auto model = m_table->model();
    //model->reset_sql_data();
}

void QueryBuilderAdditionallyWidget::onChkLimitToggled(bool checked)
{
    ui->spinBox->setEnabled(checked);
}


void QueryBuilderAdditionallyWidget::onRatioClicked()
{
    QRadioButton* btn = qobject_cast<QRadioButton*>(sender());
    Q_ASSERT(btn!=0);
    form_control(btn);
    QList<QString> m_btn{"radioSelect", "radioCreateTempTable", "radioDeleteTempTable", "radioCreateTable", "radioDeleteTable"};
    selectedQueryType((sql_global_query_type)m_btn.indexOf(btn->objectName()));
    m_current_type  = m_btn.indexOf(btn->objectName());
}

void QueryBuilderAdditionallyWidget::onToolBarFieldItemClicked(const QString &buttonName)
{
    if(buttonName == "add_item"){
        onBtnAddField();
    }else if(buttonName == "edit_item"){
        onBtnEditField();
    }
}

void QueryBuilderAdditionallyWidget::onToolBarIndexItemClicked(const QString &buttonName)
{
//    auto index = m_table->currentIndex();
//    if(!index.isValid())
//        return;
//
//    auto model = (ITableIBaseModel*)m_table->model();
//    auto object = model->object(index);
//
//    auto dlg = QueryBuilderFieldExDialog(this);
//    dlg.set_database_structure(m_structure);
//    dlg.setData(object);
//    if(dlg.exec()){
//        auto result = dlg.getField();
//        auto model = (ITreeIBaseModel*)treeView->get_model();
//        model->set_struct(result, index);
//    }

}

void QueryBuilderAdditionallyWidget::onBtnAddField()
{
//    auto dlg = QueryBuilderFieldExDialog(this);
//    dlg.set_database_structure(m_structure);
//    if(dlg.exec()){
//        auto result = dlg.getField();
//        auto model = (ITreeIBaseModel*)treeView->get_model();
//        model->add_struct(result);
//    }
}

void QueryBuilderAdditionallyWidget::onBtnEditField()
{
//    auto index = treeView->current_index();
//    if(!index.isValid())
//        return;
//    auto model = (ITreeIBaseModel*)treeView->get_model();
//    auto dlg = QueryBuilderFieldExDialog(this);
//    dlg.set_database_structure(m_structure);
//    dlg.setData(model->object(index));
//    if(dlg.exec()){
//        auto result = dlg.getField();
//        auto model = (ITreeIBaseModel*)treeView->get_model();
//        model->set_struct(result, index);
//    }
}

void QueryBuilderAdditionallyWidget::form_control(QWidget* btn)
{
    if(btn==0)
        return;
    lblBottom->setEnabled(false);
    lblTop->setEnabled(false);
    m_table->setEnabled(false);
    m_table_indexes->setEnabled(false);
    m_toolBarField->setEnabled(false);
    m_toolBarIndices->setEnabled(false);
    if(btn->objectName() == "radioSelect"){
        ui->txtTenpTableName->setEnabled(true);
        ui->groupBox->setEnabled(true);
    }else if(btn->objectName() == "radioCreateTempTable"){
        ui->txtTenpTableName->setEnabled(true);
        ui->groupBox->setEnabled(false);
    }else if(btn->objectName() == "radioDeleteTempTable"){
        ui->txtTenpTableName->setEnabled(true);
        ui->groupBox->setEnabled(false);
    }else if(btn->objectName() == "radioCreateTable"){
        ui->txtTenpTableName->setEnabled(true);
        lblBottom->setEnabled(true);
        lblTop->setEnabled(true);
        m_table->setEnabled(true);
        m_table_indexes->setEnabled(true);
        m_toolBarField->setEnabled(true);
        m_toolBarIndices->setEnabled(true);
        ui->groupBox->setEnabled(false);
    }else if(btn->objectName() == "radioDeleteTable"){
        ui->txtTenpTableName->setEnabled(true);
        ui->groupBox->setEnabled(false);
    }

}

ITableIBaseModel* QueryBuilderAdditionallyWidget::create_model(const QString &table_name)
{
    auto model = new ITableIBaseModel(this);
    model->get_conf()->reorder_columns(QList<QString>{"name", "data_type", "size", "query"});
    auto aliases = AliasesMap({
                                      qMakePair("name", "Поле"),
                                      qMakePair("data_type", "Тип"),
                                      qMakePair("size", "Размер"),
                                      qMakePair("query", "Значение по умолчанию")
                              });
    model->set_columns_aliases(aliases);
    model->display_icons(true);

//    model->set_user_sql_where(json{{"package_ref", m_package_uuid.toString(QUuid::WithoutBraces).toStdString()}});
//    model->set_connection(root_tree_conf::sqlIteMemoryConnection, "", table_name);
//    model->enable_database_changed();

    return model;
}

void QueryBuilderAdditionallyWidget::hide_columns(TableWidget *table)
{
    auto model = (ITableIBaseModel*)table->model();
    for (int i = 0; i < model->columnCount(); ++i) {
        auto column = model->column_name(i);
        if(model->get_conf()->columns_order().indexOf(column) !=-1)
            m_table->setColumnHidden(i, false);
        else
            m_table->setColumnHidden(i, true);
    }

}

