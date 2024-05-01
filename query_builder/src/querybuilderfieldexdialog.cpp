#include "../include/querybuilderfieldexdialog.h"
#include "../ui/ui_querybuilderfieldexdialog.h"

#include <itree.hpp>
#include <QStringListModel>
#include <QMessageBox>
#include <QToolButton>
#include <QMap>
#include "../include/querybuilderforeginkeydialog.h"

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::query_builder_ui), field_restrictions,
    (std::string, type)
    (std::string, name)
    (std::string, query)
    (sql_foreign_key, value)
);

using namespace arcirk::query_builder_ui;


QueryBuilderFieldExDialog::QueryBuilderFieldExDialog(QWidget *parent, const QUuid& package_uuid) :
    QDialog(parent),
    ui(new Ui::QueryBuilderFieldExDialog)
{
    ui->setupUi(this);

    mnu_index_type = new QMenu(this);
    //m_primary = new QAction("Первичный ключ", mnu_index_type);
    m_ext = new QAction("Внешний ключ", mnu_index_type);
    m_ext->setObjectName("ExtKey");
    m_check = new QAction("Проверочное ограничение", mnu_index_type);
    m_compare = new QAction("Сравнение", mnu_index_type);
    mnu_index_type->addAction(m_ext);
    mnu_index_type->addAction(m_check);
    mnu_index_type->addAction(m_compare);

    ui->btnAdd->setMenu(mnu_index_type);

    m_table = new TableWidget(this);
//    treeView->enable_sort(false);
//    treeView->set_drag_group(false);

    auto model = new ITable<field_restrictions>(this);
//    model->set_hierarchical_list(false);
//    model->enable_drag_and_drop(false);
    //model->set_enable_rows_icons(false);
    auto aliases = AliasesMap({
        qMakePair("type", "Тип"),
        qMakePair("name", "Имя"),
        qMakePair("query", "Подробности")
    });
    model->set_columns_aliases(aliases);
    model->reorder_columns(QList<QString>{
        "type", "name", "query"
    });
    m_table->setModel(model);

    for (int i = 0; i < model->columnCount(); ++i) {
        if(model->get_conf()->columns_order().indexOf(model->column_name(i)) == -1)
            m_table->hideColumn(i);
    }

    ui->verticalLayout->addWidget(m_table);

    int i = sqlite_types_qt.indexOf("TEXT");
    ui->cmbDataType->setModel(new QStringListModel(sqlite_types_qt, this));
    ui->cmbDataType->setCurrentIndex(i);

    connect(m_ext, &QAction::triggered, this, &QueryBuilderFieldExDialog::onMenuClicked);
    connect(m_check, &QAction::triggered, this, &QueryBuilderFieldExDialog::onMenuClicked);
    connect(m_compare, &QAction::triggered, this, &QueryBuilderFieldExDialog::onMenuClicked);
    connect(ui->btnDelete, &QToolButton::clicked, this, &QueryBuilderFieldExDialog::onBtnDeleteClicked);
    connect(ui->btnEdit, &QToolButton::clicked, this, &QueryBuilderFieldExDialog::onBtnEditClicked);

    m_package_uuid = package_uuid;

    m_field = ibase_object_structure();
    m_field.ref = generate_uuid();
    m_field.package_ref = to_byte(to_binary(m_package_uuid)));
    m_field.parent = to_nil_uuid();
}

QueryBuilderFieldExDialog::~QueryBuilderFieldExDialog()
{
    delete ui;
}

void QueryBuilderFieldExDialog::set_database_structure(ITable<ibase_object_structure> *structure)
{
    m_structure = structure;
}

ibase_object_structure QueryBuilderFieldExDialog::getField()
{
    m_field.name = ui->txtFieldName->text().toStdString();
    m_field.alias =  m_field.name;
    m_field.data_type = ui->cmbDataType->currentText().toStdString();
    m_field.object_type = "field";
    m_field.size = ui->spinSize1->value();
    QStringList lst{};
    if(ui->chkPrimaryKey->isChecked()){
        lst.append("PRIMARY KEY AUTOINCREMENT");
    }
    if(ui->chkNotNull->isChecked()){
        lst.append("NOT NULL");
    }
    if(ui->chkUnique->isChecked()){
        lst.append("UNIQUE");
    }

    m_field.query = lst.join("|").toStdString();
    m_field.def_value = ui->txtDefValue->text().toStdString();

    return m_field;
}

void QueryBuilderFieldExDialog::setData(const ibase_object_structure &value)
{
    m_field = value;
    ui->txtFieldName->setText(m_field.name.c_str());
    ui->cmbDataType->setCurrentIndex(sqlite_types_qt.indexOf(m_field.data_type.c_str()));
    ui->spinSize1->setValue(m_field.size);
    auto lst = QString(m_field.query.c_str()).split("|");
    ui->chkPrimaryKey->setChecked(lst.indexOf("PRIMARY KEY AUTOINCREMENT"));
    ui->chkNotNull->setChecked(lst.indexOf("NOT NULL"));
    ui->chkUnique->setChecked(lst.indexOf("UNIQUE"));
    ui->chkDefValue->setChecked(!m_field.def_value.empty());
    ui->txtDefValue->setText(m_field.def_value.c_str());
}

std::string QueryBuilderFieldExDialog::get_query(const sql_foreign_key& value)
{
    std::string result;
//    if(!value.name.empty()){
//        result.append("CONSTRAINT ");
//        result.append(value.name);
//        result.append(" ");
//    }
//
//    result.append("REFERENCES ");
//    result.append(value.refereces_table + " (");
//    result.append(value.refereces_field + ")");
//
//    json events = json::array();
//    auto str = byte_array_to_string(value.events);
//    if(json::accept(str)){
//        events = json::parse(str);
//    }
//
//    for (auto itr = events.begin(); itr != events.end(); ++itr) {
//        json e = *itr;
//        result.append(" ");
//        result.append(e.get<std::string>());
//    }
//
//    if(!value.deferrable.empty()){
//        result.append(" ");
//        result.append(value.deferrable);
//    }
//
//    if(!value.deferred.empty()){
//        result.append(" INITIALLY ");
//        result.append(value.deferred);
//    }

    return result;
}

void QueryBuilderFieldExDialog::onMenuClicked()
{

    auto action = qobject_cast<QAction*>(sender());
    if(action){
        if(action->objectName() == "ExtKey"){
            auto dlg = QueryBuilderForeginKeyDialog(this);
            dlg.set_database_structure(m_structure);
            if(dlg.exec()){
                auto result = dlg.getResult();
                result.field = ui->txtFieldName->text().toStdString();
                result.query = get_query(result);
                auto item = field_restrictions();
                item.name = result.name;
                item.query = result.query;
                item.type = "FOREIGN KEY";
                item.value = result;
                auto model = (ITable<field_restrictions>*)m_table->model();
                model->add_struct(item);
            }
        }
    }

}


void QueryBuilderFieldExDialog::onBtnEditClicked()
{
    auto index = m_table->currentIndex();
    if(!index.isValid())
        return;

    auto model = (ITable<field_restrictions>*)m_table->model();
    auto object = model->object(index);
    if(object.type == "FOREIGN KEY"){
        auto dlg = QueryBuilderForeginKeyDialog(this);
        dlg.set_database_structure(m_structure);
        dlg.set_object(object.value);
        if(dlg.exec()){
            auto result = dlg.getResult();
            result.field = ui->txtFieldName->text().toStdString();
            result.query = get_query(result);
            auto item = field_restrictions();
            item.name = result.name;
            item.query = result.query;
            item.type = "FOREIGN KEY";
            item.value = result;
            auto model = (ITable<field_restrictions>*)m_table->model();
            model->set_struct(item, index);
        }
    }
}


void QueryBuilderFieldExDialog::onBtnDeleteClicked()
{

    auto index = m_table->currentIndex();
    if(!index.isValid())
        return;

    if(QMessageBox::question(this, "Удаление", "Удалить выбранную строку?") == QMessageBox::No)
        return;

    auto model = (ITable<field_restrictions>*)m_table->model();
    model->remove(index);

}

