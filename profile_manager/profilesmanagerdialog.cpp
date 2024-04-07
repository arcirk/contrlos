#include "profilesmanagerdialog.h"
#include "./ui_profilesmanagerdialog.h"
#include <QTabWidget>
#include "../global/global.hpp"
#include "../global/fs.hpp"
#include <QSqlError>
#include <QMessageBox>
#include "../include/sql/datautils.h"
#include "../global/sql/query_builder.hpp"
#include <QDialogButtonBox>

using namespace arcirk::profile_manager;
using namespace arcirk::filesystem;
using namespace arcirk::database;

ProfilesManagerDialog::ProfilesManagerDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProfilesManagerDialog)
{
    ui->setupUi(this);
    setWindowTitle("Менеджер профилей");

    m_conf = std::make_shared<ProfilesConf>();

    //m_status_bar = new QStatusBar(this);
    //ui->horizontalLayout->addWidget(m_status_bar);

    auto path = FSPath();
    path.init_app_data_dir();

    path /= "data";
    if(!path.dir_exists())
        path.mkpath();

    path /= "config.sqlite";

    m_database = QSqlDatabase::addDatabase("QSQLITE", "local_database");
    m_database.setDatabaseName(path.path());

    if(!m_database.open()){
        std::cerr << m_database.lastError().text().toStdString() << std::endl;
    }else{
        auto db_utils = DataUtils(m_database);
        auto result = db_utils.verify();
        if(result){
            db_utils.verify_default_data();
        }
    }

    m_current_user = std::make_shared<CertUser>();
    m_current_user->read_user_info(this);
    m_profile_page = new ProfilesPage(m_conf, m_database, this);
    ui->verticalLayoutPF->addWidget(m_profile_page);

    read_data();

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ProfilesManagerDialog::onOkClicked);

}

ProfilesManagerDialog::~ProfilesManagerDialog()
{
    delete ui;
}

void ProfilesManagerDialog::read_data() {

    auto query = builder::query_builder();
    query.select().from(enum_synonym(arcirk::database::tables::tbApplicationConfig));
    auto conf_arr = query.rows_to_array<arcirk::database::application_config>(m_database);
    if(conf_arr.empty())
        return;

    auto conf = conf_arr[0];
    m_conf->set_conf(conf);

    m_profile_page->set_firefox_path(conf.firefox.c_str());

    auto profiles = m_current_user->read_mozilla_profiles();
    QByteArray qba = profiles.join(",").toUtf8();
    ByteArray ba(qba.size());
    std::copy(qba.begin(), qba.end(), ba.begin());
    m_profile_page->set_firefox_profiles(ba);
}

void ProfilesManagerDialog::onOkClicked() {
    typedef std::vector<std::tuple<std::string, arcirk::BJson>> blobs;
    auto var_blobs = blobs();
    if(m_database.isOpen()) {
        auto query = builder::query_builder();
        query.use(pre::json::to_json(m_conf->conf()));
        auto query_string = query.update(enum_synonym(tbApplicationConfig), var_blobs).where(json{{"ref", m_conf->conf().ref}}, var_blobs, false).prepare();
        auto rs = QSqlQuery(m_database);
        rs.prepare(query_string.c_str());
        for(auto val : var_blobs){
            std::string key = std::get<0>(val);
            arcirk::BJson b = std::get<1>(val);
            auto qba = byte_to_qbyte(b);
            rs.bindValue(key.c_str(), qba);
        }
        auto result = rs.exec();

        if(!result){
            std::cerr << rs.lastError().text().toStdString() << std::endl;
        }
    }

    return QDialog::accept();
}
