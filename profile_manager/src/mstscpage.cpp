//
// Created by admin on 11.05.2024.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MstscPage.h" resolved
#include <itree.hpp>
#include "../include/mstscpage.h"
#include "../ui/ui_mstscpage.h"
#include "../include/mstscitemdialog.h"
#include "../include/sql/datautils.h"
#include "../global/sql/query_builder.hpp"
#include <QSqlError>
#include <QSqlQuery>
#include <QHeaderView>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <sh/commandline.h>

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
    connect(m_tree, &TreeViewWidget::rowMove, this, &MstscPage::onRowMove);

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
            openMstscEditDialog();
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
        if(model->is_group(index)){
            m_tree->openOpenEditDialog();
            return;
        }
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
    if(new_index.isValid()){
        update_database(new_index);
        emit reset();
    }


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
    //auto rs = QSqlQuery(query.select().from(enum_synonym(tbMstscConnections)).prepare().c_str(), m_db);
    auto rs = QSqlQuery(query.select().from("MstscItemView").prepare().c_str(), m_db);
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
            item_json[column_name] = from_variant(val, item_json[column_name].type());
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

    //m_tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
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
    object.pos = index.row();

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

//    if(!m_db.isOpen())
//        return;
//    using namespace arcirk::database::builder;
//
//    auto st = arcirk::secure_serialization<mstsc_item>(object);
}

void MstscPage::onTableItemChanged(const QModelIndex &index) {
    update_database(index);
    emit reset();
}

QString MstscPage::cache_mstsc_directory() const {
    auto app_data = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    app_data.append("/mstsc");
    QDir f(app_data);
    if(!f.exists())
        f.mkpath(f.path());
    return f.path();
}

QString MstscPage::rdp_file_text() {
    QString text = "screen mode id:i:%1\n"
                   "use multimon:i:0\n"
                   "desktopwidth:i:%2\n"
                   "desktopheight:i:%3\n"
                   "session bpp:i:32\n"
                   "winposstr:s:0,3,0,0,%2,%3\n"
                   "compression:i:1\n"
                   "keyboardhook:i:2\n"
                   "audiocapturemode:i:0\n"
                   "videoplaybackmode:i:1\n"
                   "connection type:i:7\n"
                   "networkautodetect:i:1\n"
                   "bandwidthautodetect:i:1\n"
                   "displayconnectionbar:i:1\n"
                   "username:s:%4\n"
                   "enableworkspacereconnect:i:0\n"
                   "disable wallpaper:i:0\n"
                   "allow font smoothing:i:0\n"
                   "allow desktop composition:i:0\n"
                   "disable full window drag:i:1\n"
                   "disable menu anims:i:1\n"
                   "disable themes:i:0\n"
                   "disable cursor setting:i:0\n"
                   "bitmapcachepersistenable:i:1\n"
                   "full address:s:%5\n"
                   "audiomode:i:0\n"
                   "redirectprinters:i:1\n"
                   "redirectcomports:i:0\n"
                   "redirectsmartcards:i:1\n"
                   "redirectclipboard:i:1\n"
                   "redirectposdevices:i:0\n"
                   "autoreconnection enabled:i:1\n"
                   "authentication level:i:2\n"
                   "prompt for credentials:i:0\n"
                   "negotiate security layer:i:1\n"
                   "remoteapplicationmode:i:0\n"
                   "alternate shell:s:\n"
                   "shell working directory:s:\n"
                   "gatewayhostname:s:\n"
                   "gatewayusagemethod:i:4\n"
                   "gatewaycredentialssource:i:4\n"
                   "gatewayprofileusagemethod:i:0\n"
                   "promptcredentialonce:i:0\n"
                   "gatewaybrokeringtype:i:0\n"
                   "use redirection server name:i:0\n"
                   "rdgiskdcproxy:i:0\n"
                   "kdcproxyname:s:";

    return text;
}

void MstscPage::openMstscEditDialog(const QUuid& uuid) {

    QString file_name;
    auto model = (ITree<mstsc_item>*)m_tree->get_model();
    mstsc_item object;

    if(uuid.isNull()){
        auto current_index = m_tree->current_index();
        if(!current_index.isValid() || model->is_group(current_index))
            return;

        QDir dir(cache_mstsc_directory());
        object = model->object(current_index);
        file_name = dir.path() + "/";
        file_name.append(object.name.c_str());
        file_name.append(".rdp");
    }else{
        auto current_index = model->find(uuid);
        if(!current_index.isValid() || model->is_group(current_index))
            return;
        QDir dir(cache_mstsc_directory());
        object = model->object(current_index);
        file_name = dir.path() + "/";
        file_name.append(object.name.c_str());
        file_name.append(".rdp");
    }

    QFile f(file_name);

    if(!f.exists()) {
        if (f.open(QIODevice::WriteOnly)) {
            int screenMode = 2;
            if (object.not_full_window) {
                screenMode = 1;
            }
            QString rdp = rdp_file_text().arg(QString::number(screenMode), QString::number(object.width),
                                              QString::number(object.height), object.user.c_str(),
                                              object.address.c_str());
            f.write(rdp.toUtf8());
            f.close();
        }
    }

    editMstsc(f.fileName());

}

void MstscPage::editMstsc(const QString &fileName) {

    QString command;

    QFile f(fileName);
    if(!f.exists()){
        QMessageBox::critical(this, "Ошибка", "Настройка не найдена!");
        return;
    }

    command.append("mstsc /edit \"" + QDir::toNativeSeparators(fileName) + "\" & exit");

    auto cmd = CommandLine(this);
    QEventLoop loop{};

    auto started = [&cmd, &command]() -> void
    {
        cmd.send(command, CmdCommand::mstscEditFile);
    };
    loop.connect(&cmd, &CommandLine::started_process, started);

    auto output = [](const QByteArray& data) -> void
    {
        std::string result_ = to_utf(data.toStdString(), DEFAULT_CHARSET_);
    };
    loop.connect(&cmd, &CommandLine::output, output);

    auto err = [&loop, &cmd](const QString& data, int command) -> void
    {
        //qDebug() << __FUNCTION__ << data << command;
        cmd.stop();
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::error, err);

    auto state = [&loop]() -> void
    {
        loop.quit();
    };
    loop.connect(&cmd, &CommandLine::complete, state);

    cmd.start();
    loop.exec();
}

json MstscPage::get_mstsc_items() const {
    auto model = (ITree<mstsc_item>*)m_tree->get_model();
    auto root_uuid = QUuid::fromString("416a9257-788a-4115-a978-e1a4e6194f29");
    auto root = model->find(root_uuid);
    auto tree = json::object();
    model->to_tree(tree, root);
    return tree;
}

void MstscPage::onRowMove() {

    if(!m_db.isOpen())
        return;
    using namespace arcirk::database::builder;

    auto model = m_tree->get_model();
    auto root_uuid = QUuid::fromString("416a9257-788a-4115-a978-e1a4e6194f29");
    auto root = model->find(root_uuid);

    m_db.transaction();
    update_rows_positions(model, root);
    m_db.commit();

    emit reset();
}

void MstscPage::update_rows_positions(TreeModel *model, const QModelIndex &parent) {

    auto query = builder::query_builder();
    auto rs = QSqlQuery(m_db);

    for (int i = 0; i < model->rowCount(parent); ++i) {
        query.clear();
        rs.clear();
        auto index = model->index(i, 0, parent);
        query.use(json::object({
            {"pos", index.row()}
        }));
        auto object = pre::json::from_json<mstsc_item>(model->to_object(index));
        std::vector<std::tuple<std::string, arcirk::ByteArray>> blobs;
        rs.prepare(query.update(enum_synonym(tbMstscConnections)).where(json{{"ref", object.ref}}, blobs).prepare().c_str());
        for(auto itr : blobs){
            std::string key = std::get<0>(itr);
            ByteArray b = std::get<1>(itr);
            auto qba = QByteArray(reinterpret_cast<const char*>(b.data()), (qsizetype)b.size());
            rs.bindValue(key.c_str(), qba);
        }
        rs.exec();
        if(model->rowCount(index) > 0)
            update_rows_positions(model, index);
    }
}

