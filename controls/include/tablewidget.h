//
// Created by admin on 13.01.2024.
//

#ifndef CONTROLSPROG_TABLEWIDGET_H
#define CONTROLSPROG_TABLEWIDGET_H

#include "../controls_global.h"
#include "QTableView"
#include "tabletoolbar.h"

namespace arcirk::widgets{

    enum table_toolbar_buttons{
        table_add_item,
        table_delete_item,
        table_edit_item,
        table_move_down_item,
        table_move_up_item,
        table_btnINVALID = -1
    };
    NLOHMANN_JSON_SERIALIZE_ENUM(table_toolbar_buttons, {
        {table_btnINVALID, nullptr}    ,
        {table_add_item, "table_add_item"}  ,
        {table_delete_item, "table_delete_item"}  ,
        {table_edit_item, "table_edit_item"}  ,
        {table_move_down_item, "table_move_down_item"}  ,
        {table_move_up_item, "table_move_up_item"}  ,
    })

    class CONTROLS_EXPORT TableWidget  : public QTableView{
    Q_OBJECT
    public:
        explicit TableWidget(QWidget *parent = nullptr, const QString& typeName = "TableWidget");
        virtual ~TableWidget(){};

        void setModel(QAbstractItemModel* model) override;

        void setTableToolBar(TableToolBar* value);
        TableToolBar* toolBar();
        void set_inners_dialogs(bool value);
        void allow_default_command(bool value);

        void addRow();
        void editRow();
        void moveUp();
        void moveDown();
        void removeRow();
        void clear();

        void header_visible(Qt::Orientation orientation, bool value);
        void close_editor();

    private:
        bool m_inners_dialogs;
        bool m_allow_def_commands;
        TableToolBar* m_toolBar;
        void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

        void editRowInDialog(const QModelIndex& index);

    signals:
        void itemClicked(const QModelIndex& index);
        void itemDoubleClicked(const QModelIndex& index);
        void toolBarItemClicked(const QString& buttonName);
        void removeTableItem(const json& object);
        void tableCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
        void rowChanged(int row);
        void rowMove();

    private slots:
        void onItemClicked(const QModelIndex& index);
        void onItemDoubleClicked(const QModelIndex& index);
        //void onSourceModelChanged();
        void onToolBarItemClicked(const QString& buttonName);

        void onSelectValue(int row, int col, const table_command& type);

    };

}

#endif //CONTROLSPROG_TABLEWIDGET_H
