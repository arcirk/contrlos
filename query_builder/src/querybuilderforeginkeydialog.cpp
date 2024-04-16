#include "../include/querybuilderforeginkeydialog.h"
#include "../ui/ui_querybuilderforeginkeydialog.h"
#include <QComboBox>
#include <QCheckBox>

using namespace arcirk::query_builder_ui;

QueryBuilderForeginKeyDialog::QueryBuilderForeginKeyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QueryBuilderForeginKeyDialog)
{
    ui->setupUi(this);

    ui->cmbMatch->setModel(new QStringListModel(m_match_events, this));
    ui->cmbOnDelete->setModel(new QStringListModel(m_events, this));
    ui->cmbOnUpdate->setModel(new QStringListModel(m_events, this));
    ui->cmbDeferable->setModel(new QStringListModel(m_deferrable, this));
    ui->cmbDeferred->setModel(new QStringListModel(m_deferred, this));

    connect(ui->cmbTables, &QComboBox::currentTextChanged, this, &QueryBuilderForeginKeyDialog::onCmbTablesCurrentTextChanged);
    connect(ui->chkIsName, &QCheckBox::clicked, this, &QueryBuilderForeginKeyDialog::onCmbSelectedItem);
    connect(ui->chkMatch, &QCheckBox::clicked, this, &QueryBuilderForeginKeyDialog::onCmbSelectedItem);
    connect(ui->chkOnDelete, &QCheckBox::clicked, this, &QueryBuilderForeginKeyDialog::onCmbSelectedItem);
    connect(ui->chkOnUpdate, &QCheckBox::clicked, this, &QueryBuilderForeginKeyDialog::onCmbSelectedItem);
}

QueryBuilderForeginKeyDialog::~QueryBuilderForeginKeyDialog()
{
    delete ui;
}

void QueryBuilderForeginKeyDialog::set_database_structure(ITree<ibase_object_structure> *structure){

    m_structure = structure;
    if(!m_structure)
        return;

    auto parent = m_structure->find(m_structure->column_index("object_type"), "tablesRoot");
    if(parent.isValid()){
        auto tbl = m_structure->array(parent);
        foreach (auto itr, tbl) {
            m_tables.append(itr.name.c_str());
        }
        ui->cmbTables->setModel(new QStringListModel(m_tables, this));
    }

}

void QueryBuilderForeginKeyDialog::accept()
{
    m_result = sql_foreign_key();
    m_result.refereces_table = ui->cmbTables->currentText().toStdString();
    m_result.refereces_field = ui->cmbFields->currentText().toStdString();

    auto events = json::array();

    if(ui->chkOnUpdate->isChecked()){
        auto val = QString("on update %1").arg(ui->cmbOnUpdate->currentText()).toUpper().toStdString();
        events.push_back(val);
    }
    if(ui->chkOnDelete->isChecked()){
        auto val = QString("on delete %1").arg(ui->cmbOnDelete->currentText()).toUpper().toStdString();
        events.push_back(val);
    }
    if(ui->chkMatch->isChecked()){
        auto s = ui->cmbMatch->currentText();
        if(s == "FULL")
            s = "[FULL]";
        auto val = QString("match %1").arg(s).toUpper().toStdString();
        events.push_back(val);
    }

    m_result.events = string_to_byte_array(events.dump());
    m_result.deferrable = ui->cmbDeferable->currentText().toStdString();
    m_result.deferred = ui->cmbDeferred->currentText().toStdString();

    if(ui->chkIsName->isChecked()){
        m_result.name = ui->txtIsName->text().toStdString();
    }

    return QDialog::accept();
}

void QueryBuilderForeginKeyDialog::set_object(const sql_foreign_key &object)
{
    //m_result = object; //sql_foreign_key(object);
    ui->cmbTables->setCurrentIndex(m_tables.indexOf(object.refereces_table.c_str()));
    onCmbTablesCurrentTextChanged(ui->cmbTables->currentText());
    ui->cmbFields->setCurrentIndex(m_fields.indexOf(object.refereces_field.c_str()));

    json events = json::array();
    auto str = byte_array_to_string(object.events);
    if(json::accept(str)){
        events = json::parse(str);
    }

    for (auto itr = events.begin(); itr != events.end(); ++itr) {
        json e = *itr;
        QString str = e.get<std::string>().c_str();
        if(str.indexOf("ON DELETE") != -1){
            ui->chkOnDelete->setChecked(true);
            auto val = str.right(str.length() - QString("ON DELETE").length()).trimmed();
            ui->cmbOnDelete->setCurrentIndex(m_events.indexOf(val));
        }else if(str.indexOf("ON UPDATE") != -1){
            ui->chkOnUpdate->setChecked(true);
            auto val = str.right(str.length() - QString("ON UPDATE").length()).trimmed();
            ui->cmbOnUpdate->setCurrentIndex(m_events.indexOf(val));
        }else if(str.indexOf("MATCH") != -1){
            ui->chkMatch->setChecked(true);
            auto val = str.right(str.length() - QString("MATCH").length()).trimmed();
            ui->cmbMatch->setCurrentIndex(m_match_events.indexOf(val));
        }
    }
    ui->cmbDeferable->setCurrentIndex(m_deferrable.indexOf(object.deferrable.c_str()));
    ui->cmbDeferred->setCurrentIndex(m_deferred.indexOf(object.deferred.c_str()));
    if(!object.name.empty()){
        ui->chkIsName->setChecked(true);
        ui->txtIsName->setText(object.name.c_str());
    }

    form_control();
}

void QueryBuilderForeginKeyDialog::form_control()
{
    ui->cmbFields->setEnabled(true);
    ui->chkMatch->setEnabled(true);
    ui->chkIsName->setEnabled(true);
    ui->chkOnDelete->setEnabled(true);
    ui->chkOnUpdate->setEnabled(true);
    ui->lblFields->setEnabled(true);
    ui->cmbDeferable->setEnabled(true);
    ui->cmbDeferred->setEnabled(true);
    ui->cmbMatch->setEnabled(ui->chkMatch->isChecked());
    ui->txtIsName->setEnabled(ui->chkIsName->isChecked());
    ui->cmbOnDelete->setEnabled(ui->chkOnDelete->isChecked());
    ui->cmbOnUpdate->setEnabled(ui->chkOnUpdate->isChecked());
}

void QueryBuilderForeginKeyDialog::onCmbTablesCurrentTextChanged(const QString &arg1)
{
    if(!m_structure)
        return;

    auto parent = m_structure->find(m_structure->column_index("name"), arg1);
    if(parent.isValid()){
        auto fld = m_structure->array(parent);
        foreach (auto itr, fld) {
            m_fields.append(itr.name.c_str());
        }
        ui->cmbFields->setModel(new QStringListModel(m_fields, this));
    }
    ui->cmbFields->setEnabled(true);
    ui->chkMatch->setEnabled(true);
    ui->chkIsName->setEnabled(true);
    ui->chkOnDelete->setEnabled(true);
    ui->chkOnUpdate->setEnabled(true);
    ui->cmbDeferable->setEnabled(true);
    ui->cmbDeferred->setEnabled(true);
}

void QueryBuilderForeginKeyDialog::onCmbSelectedItem(bool check)
{
    auto chk = qobject_cast<QCheckBox*>(sender());
    if(!chk)
        return;
    if(chk->objectName() == "chkMatch"){
        ui->cmbMatch->setEnabled(check);
    }else if(chk->objectName() == "chkIsName"){
        ui->txtIsName->setEnabled(check);
    }else if(chk->objectName() == "chkOnDelete"){
        ui->cmbOnDelete->setEnabled(check);
    }else if(chk->objectName() == "chkOnUpdate"){
        ui->cmbOnUpdate->setEnabled(check);
    }
}

