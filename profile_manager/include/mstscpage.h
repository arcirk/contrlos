//
// Created by admin on 11.05.2024.
//

#ifndef CONTROLSPROG_MSTSCPAGE_H
#define CONTROLSPROG_MSTSCPAGE_H

#include <QWidget>
#include <treewidget.h>
#include <tabletoolbar.h>
#include <QSqlDatabase>

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

    private:
        Ui::MstscPage *ui;
        TableToolBar* m_tool_bar;
        TreeViewWidget * m_tree;
        QSqlDatabase& m_db;

        void init();
        void initData();
        void openItemEditDialog(bool isNew = false);

    private slots:
        void onToolBarItemClicked(const QString& buttonName);
        void onCurrentChanged(const QModelIndex &current);
        void onRemoveItem(const json& object_removed);
        void onTableItemChanged(const QModelIndex& index);

    private:
        void update_database(const QModelIndex& index);
        void update_database(const json& object);
    };
} // arcirk::profile_manager

#endif //CONTROLSPROG_MSTSCPAGE_H
