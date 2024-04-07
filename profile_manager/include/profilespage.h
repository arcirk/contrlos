//
// Created by admin on 27.01.2024.
//

#ifndef CONTROLSPROG_PROFILESPAGE_H
#define CONTROLSPROG_PROFILESPAGE_H
#include <QWidget>
#include <tablewidget.h>
#include <tabletoolbar.h>
#include <memory>
#include "../../global/profile_conf.hpp"
#include <QSqlDatabase>

QT_BEGIN_NAMESPACE
namespace Ui {
    class ProfilesPage;
}
QT_END_NAMESPACE

using namespace arcirk::widgets;

namespace arcirk::profile_manager {

    class ProfilesPage : public QWidget
    {
        Q_OBJECT

        public:
            explicit ProfilesPage(std::shared_ptr<ProfilesConf>& conf, QSqlDatabase& db, QWidget *parent = nullptr);
            ~ProfilesPage();

            void set_firefox_path(const QString& path);
            void set_firefox_profiles(const ByteArray& ba);
            QString firefox_path() const;

        private:
            Ui::ProfilesPage *ui;
            TableToolBar* m_tool_bar;
            TableWidget* m_table;
            std::shared_ptr<ProfilesConf>& m_conf;
            QSqlDatabase& m_db;

            void init();
            [[nodiscard]] static QByteArray get_favicon(const QString& url) ;
            void update_database(const arcirk::database::profile_item& object);
            void load_model();
            void reset_pos();



        private slots:
            void onSelectFirefox();
            void onToolbarItemClick(const QString& buttonName);
            void onTableCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
            void onRowChanged(int row);
            void onRemoveTableItem(const json& object);
            void onRowMove();
    };
}

#endif //CONTROLSPROG_PROFILESPAGE_H
