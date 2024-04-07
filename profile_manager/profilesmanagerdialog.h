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

    private:
        Ui::ProfilesManagerDialog *ui;
        //QStatusBar * m_status_bar;
        std::shared_ptr<ProfilesConf> m_conf;
        std::shared_ptr<CertUser> m_current_user;

        //pages
        ProfilesPage* m_profile_page;

        QSqlDatabase m_database;

        void read_data();

    private slots:
        void onOkClicked();
    };
}


#endif // PROFILE_MANAGER_MAINDIALOG_H
