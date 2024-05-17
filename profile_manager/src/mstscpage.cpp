//
// Created by admin on 11.05.2024.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MstscPage.h" resolved

#include "../include/mstscpage.h"
#include "../ui/ui_mstscpage.h"
#include <itree.hpp>
#include "../include/mstscitemdialog.h"
#include "../include/sql/datautils.h"
#include "../global/sql/query_builder.hpp"
#include <QSqlError>
#include <QSqlQuery>

using namespace arcirk::profile_manager;
using namespace arcirk::database;
using namespace arcirk::widgets;

MstscPage::MstscPage(QSqlDatabase& db, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::MstscPage),
        m_db(db)
{
    ui->setupUi(this);

    m_tool_bar = new widgets::TableToolBar(this);
    m_tool_bar->setHierarchyState(true);
    ui->verticalLayout->addWidget(m_tool_bar);
    m_tree = new widgets::TreeViewWidget(this);
    ui->verticalLayout->addWidget(m_tree);

    init();
    initData();

    m_tool_bar->addButton("mstsc", QIcon("://res/img/mstsc.png"), false, -1, "Открыть диалог MSTSC");
    connect(m_tree, &TreeViewWidget::itemClicked, this, &MstscPage::onCurrentChanged);
    connect(m_tree, &TreeViewWidget::tableItemChanged, this, &MstscPage::onTableItemChanged);
    connect(m_tree, &TreeViewWidget::removeTreeRow, this, &MstscPage::onRemoveItem);
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
    model->get_conf()->set_row_icon(tree_rows_icons::Item, QIcon("://res/img/mstsc.png"));

    m_tree->setModel(model);
    m_tree->hide_not_ordered_columns();
    m_tree->setTableToolBar(m_tool_bar);
    m_tree->set_inners_dialogs(false);
    m_tree->allow_def_commands(false);
    connect(m_tree, &TreeViewWidget::toolBarItemClicked, this, &MstscPage::onToolBarItemClicked);
    connect(m_tree, &TreeViewWidget::removeTreeRow, this, &MstscPage::onRemoveItem);
}

void MstscPage::onToolBarItemClicked(const QString &buttonName) {

    json b = buttonName.toStdString();
    auto btn = b.get<table_toolbar_buttons>();
    if(btn == table_add_item){
        openItemEditDialog(true);
    }else if(btn == table_add_group){
        m_tree->openNewGroupDialog();
    }else if(btn == table_delete_item){
        m_tree->deleteItem();
    }else if(btn == table_edit_item){
        openItemEditDialog();
    }else if(btn == table_move_to_item){
        m_tree->openOpenMoveToDialog();
    }else if(btn == table_move_up_item){
        m_tree->moveUp();
    }else if(btn == table_move_down_item) {
        m_tree->moveDown();
    }else{
        if(buttonName == "mstsc"){

        }
    }

}

void MstscPage::openItemEditDialog(bool isNew) {

    auto model = (ITree<mstsc_item>*)m_tree->get_model();
    auto item = mstsc_item();
    auto index = m_tree->current_index();
    auto parent = QModelIndex();
    if(index.isValid()){
        if(model->is_group(index))
            parent = index;
        else
            parent = index.parent();
    }
    if(isNew){
        item.ref = to_byte(to_binary(QUuid::createUuid()));
        item.parent = to_byte(to_binary(model->ref(parent)));
        item.def_port = true;
        item.port = 3389;
    }else{
        if(model->is_group(index))
            return;
        item = model->object(index);
    }
    auto dlg = MstscItemDialog(item, this);
    QModelIndex new_index = {};
    if(dlg.exec()){
        if(isNew){
            new_index = model->add_struct(item, parent);
        }else{
            new_index = index;
            model->set_struct(item, index);
        }
    }
    if(new_index.isValid())
        update_database(new_index);

}

void MstscPage::initData() {

    auto item = mstsc_item();
    item.ref = to_byte(to_binary(QUuid::fromString("416a9257-788a-4115-a978-e1a4e6194f29")));
    item.parent = to_byte(to_binary(QUuid()));
    item.def_port = true;
    item.port = 0;
    item.predefined = true;
    item.name = "Список элементов меню";
    item.is_group = true;
    auto model = (ITree<mstsc_item>*)m_tree->get_model();
    auto index = model->add_struct(item, QModelIndex());
    model->set_row_not_move(index, true);

    if(!m_db.isOpen())
        return;
    using namespace arcirk::database::builder;
    auto query = builder::query_builder();
    auto rs = QSqlQuery(query.select().from(enum_synonym(tbMstscConnections)).prepare().c_str(), m_db);
    rs.exec();

    while (rs.next()){
        item = arcirk::database::mstsc_item();
        auto item_json = pre::json::to_json(item);
        QSqlRecord row = rs.record();

        for (int i = 0; i < row.count(); ++i) {
            std::string column_name = row.fieldName(i).toStdString();
            if(item_json.find(column_name) == item_json.end())
                continue;
            QVariant val = row.field(i).value();
//            auto t = val.typeName();
//            if(t)
//                std::cout << val.typeName() << " " << column_name << std::endl;
            if(val.userType() == QMetaType::QString && item_json[column_name].is_string())
                item_json[column_name] = val.toString().toStdString();
            else if(val.userType() == QMetaType::Double && item_json[column_name].is_number())
                item_json[column_name] = val.toDouble();
            else if(val.userType() == QMetaType::Int && item_json[column_name].is_number())
                item_json[column_name] = val.toInt();
            else if(val.userType() == QMetaType::Int && item_json[column_name].is_boolean())
                item_json[column_name] = val.toInt() > 0;
            else if(val.userType() == QMetaType::LongLong && item_json[column_name].is_boolean())
                item_json[column_name] = val.toLongLong() > 0;
            else if(val.userType() == QMetaType::LongLong && item_json[column_name].is_number())
                item_json[column_name] = val.toLongLong();
            else if(val.userType() == QMetaType::ULongLong && item_json[column_name].is_number())
                item_json[column_name] = val.toULongLong();
            else if(val.userType() == QMetaType::QByteArray && item_json[column_name].is_array())
                item_json[column_name] = qbyte_to_byte(val.toByteArray());
        }
        item = pre::json::from_json<mstsc_item>(item_json);
        auto parent_data = item_data(item_json["parent"]);
        auto parent_uuid = QUuid::fromRfc4122(byte_to_qbyte(parent_data.data()->data) );
        auto parent = model->find(parent_uuid);
        index = model->add_struct(item, parent);
        if(index.isValid()){
//            auto item_d = arcirk::widgets::item_data(item.icon);
//            if(item_d.data()->subtype == subtypeByte){
//                auto qba = QByteArray(reinterpret_cast<const char*>(item_d.data()->data.data()), item_d.data()->data.size());
//                QPixmap p;
//                p.loadFromData(qba);
//                auto icon = QIcon(p);
//                if(!icon.isNull())
//                    model->setData(index, QIcon(p), Qt::DecorationRole);
//            }
        }
    }
}

void MstscPage::onCurrentChanged(const QModelIndex &current) {

    auto model = m_tree->get_model();
    auto index = m_tree->current_index();

    if(!index.isValid())
        m_tool_bar->button("mstsc")->setEnabled(false);
    else
        m_tool_bar->button("mstsc")->setEnabled(!model->is_group(index));

}

void MstscPage::onRemoveItem(const json &object_removed) {
    update_database(object_removed);
}

void MstscPage::update_database(const QModelIndex &index) {

    if(!index.isValid())
        return;

    if(!m_db.isOpen())
        return;
    using namespace arcirk::database::builder;

    auto model = (ITree<mstsc_item>*)m_tree->get_model();
    auto object = model->object(index);

    auto query = builder::query_builder();
    auto rs = QSqlQuery(m_db);
    std::vector<std::tuple<std::string, arcirk::ByteArray>> blobs;
    rs.prepare(query.row_count().from(enum_synonym(tbMstscConnections)).where(json{{"ref", object.ref}}, blobs).prepare().c_str());
    auto ba = QByteArray(reinterpret_cast<const char*>(object.ref.data()), (qsizetype)object.ref.size());
    rs.bindValue(0, ba);
    rs.exec();
    int count = 0;
    if(rs.lastError().type() == QSqlError::NoError){
        while (rs.next()){
            count = rs.value(0).toInt();
        }
    }
    query.clear();
    query.use(pre::json::to_json(object));
    rs.clear();
    blobs.clear();

    m_db.transaction();
    std::string query_string;
    if(count > 0){
        query_string = query.update(enum_synonym(tbMstscConnections), blobs).where(json{{"ref", object.ref}}, blobs).prepare();
    }else{
        query_string = query.insert(enum_synonym(tbMstscConnections), blobs).prepare();
    }
    rs.prepare(query_string.c_str());
    for(auto itr : blobs){
        std::string key = std::get<0>(itr);
        ByteArray b = std::get<1>(itr);
        auto qba = QByteArray(reinterpret_cast<const char*>(b.data()), (qsizetype)b.size());
        rs.bindValue(key.c_str(), qba);
    }
    auto result = rs.exec();
    m_db.commit();
    if(!result){
        std::cerr << query_string << std::endl;
        std::cerr << rs.lastError().text().toStdString() << std::endl;
    }
}

void MstscPage::update_database(const json &object) {

    if(!m_db.isOpen())
        return;
    using namespace arcirk::database::builder;

    auto st = arcirk::secure_serialization<mstsc_item>(object);
}

void MstscPage::onTableItemChanged(const QModelIndex &index) {
    update_database(index);
}

