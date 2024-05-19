//
// Created by admin on 11.05.2024.
//

#ifndef CONTROLSPROG_MSTSCPAGE_H
#define CONTROLSPROG_MSTSCPAGE_H

#include <QWidget>
#include <treewidget.h>
#include <tabletoolbar.h>
#include <QSqlDatabase>
#include <profile_conf.hpp>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MstscPage;
}
QT_END_NAMESPACE

using namespace arcirk::widgets;

namespace arcirk::profile_manager {

    class MstscPage : public QWidget
    {
        Q_OBJECT

        public:
            explicit MstscPage(QSqlDatabase& db, QWidget *parent = nullptr);

            ~MstscPage();

            [[nodiscard]] json get_mstsc_items() const;
            void openMstscEditDialog(const QUuid& uuid = {});

        private:
            Ui::MstscPage *ui;
            TableToolBar* m_tool_bar;
            TreeViewWidget * m_tree;
            QSqlDatabase& m_db;

            void init();
            void initData();
            void openItemEditDialog(bool isNew = false);

            void editMstsc(const QString& fileName);

        private slots:
            void onToolBarItemClicked(const QString& buttonName);
            void onCurrentChanged(const QModelIndex &current);
            void onRemoveItem(const json& object_removed);
            void onTableItemChanged(const QModelIndex& index);
            void onRowMove();

        private:
            void update_database(const QModelIndex& index);
            void update_database(const json& object);
            [[nodiscard]] QString cache_mstsc_directory() const;
            QString rdp_file_text();
            void update_rows_positions(TreeModel* model, const QModelIndex& parent);

        signals:
            void reset();
    };
} // arcirk::profile_manager

#endif //CONTROLSPROG_MSTSCPAGE_H
