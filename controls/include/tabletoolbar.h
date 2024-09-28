#ifndef TABLETOOLBAR_H
#define TABLETOOLBAR_H

#ifndef IS_OS_ANDROID
#include "../controls_global.h"
#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QToolButton>

namespace Ui {
class TableToolBar;
}

enum table_toolbar_buttons{
    table_add_item,
    table_add_group,
    table_delete_item,
    table_edit_item,
    table_move_to_item,
    table_move_up_item,
    table_move_down_item,
    table_options,
    table_btnINVALID = -1
};
NLOHMANN_JSON_SERIALIZE_ENUM(table_toolbar_buttons, {
    {table_btnINVALID, nullptr}    ,
    {table_add_item, "table_add_item"}  ,
    {table_add_group, "table_add_group"}  ,
    {table_delete_item, "table_delete_item"}  ,
    {table_edit_item, "table_edit_item"}  ,
    {table_move_to_item, "table_move_to_item"}  ,
    {table_move_up_item, "table_move_up_item"}  ,
    {table_move_down_item, "table_move_down_item"}  ,
    {table_options, "table_options"}  ,
})

namespace arcirk::widgets {
    class CONTROLS_EXPORT TableToolBar : public QWidget
    {
        Q_OBJECT

    public:
        explicit TableToolBar(QWidget *parent = nullptr);
        ~TableToolBar();

        void setButtonEnabled(const QString& name, bool value);
        void setButtonVisible(const QString& name, bool value);
        void setSeparatorVisible(int index, bool value);
        QToolButton *button(const QString& name);
        void addButton(const QString& name, const QIcon& ico, bool checkable = false, int position = -1, const QString& toolTip = {});
        void insertSeparator(int position = -1);

        void setHierarchyState(bool state);

        QMenu* context_menu();

    private:
        Ui::TableToolBar *ui;
        QMap<QString,QToolButton*> m_buttons;
        QMenu* m_context_menu;
        QMap<table_toolbar_buttons, QAction*> m_map_commands{};
        bool m_hierarchical_list;

    public slots:
        void onTableEnabled(bool value);

    private slots:
        void onButtonClicked();

    signals:
        void itemClicked(const QString& buttonName);
    };
}

#endif
#endif // TABLETOOLBAR_H
