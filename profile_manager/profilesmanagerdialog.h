#ifndef PROFILE_MANAGER_MAINDIALOG_H
#define PROFILE_MANAGER_MAINDIALOG_H

#include <QDialog>
#include <QDebug>
#include <memory>
#include "../../global/profile_conf.hpp"
#include <QStatusBar>
#include "include/profilespage.h"
#include <QSqlDatabase>
#include "../global/certuser/certuser.h"

#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QCloseEvent>
#include <QProcess>

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
        //QStatusBar * m_status_bar;
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
        ProfilesPage* m_profile_page;

        QSqlDatabase m_database;

        void read_data();
        void createTrayActions();
        void createTrayIcon();
        void createDynamicMenu();



    private slots:
        void onOkClicked();

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
    };
}


#endif // PROFILE_MANAGER_MAINDIALOG_H
