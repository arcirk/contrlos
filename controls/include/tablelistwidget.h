//
// Created by admin on 19.01.2024.
//

#ifndef CONTROLSPROG_TABLELISTWIDGET_H
#define CONTROLSPROG_TABLELISTWIDGET_H

#include <QWidget>
#include "../controls_global.h"
#include "tabletoolbar.h"
#include "tablewidget.h"
#include "tablemodel.h"

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::widgets), table_list_item,
    (bool, select)
    (BJson, value)
)

namespace Ui {
    class TableListWidget;
}

namespace arcirk::widgets {

    class CONTROLS_EXPORT TableListWidget : public QWidget
    {
        Q_OBJECT

        public:
            explicit TableListWidget(QWidget *parent = nullptr, editor_inner_role role = editorNullType);
            ~TableListWidget();

            [[nodiscard]] json to_array() const;
            void set_array(const json& data);

            void close_editor();

            void set_toolbar_visible(bool value);
            void set_checked(bool value);

        private:
            Ui::TableListWidget *ui;
            TableToolBar* m_tool_bar;
            TableWidget* m_table_widget;
            TableModel* m_model;
            editor_inner_role m_role;

            [[nodiscard]] json default_value() const;

    private slots:
        void onToolbarButtonClick(const QString& button_name);

    };
} // arcirk::widgets

#endif //CONTROLSPROG_TABLELISTWIDGET_H
