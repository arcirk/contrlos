#ifndef TREEWIDGET_H
#define TREEWIDGET_H

#ifndef IS_OS_ANDROID

#include <QTreeView>
#include <QWidget>
#include "treemodel.h"
#include "treesortmodel.h"
#include "treeitemdelegate.h"
#include "tabletoolbar.h"

namespace arcirk::widgets {

    enum drag_drop_behavior{
        behaviorDefault = 0,
        behaviorEmit,
        behaviorText
    };

    enum toolbar_buttons{
        add_item,
        add_group,
        delete_item,
        edit_item,
        move_to_item,
        move_down_item,
        move_up_item,
        btnINVALID = -1
    };
    NLOHMANN_JSON_SERIALIZE_ENUM(toolbar_buttons, {
        {btnINVALID, nullptr}    ,
        {add_item, "add_item"}  ,
        {add_group, "add_group"}  ,
        {delete_item, "delete_item"}  ,
        {edit_item, "edit_item"}  ,
        {move_to_item, "move_to_item"}  ,
        {move_down_item, "move_down_item"}  ,
        {move_up_item, "move_up_item"}  ,
    })

    class CONTROLS_EXPORT TreeViewWidget : public QTreeView
{
    Q_OBJECT
    public:
    explicit TreeViewWidget(QWidget *parent = nullptr, const QString& typeName = "TreeViewWidget");
    ~TreeViewWidget() override= default;

    void setModel(QAbstractItemModel* model) override;

    TreeModel* get_model();
    [[nodiscard]] QModelIndex current_index() const;
    void set_current_index(const QModelIndex& index);
    [[nodiscard]] QModelIndex get_index(const QModelIndex& sort_index) const;

    void set_drag_drop_behavior(drag_drop_behavior value){m_behavior = value;};

    void insert(const json& object, const QModelIndex& parent);
    void remove(const QString& uuid);

    void set_drag_group(bool value){m_drag_groups = value;};
    void set_drag_data_column(const QString& value){m_drop_column = value;};

    void data_is_base64(bool value){data_is_base64_ = value;};
    void set_drag_exceptions(const QMap<int, QList<QVariant>>& values);
    void set_drop_exceptions(const QMap<int, QList<QVariant>>& values);


    void setEnabled(bool value);

    int content_width(int column);

    QString object_name() const {return "TreeViewWidget";};

    void enable_sort(bool value){m_not_sort = value;};

    void clear();

    TreeItemDelegate* delegate() {return (TreeItemDelegate*)this->itemDelegate();};

    void setTableToolBar(TableToolBar* value);
        TableToolBar* toolBar() const;

    void set_inners_dialogs(bool value);
    void set_only_groups_in_root(bool value){m_only_groups_in_root = value;};

    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

    void add_group_in_root_only(bool value){m_add_group_in_root_only = value;};

    void hide_default_columns();

    void close_editor();

    void openNewItemDialog();
    void openNewGroupDialog();
    void openOpenEditDialog();
    void openOpenMoveToDialog();
    void deleteItemCommand();

    void allow_def_commands(bool value) {m_allow_def_commands = value;};

    void addRow();
    void editRow();
    void moveUp();
    void moveDown();

    protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void changeEvent(QEvent *event) override;

    private:
    QPoint startPos;
    drag_drop_behavior m_behavior;
    bool m_drag_groups;
    QString m_drop_column;
    bool data_is_base64_;
    QMap<int, QList<QVariant>> m_drag_exceptions;
    QMap<int, QList<QVariant>> m_drop_exceptions;
    bool m_hierarchy_list;
    TreeSortModel* m_sort_model;
    bool m_not_sort;
        TableToolBar* m_toolBar;
    bool m_inners_dialogs;
    bool m_only_groups_in_root; //
    bool m_add_group_in_root_only;
    bool m_allow_def_commands;
    //
    void set_hierarchy_list(bool value);
    void performDrag();
    int text_width(int column, const QModelIndex& parent, const int& result = 0);

    signals:
    void doDropEvent(const QModelIndex& index, const QString& sender);
    void doDropEventJson(const json& data, const QString& sender);
    void itemClicked(const QModelIndex& index, const QString& item_name);
    void itemDoubleClicked(const QModelIndex& index, const QString& item_name);
    void fetch(const QModelIndex& parent);
    void toolBarItemClicked(const QString& buttonName);
    void treeItemClicked(const QModelIndex& index);

    void addTreeItem(const QModelIndex& index, const json& data);
    void editTreeItem(const QModelIndex& index, const json& data);
    void deleteTreeItem(const json& data);

    private slots:
    void onItemClicked(const QModelIndex& index);
    void onItemDoubleClicked(const QModelIndex& index);
    void onSourceModelChanged();
    void onTreeFetch(const QModelIndex& parent);
    void onToolBarItemClicked(const QString& buttonName);
};
}

#endif
#endif // TREEWIDGET_H
