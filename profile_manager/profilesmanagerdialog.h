#ifndef PROFILE_MANAGER_MAINDIALOG_H
#define PROFILE_MANAGER_MAINDIALOG_H

#include <QDialog>
#include <QDebug>
#include <memory>
#include "../../global/profile_conf.hpp"
#include <QStatusBar>
#include "include/profilespage.h"
#include "include/mstscpage.h"
#include <QSqlDatabase>
#include "../global/certuser/certuser.h"
#include "include/certspage.h"

#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QCloseEvent>
#include <QProcess>

#define BANK_CLIENT_FILE "sslgate.url"
#define BANK_CLIENT_USB_KEY "BankUsbKey"

QT_BEGIN_NAMESPACE
namespace Ui {
class ProfilesManagerDialog;
}
QT_END_NAMESPACE

namespace arcirk::profile_manager{
    class ProfilesManagerDialog : public QDialog
    {
    Q_OBJECT

    public:
        explicit ProfilesManagerDialog(QWidget *parent = nullptr);
        ~ProfilesManagerDialog() override;

        void accept() override;

    protected:
        void closeEvent(QCloseEvent *event) override;

    private:
        Ui::ProfilesManagerDialog *ui;
        QProcess * mozillaApp;
        std::shared_ptr<ProfilesConf> m_conf;
        std::shared_ptr<CertUser> m_current_user;

        //tray menu
        QSystemTrayIcon *trayIcon;
        QMenu           *trayIconMenu;
        QAction         *quitAction;
        QAction         *showAction;
        QAction         *checkIpAction;
        QAction         *openFirefox;
        QAction         *installCertAction;

        //pages
        ProfilesPage*   m_profile_page;
        MstscPage*      m_mstsc_page;
        CertsPage*      m_cert_page;

        QSqlDatabase    m_database;

        void read_data();
        void createTrayActions();
        void createTrayIcon();
        void createDynamicMenu();
        void createMenuRecursive(QMenu * parent, const json& array);
        [[nodiscard]] QString cache_mstsc_directory() const;
        void run_mstsc_link(const QString& fileName);
        void run_mozilla_firefox(const QString& defPage, const QString& profName);

    private slots:
        void onOkClicked();
        void onCheckBoxClicked(bool checked);

        //tray
        void onTrayTriggered();
        void onTrayMstscConnectToSessionTriggered();
        void onAppExit();
        void onWindowShow();
        void trayMessageClicked();
        void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
        void trayShowMessage(const QString& msg, int isError = false);
        void onInstallCertificate();
        void openMozillaFirefox();
        void onCheckIP();

        //http links page
        void onResetHttpList();

        //mstscpage
        void onResetMstscPage();
    };
}


#endif // PROFILE_MANAGER_MAINDIALOG_H
