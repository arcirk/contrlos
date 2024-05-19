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
#include <QCheckBox>
#include <QFileDialog>
#include "../global/sh/commandline.h"

using namespace arcirk::profile_manager;
using namespace arcirk::filesystem;
using namespace arcirk::database;

ProfilesManagerDialog::ProfilesManagerDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProfilesManagerDialog)
{
    ui->setupUi(this);
    setWindowTitle("Менеджер профилей");
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
    m_conf = std::make_shared<ProfilesConf>(m_current_user);
    m_current_user->read_user_info(this);
    m_profile_page = new ProfilesPage(m_conf, m_database, this);
    ui->verticalLayoutPF->addWidget(m_profile_page);
    connect(m_profile_page, &ProfilesPage::reset, this, &ProfilesManagerDialog::onResetHttpList);

    m_mstsc_page = new MstscPage(m_database, this);
    ui->verticalLayoutRDP->addWidget(m_mstsc_page);
    connect(m_mstsc_page, &MstscPage::reset, this, &ProfilesManagerDialog::onResetMstscPage);

    m_cert_page = new CertsPage(m_conf, m_database, this);
    ui->verticalLayoutCerts->addWidget(m_cert_page);

    read_data();

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ProfilesManagerDialog::onOkClicked);

    createTrayActions();
    createTrayIcon();
    createDynamicMenu();
    trayIcon->show();

    connect(ui->checkBoxEnableRdp, &QCheckBox::clicked, this, &ProfilesManagerDialog::onCheckBoxClicked);
    connect(ui->checkBoxEnableRdpUsers, &QCheckBox::clicked, this, &ProfilesManagerDialog::onCheckBoxClicked);

    mozillaApp = new QProcess(this);
}

ProfilesManagerDialog::~ProfilesManagerDialog()
{
//    mozillaApp->close();
//    delete mozillaApp;
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

    ui->checkBoxEnableRdpUsers->setChecked(m_conf->conf().allow_mstsc_users);
    ui->checkBoxEnableRdp->setChecked(m_conf->conf().allow_mstsc);

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

    //qDebug() << __FUNCTION__;
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

    auto action = qobject_cast<QAction*>(sender());
    if(!action)
        return;

    auto type = action->property("type").toString();

    if (type == "mstsc"){
        auto file = QFile(cache_mstsc_directory() + "/" + action->text() + ".rdp");
        if(!file.exists()){
            auto uuid = action->property("uuid").toUuid();
            m_mstsc_page->openMstscEditDialog(uuid);
        }else{
            run_mstsc_link(file.fileName());
        }
    }else if(type == "link"){
        run_mozilla_firefox(action->property("url").toString(), action->property("profile").toString());
    }
}

void ProfilesManagerDialog::onTrayMstscConnectToSessionTriggered() {

}

void ProfilesManagerDialog::onAppExit() {
    //qDebug() << __FUNCTION__;
    QApplication::exit();
}

void ProfilesManagerDialog::onWindowShow() {
    //qDebug() << __FUNCTION__;
    setVisible(true);
}

void ProfilesManagerDialog::trayMessageClicked() {

}

void ProfilesManagerDialog::trayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    //qDebug() << __FUNCTION__ << reason;
    if(reason == QSystemTrayIcon::DoubleClick){
        setVisible(true);
    }else if(reason == QSystemTrayIcon::Context){
        //qDebug() << "QSystemTrayIcon::Context";
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

    //qDebug() << __FUNCTION__;
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);

    QIcon icon = QIcon("://res/img/briefcase.ico");
    trayIcon->setIcon(icon);
    setWindowIcon(icon);

    trayIcon->setToolTip("Менеджер профилей");

    connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &ProfilesManagerDialog::trayMessageClicked);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &ProfilesManagerDialog::trayIconActivated);

}

void ProfilesManagerDialog::createMenuRecursive(QMenu *parent, const json &array) {
    for (auto itr = array.begin(); itr != array.end(); ++itr) {
        auto object = *itr;
        if(object.is_object()){
            auto items = object.value("items", json::array());
            object.erase("items");
            auto item = pre::json::from_json<mstsc_item>(object);
            if(item.is_group){
                auto section = parent->addMenu(item.name.c_str());
                createMenuRecursive(section, items);
            }else{
                QString name = item.name.c_str();
                auto action = new QAction(name, this);
                action->setProperty("type", "mstsc");
                action->setIcon(QIcon("://res/img/mstsc.png"));
                auto _item_data = widgets::item_data(item.ref);
                QUuid uuid = QUuid::fromRfc4122(_item_data.data()->data);
                action->setProperty("uuid", qvariant_cast<QUuid>(uuid));
                parent->addAction(action);
                connect(action, &QAction::triggered, this, &ProfilesManagerDialog::onTrayTriggered);
            }
        }else{
            std::cout << type_string(object.type()) << std::endl;
        }

    }
}

void ProfilesManagerDialog::createDynamicMenu() {

    trayIconMenu->clear();
    trayIconMenu->addAction(showAction);
    trayIconMenu->addAction(checkIpAction);
    trayIconMenu->addAction(openFirefox);
    trayIconMenu->addAction(installCertAction);

    auto mstsc_tree = m_mstsc_page->get_mstsc_items();
    if(!mstsc_tree.empty()){
        std::cout << mstsc_tree.dump(4) << std::endl;
        trayIconMenu->addSeparator();
        for (auto itr = mstsc_tree.begin(); itr != mstsc_tree.end(); ++itr) {
            auto root_object = *itr;
            if(!root_object.is_object())
                break;
            auto items = root_object.value("items", json::array());
            root_object.erase("items");
            try {
                auto object = pre::json::from_json<mstsc_item>(root_object);
                if(object.is_group){
                    createMenuRecursive(trayIconMenu, items);
                }
            } catch (const std::exception &e) {
                std::cerr << e.what() <<std::endl;
                std::cout << root_object.dump(4) << std::endl;
            }
        }
    }

    auto mpl_list = m_profile_page->profiles();

    if(mpl_list.size() > 0){
        trayIconMenu->addSeparator();
        for (auto itr = mpl_list.begin(); itr != mpl_list.end(); ++itr) {

            QString name = itr->name.c_str();
            auto action = new QAction(name, this);
            action->setProperty("url", itr->url.c_str());
            action->setProperty("profile", itr->profile.c_str());
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
    //qDebug() << __FUNCTION__;
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

void ProfilesManagerDialog::onCheckBoxClicked(bool checked) {

    auto check = qobject_cast<QCheckBox*>(sender());

    if(check){
        if(check->objectName() == "checkBoxEnableRdp")
            m_conf->conf().allow_mstsc = checked;
        else if(check->objectName() == "checkBoxEnableRdpUsers")
            m_conf->conf().allow_mstsc_users = checked;

    }

}

void ProfilesManagerDialog::onResetMstscPage() {
    createDynamicMenu();
}

QString ProfilesManagerDialog::cache_mstsc_directory() const {
    auto app_data = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    app_data.append("/mstsc");
    QDir f(app_data);
    if(!f.exists())
        f.mkpath(f.path());
    return f.path();
}

void ProfilesManagerDialog::run_mstsc_link(const QString& fileName)
{

    QString command;
    QFile f(fileName);
    command.append("mstsc \"" + QDir::toNativeSeparators(f.fileName()) + "\" & exit");

    auto cmd = CommandLine(this);
    QEventLoop loop;

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

void ProfilesManagerDialog::run_mozilla_firefox(const QString& defPage, const QString& profName) {

    QString page(defPage);

    //открываем адрес указанный на флешке банка
    if(defPage == BANK_CLIENT_USB_KEY){
        foreach (const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
            if (storage.isValid()) {
                // << storage.rootPath() + QDir::separator() + BANK_CLIENT_FILE;
                QFile file = QFile(storage.rootPath() + QDir::separator() + BANK_CLIENT_FILE);
                if(file.exists()){
                    QSettings lnk(file.fileName(), QSettings::IniFormat);
                    QStringList keys = lnk.allKeys();
                    foreach(const QString& key, keys){
                        if(key.compare("InternetShortcut")){
                            if(key.endsWith("/URL")){
                                page = lnk.value(key).toString();
                                break;
                            }

                        }
                    }
                }
            }
        }
    }

    QStringList args;

    //args.append("-new-instance");
    if(!profName.isEmpty()){
        args.append("-P");
        args.append(profName);
        if(!page.isEmpty()){
            QFile file = QFile(page);
            if(file.exists())
                page = "file:///" + page;
            args.append("-URL");
            args.append(page);
        }
    }

    QString exeFile = "firefox";
#ifdef Q_OS_WINDOWS
    exeFile.append(".exe");
#endif
    QFile exe(m_profile_page->firefox_path());
    if(!exe.exists()){
        QString file = QFileDialog::getOpenFileName(this, tr("Путь к firefox"),
                                                    QDir::homePath(),
                                                    exeFile);
        if(file != ""){
            QFileInfo fi(file);
            exe.setFileName(file);
            m_profile_page->set_firefox_path(QDir::toNativeSeparators(fi.absolutePath()));
        }
    }

    if(exe.exists()){
        //qDebug() << mozillaApp->state();
        mozillaApp->terminate();
        mozillaApp->kill();
        mozillaApp->waitForFinished();
        mozillaApp->start(exe.fileName(), args);
    }

}
