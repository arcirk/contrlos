//
// Created by admin on 13.01.2024.
//

#ifndef CONTROLSPROG_TABLEWIDGET_H
#define CONTROLSPROG_TABLEWIDGET_H

#include "../controls_global.h"
#include "QTableView"
#include "tabletoolbar.h"
#include <QMenu>
#include "tablemodel.h"

namespace arcirk::widgets{

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
        void set_standard_context_menu(bool value);

        void addRow();
        void editRow();
        void moveUp();
        void moveDown();
        void removeRow();
        void clear();

        TableModel* get_model() const;

        void header_visible(Qt::Orientation orientation, bool value);
        void close_editor();
        void hide_default_columns();
        void hide_not_ordered_columns();

        void open_table_options_dialog();

    private:
        bool m_inners_dialogs;
        bool m_allow_def_commands;
        TableToolBar* m_toolBar;
        bool m_standard_context_menu;
        QMenu* m_standard_menu;

        void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

        void editRowInDialog(const QModelIndex& index);
//    protected:
//        void mouseDoubleClickEvent(QMouseEvent * e) override;

    signals:
        void itemClicked(const QModelIndex& index);
        void itemDoubleClicked(const QModelIndex& index);
        void toolBarItemClicked(const QString& buttonName);
        void removeTableItem(const json& object);
        void tableCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
        void rowChanged(int row);
        void rowMove();
        //void mouseDoubleClickEvent(const QModelIndex& index);

    private slots:
        void onItemClicked(const QModelIndex& index);
        void onItemDoubleClicked(const QModelIndex& index);
        void onMouseRightItemClick(const QModelIndex& index);
        void onToolBarItemClicked(const QString& buttonName);
        void slotCustomMenuRequested(QPoint pos);
        void onSelectValue(int row, int col, const table_command& type);
        //void onMouseDoubleClickEvent(const QModelIndex& index);

    };

}

#endif //CONTROLSPROG_TABLEWIDGET_H
