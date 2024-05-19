//
// Created by admin on 19.05.2024.
//

#ifndef CONTROLSPROG_CERTSPAGE_H
#define CONTROLSPROG_CERTSPAGE_H

#include <QWidget>
#include <tablewidget.h>
#include <tabletoolbar.h>
#include <memory>
#include "../../global/profile_conf.hpp"
#include <QSqlDatabase>

using namespace arcirk::widgets;

QT_BEGIN_NAMESPACE
namespace Ui { class CertsPage; }
QT_END_NAMESPACE

namespace arcirk::profile_manager {

    class CertsPage : public QWidget {
        Q_OBJECT

        public:
            explicit CertsPage(std::shared_ptr<ProfilesConf> &conf, QSqlDatabase &db, QWidget *parent = nullptr);

            ~CertsPage() override;

        private:
            Ui::CertsPage *ui;
            TableToolBar* m_tool_bar;
            TableWidget* m_table;
            std::shared_ptr<ProfilesConf>& m_conf;
            QSqlDatabase& m_db;
    };

}
#endif //CONTROLSPROG_CERTSPAGE_H
