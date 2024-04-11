//
// Created by admin on 27.01.2024.
//
#include "../include/profilespage.h"
#include "../ui/ui_profilespage.h"
#include <QFile>
#include <QFileDialog>
#include <QToolButton>
#include "../include/itable.hpp"
#include "../include/setimagedialog.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include "../include/sql/datautils.h"
#include "../global/sql/query_builder.hpp"
#include <QSqlError>
#include <QSqlQuery>

using namespace arcirk::profile_manager;
using namespace arcirk::database;

ProfilesPage::ProfilesPage(std::shared_ptr<ProfilesConf>& conf, QSqlDatabase& db, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProfilesPage),
    m_conf(conf),
    m_db(db)
{
    ui->setupUi(this);

    m_tool_bar = new widgets::TableToolBar(this);
    m_tool_bar->addButton("selectImage", QIcon("://res/img/kontur/file-format-png-24-regular.svg"));
    m_tool_bar->button("selectImage")->setToolTip("Установить картинку строки");
    m_table = new widgets::TableWidget(this);
    m_table->setTableToolBar(m_tool_bar);

    ui->verticalLayoutPP->addWidget(m_tool_bar);
    ui->verticalLayoutPP->addWidget(m_table);

    ui->txtFirefoxPath->setText(m_conf->firefox_path().c_str());

    connect(ui->btnSelectPathFirefox, &QToolButton::clicked, this, &ProfilesPage::onSelectFirefox);
    connect(m_table, &TableWidget::toolBarItemClicked, this, &ProfilesPage::onToolbarItemClick);
    connect(m_table, &TableWidget::tableCurrentChanged, this, &ProfilesPage::onTableCurrentChanged);
    connect(m_table, &TableWidget::rowChanged, this, &ProfilesPage::onRowChanged);
    connect(m_table, &TableWidget::removeTableItem, this, &ProfilesPage::onRemoveTableItem);
    connect(m_table, &TableWidget::rowMove, this, &ProfilesPage::onRowMove);

    init();
}

ProfilesPage::~ProfilesPage() {
    delete ui;
}

void ProfilesPage::onSelectFirefox() {

    auto result = QFileDialog::getOpenFileName(this, "Выбрать файл", "", "Файлы приложений (*.exe)");
    if(!result.isEmpty()){
        m_conf->set_firefox_path(result.toStdString());
        ui->txtFirefoxPath->setText(result);
    }
}

void ProfilesPage::init() {

    auto model = new ITable<profile_item>(this);
    model->get_conf()->set_column_role("url", editor_inner_role::editorMultiText);
    model->get_conf()->set_column_not_public("icon", true);
    model->get_conf()->set_column_not_public("pos", true);
    model->get_conf()->set_column_select_type("profile", false);
    model->get_conf()->set_column_select_type("name", false);
    model->get_conf()->set_column_select_type("url", false);
    auto aliases = AliasesMap({
          qMakePair("profile", "Профиль"),
          qMakePair("name", "Наименование"),
          qMakePair("url", "Адрес")
    });
    model->set_columns_aliases(aliases);
    model->reorder_columns(QList<QString>{"name", "profile", "url"});
    model->display_icons(true);

    m_table->setModel(model);
    m_table->set_inners_dialogs(true);

    load_model();

}

void ProfilesPage::onToolbarItemClick(const QString &buttonName) {
    //std::cout << buttonName.toStdString() << std::endl;
    auto index = m_table->currentIndex();
    if(!index.isValid())
        return;
    auto model = (ITable<profile_item>*)m_table->model();
    auto object = model->object(index);
    if(buttonName == "selectImage"){
        auto dlg = SetImageDialog(this);
        dlg.setUrl(object.url.c_str());
        if(dlg.exec()){
            auto qba = dlg.favicon();
            set_row_icon(model->index(index.row(), 0), qba, object);
        }
        update_database(object);
    }
}

void ProfilesPage::onTableCurrentChanged(const QModelIndex &current, const QModelIndex &previous) {
    auto btn = m_tool_bar->button("selectImage");
    if(btn)
        btn->setEnabled(current.isValid());
}

void ProfilesPage::onRowChanged(int row) {

    auto model = (ITable<profile_item>*)m_table->model();
    auto index = model->index(row, 0);
    if(index.isValid()){
        auto object = model->object(index);
        auto item = arcirk::widgets::item_data(object.icon);

        if(item.data()->subtype != subtypeByte){
            auto qba = get_favicon(object.url.c_str());
            set_row_icon(index, qba, object);
        }
        update_database(object);
        emit reset();
    }
}

QByteArray ProfilesPage::get_favicon(const QString &url) {

    if(url.isEmpty())
        return {};

    QEventLoop loop;
    int httpStatus = 200;
    QByteArray httpData;
    QNetworkAccessManager httpService;

    auto finished = [&loop, &httpStatus, &httpData](QNetworkReply* reply) -> void
    {
        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if(status_code.isValid()){
            httpStatus = status_code.toInt();
            if(httpStatus != 200){
                qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "Error: " << httpStatus << " " + reply->errorString() ;
            }else
            {
                httpData = reply->readAll();
            }
        }
        loop.quit();
    };

    loop.connect(&httpService, &QNetworkAccessManager::finished, finished);
    QUrl domain(url);
    QUrl url_("https://www.google.com/s2/favicons?domain=" + domain.host());
    QNetworkRequest request(url_);
    httpService.get(request);
    loop.exec();

    if(httpStatus != 200){
        return {};
    }
    return httpData;
}

void ProfilesPage::set_firefox_path(const QString &path) {
    m_conf->set_firefox_path(path.toStdString());
    ui->txtFirefoxPath->setText(path);
}

void ProfilesPage::set_firefox_profiles(const ByteArray &ba) {
    auto model = (ITable<profile_item>*)m_table->model();
    model->get_conf()->columns()[model->column_index("profile")].selection_list = ba;
    model->get_conf()->columns()[model->column_index("profile")].default_type = editor_inner_role::editorText;
}

void ProfilesPage::update_database(const arcirk::database::profile_item &object) {
    if(!m_db.isOpen())
        return;
    using namespace arcirk::database::builder;

    auto query = builder::query_builder();
    auto rs = QSqlQuery(m_db);
    std::vector<std::tuple<std::string, arcirk::ByteArray>> blobs;
    rs.prepare(query.row_count().from(enum_synonym(tbHttpAddresses)).where(json{{"ref", object.ref}}, blobs).prepare().c_str());
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
        query_string = query.update(enum_synonym(tbHttpAddresses), blobs).where(json{{"ref", object.ref}}, blobs).prepare();
    }else{
        query_string = query.insert(enum_synonym(tbHttpAddresses), blobs).prepare();
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

void ProfilesPage::load_model() {

    if(!m_db.isOpen())
        return;
    using namespace arcirk::database::builder;

    auto query = builder::query_builder();
    auto rs = QSqlQuery(query.select().from(enum_synonym(tbHttpAddresses)).order_by(json{"pos"}).prepare().c_str(), m_db);
    rs.exec();
    auto model = (ITable<profile_item>*)m_table->model();
    while (rs.next()){
        auto item = arcirk::database::profile_item();
        auto item_json = pre::json::to_json(item);
        QSqlRecord row = rs.record();

        for (int i = 0; i < row.count(); ++i) {
            std::string column_name = row.fieldName(i).toStdString();
            if(item_json.find(column_name) == item_json.end())
                continue;
            QVariant val = row.field(i).value();
            if(val.userType() == QMetaType::QString && item_json[column_name].is_string())
                item_json[column_name] = val.toString().toStdString();
            else if(val.userType() == QMetaType::Double && item_json[column_name].is_number())
                item_json[column_name] = val.toDouble();
            else if(val.userType() == QMetaType::Int && item_json[column_name].is_number())
                item_json[column_name] = val.toInt();
            else if(val.userType() == QMetaType::Int && item_json[column_name].is_boolean())
                item_json[column_name] = val.toInt() > 0;
            else if(val.userType() == QMetaType::LongLong && item_json[column_name].is_number())
                item_json[column_name] = val.toLongLong();
            else if(val.userType() == QMetaType::ULongLong && item_json[column_name].is_number())
                item_json[column_name] = val.toULongLong();
            else if(val.userType() == QMetaType::QByteArray && item_json[column_name].is_array())
                item_json[column_name] = qbyte_to_byte(val.toByteArray());
        }
        item = pre::json::from_json<profile_item>(item_json);
        auto index = model->add_struct(item);
        if(index.isValid()){
            auto item_d = arcirk::widgets::item_data(item.icon);
            if(item_d.data()->subtype == subtypeByte){
                auto qba = QByteArray(reinterpret_cast<const char*>(item_d.data()->data.data()), item_d.data()->data.size());
                QPixmap p;
                p.loadFromData(qba);
                auto icon = QIcon(p);
                if(!icon.isNull())
                    model->setData(index, QIcon(p), Qt::DecorationRole);
            }
        }
    }
}

void ProfilesPage::onRemoveTableItem(const json &object) {
    if(!m_db.isOpen())
        return;
    using namespace arcirk::database::builder;

    auto query = builder::query_builder();
    std::vector<std::tuple<std::string, arcirk::BJson>> blobs;
    auto query_string = query.remove().from(enum_synonym(tbHttpAddresses)).where(json{{"ref", object["ref"]}}, blobs).prepare();
    auto rs = QSqlQuery(m_db);
    rs.prepare(query_string.c_str());
    for(auto itr : blobs){
        std::string key = std::get<0>(itr);
        arcirk::BJson b = std::get<1>(itr);
        auto qba = QByteArray(reinterpret_cast<const char*>(b.data()), (qsizetype)b.size());
        rs.bindValue(key.c_str(), qba);
    }
    rs.exec();

}

void ProfilesPage::reset_pos() {

    if(!m_db.isOpen())
        return;

    using namespace arcirk::database::builder;
    typedef std::vector<std::tuple<std::string, arcirk::BJson>> blobs;
    std::vector<std::tuple<std::string, blobs>> requests;

    auto model = (ITable<profile_item>*)m_table->model();

    for (int i = 0; i < model->rowCount() ; ++i) {
        model->setData(model->index(i, model->column_index("ref")), i);
        auto var_blobs = blobs();
        auto query = builder::query_builder();
        query.use(json::object({{"pos", i}}));
        auto ref = qbyte_to_byte(model->index(i, model->column_index("ref")).data(TABLE_DATA_VALUE).toByteArray());
        auto query_string = query.update(enum_synonym(tbHttpAddresses)).where(json{{"ref", ref}}, var_blobs, false).prepare();
        requests.emplace_back(query_string, var_blobs);
    }

    m_db.transaction();
    auto rs = QSqlQuery(m_db);
    for (auto itr : requests) {
        rs.clear();
        rs.prepare(std::get<0>(itr).c_str());
        for(auto val : std::get<1>(itr)){
            std::string key = std::get<0>(val);
            arcirk::BJson b = std::get<1>(val);
            auto qba = byte_to_qbyte(b);
            rs.bindValue(key.c_str(), qba);
           // rs.addBindValue(qba);
        }
        auto result = rs.exec();

        if(!result){
            std::cerr << rs.lastError().text().toStdString() << std::endl;
        }
    }
    m_db.commit();
}

void ProfilesPage::onRowMove() {
    reset_pos();
    emit reset();
}

[[maybe_unused]] QString ProfilesPage::firefox_path() const {
    return ui->txtFirefoxPath->text();
}

void ProfilesPage::set_row_icon(const QModelIndex &index, const QByteArray &qba, profile_item& object) {
    if(!qba.isEmpty()){
        auto model = (ITable<profile_item>*)m_table->model();
        auto ba = qbyte_to_byte(qba);
        object.icon = to_byte(to_binary(ba, subtypeByte));
        model->set_struct(index.row(), object);
        QPixmap p;
        p.loadFromData(qba);
        model->setData(index, QIcon(p), Qt::DecorationRole);
    }
}

QVector<arcirk::database::profile_item> ProfilesPage::profiles() const {
    auto model = (ITable<profile_item>*)m_table->model();
    return model->array();
}
