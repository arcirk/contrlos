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
    connect(m_profile_page, &ProfilesPage::reset, this, &ProfilesManagerDialog::onResetHttpList);

    m_mstsc_page = new MstscPage(this);
    ui->verticalLayoutRDP->addWidget(m_mstsc_page);

    read_data();

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ProfilesManagerDialog::onOkClicked);

    createTrayActions();
    createTrayIcon();
    createDynamicMenu();
    trayIcon->show();

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

    return accept();
}

void ProfilesManagerDialog::createTrayActions() {

    qDebug() << __FUNCTION__;
    quitAction = new QAction(tr("&Выйти"), this);
    connect(quitAction, &QAction::triggered, this, &ProfilesManagerDialog::onAppExit);
    showAction = new QAction(tr("&Открыть менеджер профилей"), this);
    showAction->setIcon(QIcon(":/img/mpl.png"));
    connect(showAction, &QAction::triggered, this, &ProfilesManagerDialog::onWindowShow);
    checkIpAction = new QAction(tr("&Проверить IP адрес"), this);
    connect(checkIpAction, &QAction::triggered, this, &ProfilesManagerDialog::onCheckIP);
    openFirefox = new QAction(tr("&Открыть Firefox с выбором профиля"), this);
    connect(openFirefox, &QAction::triggered, this, &ProfilesManagerDialog::openMozillaFirefox);
    installCertAction = new QAction(tr("&Установить сертификат"), this);
    connect(installCertAction, &QAction::triggered, this, &ProfilesManagerDialog::onInstallCertificate);

    trayIconMenu = new QMenu(this);
}

void ProfilesManagerDialog::onTrayTriggered() {

}

void ProfilesManagerDialog::onTrayMstscConnectToSessionTriggered() {

}

void ProfilesManagerDialog::onAppExit() {
    qDebug() << __FUNCTION__;
    QApplication::exit();
}

void ProfilesManagerDialog::onWindowShow() {
    qDebug() << __FUNCTION__;
    setVisible(true);
}

void ProfilesManagerDialog::trayMessageClicked() {

}

void ProfilesManagerDialog::trayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    qDebug() << __FUNCTION__ << reason;
    if(reason == QSystemTrayIcon::DoubleClick){
        setVisible(true);
    }else if(reason == QSystemTrayIcon::Context){
        qDebug() << "QSystemTrayIcon::Context";
    }

}

void ProfilesManagerDialog::trayShowMessage(const QString &msg, int isError) {

}

void ProfilesManagerDialog::onInstallCertificate() {

}

void ProfilesManagerDialog::openMozillaFirefox() {

}

void ProfilesManagerDialog::onCheckIP() {

}

void ProfilesManagerDialog::createTrayIcon() {

    qDebug() << __FUNCTION__;
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);

    QIcon icon = QIcon("://res/img/briefcase.ico");
    trayIcon->setIcon(icon);
    setWindowIcon(icon);

    trayIcon->setToolTip("Менеджер профилей");

    connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &ProfilesManagerDialog::trayMessageClicked);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &ProfilesManagerDialog::trayIconActivated);

}

void ProfilesManagerDialog::createDynamicMenu() {

    qDebug() << __FUNCTION__;
    trayIconMenu->clear();
    trayIconMenu->addAction(showAction);
    trayIconMenu->addAction(checkIpAction);
    trayIconMenu->addAction(openFirefox);
    trayIconMenu->addAction(installCertAction);

//    auto cache = current_user->cache();
//    auto mstsc_param = cache.value("mstsc_param", json::object());
//    auto mpl_ = arcirk::internal_structure<arcirk::client::mpl_options>("mpl_options", cache);
//    //auto use_firefox = mpl_.use_firefox;
//    auto firefox_path = mpl_.firefox_path;
//    auto is_connect_to_users = mstsc_param.value("enable_mstsc_users_sess", false);
//
//    if(is_connect_to_users){
//        trayIconMenu->addSeparator();
//        auto action = new QAction("Подключиться к сеансу пользователя", this);
//        action->setIcon(QIcon(":/img/mstscUsers.png"));
//        trayIconMenu->addAction(action);
//        connect(action, &QAction::triggered, this, &DialogMain::onTrayMstscConnectToSessionTriggered);
//    }
//
//    if(!mstsc_param.empty()){
//        auto is_enable = mstsc_param.value("enable_mstsc", false);
//        auto detailed_records = mstsc_param.value("detailed_records", json::array());
//        if(is_enable && detailed_records.size() > 0){
//            trayIconMenu->addSeparator();
//            for(auto itr = detailed_records.begin(); itr != detailed_records.end(); ++itr){
//                auto object = *itr;
//                auto mstsc = arcirk::secure_serialization<arcirk::client::mstsc_options>(object, __FUNCTION__);
//                QString name = mstsc.name.c_str();
//                auto action = new QAction(name, this);
//                action->setProperty("data", object.dump().c_str());
//                action->setProperty("type", "mstsc");
//                action->setIcon(QIcon(":/img/mstsc.png"));
//                trayIconMenu->addAction(action);
//                connect(action, &QAction::triggered, this, &DialogMain::onTrayTriggered);
//            }
//        }
//    }
//

    auto mpl_list = m_profile_page->profiles();

    if(mpl_list.size() > 0){
        trayIconMenu->addSeparator();
        for (auto itr = mpl_list.begin(); itr != mpl_list.end(); ++itr) {

            QString name = itr->name.c_str();
            auto action = new QAction(name, this);
            action->setProperty("url", itr->url.c_str());
            action->setProperty("type", "link");
            if(!itr->icon.empty()){
                auto item_d = arcirk::widgets::item_data(itr->icon);
                if(item_d.data()->subtype == subtypeByte){
                    auto qba = QByteArray(reinterpret_cast<const char*>(item_d.data()->data.data()), item_d.data()->data.size());
                    QPixmap p;
                    p.loadFromData(qba);
                    auto icon = QIcon(p);
                    action->setIcon(QIcon(p));
                }
            }
            trayIconMenu->addAction(action);
            connect(action, &QAction::triggered, this, &ProfilesManagerDialog::onTrayTriggered);
        }
    }
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
}

void ProfilesManagerDialog::accept() {
    qDebug() << __FUNCTION__;

  //write_cache();

    //QDialog::accept();
    QDialog::hide();
}

void ProfilesManagerDialog::closeEvent(QCloseEvent *event) {
#ifdef Q_OS_MACOS
    if (!event->spontaneous() || !isVisible()) {
        return;
    }
#endif

    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}

void ProfilesManagerDialog::onResetHttpList() {
    createDynamicMenu();
}

