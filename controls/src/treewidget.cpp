#ifndef IS_OS_ANDROID
#include "../include/treewidget.h"
//#include "../sort/treesortmodel.h"
#include <QWidget>
#include <QDragEnterEvent>
//#include <QDropEvent>
//#include <QTapAndHoldGesture>
#include <QEvent>
//#include <QGestureEvent>
#include <QDrag>
#include <QApplication>
#include <QMimeData>
#include "../include/tablerowdialog.h"
#include <QMessageBox>
//#include "../ui/selectitemdialog.h"

using namespace arcirk::widgets;

TreeViewWidget::TreeViewWidget(QWidget *parent, const QString& typeName)
        : QTreeView(parent)
{

    this->setDragDropMode(QAbstractItemView::InternalMove);
    this->setSelectionMode(QAbstractItemView::SingleSelection);
    this->setDragEnabled(true);
    this->setAcceptDrops(true);
    this->setDropIndicatorShown(true);

    m_sort_model = new TreeSortModel(this);
    connect(m_sort_model, &TreeSortModel::modelChanged, this, &TreeViewWidget::onSourceModelChanged);

    m_behavior = behaviorDefault;
    m_drag_groups = false;
    data_is_base64_ = false;
    m_hierarchy_list = true;
    m_not_sort = false;
    m_toolBar = nullptr;
    m_inners_dialogs = false;
    m_only_groups_in_root = false;
    //m_add_group_in_root_only = false;
    m_allow_def_commands = true;

    setProperty("typeName", typeName);

    connect(this, &QAbstractItemView::clicked, this, &TreeViewWidget::onItemClicked);
    connect(this, &QAbstractItemView::doubleClicked, this, &TreeViewWidget::onItemDoubleClicked);

    auto delegate = new TreeItemDelegate(false, this);
    setItemDelegate(delegate);
    setEditTriggers(QAbstractItemView::AllEditTriggers);

}

TreeModel* TreeViewWidget::get_model()
{
    if(!this->model())
        return nullptr;
    if(this->model()->property("typeName").toString() == "TreeSortModel"){
        auto m_model = (TreeSortModel*)this->model();
        return qobject_cast<TreeModel*>(m_model->sourceModel());
    }else{
        return qobject_cast<TreeModel*>(this->model());
    }
}

QModelIndex TreeViewWidget::current_index() const
{
    auto index = currentIndex();
    if(!index.isValid())
        return {};

    if(this->model()->property("typeName").toString() == "TreeSortModel"){
        auto sort_model = (TreeSortModel*)this->model();
        return sort_model->mapToSource(index);
    }else
        return index;
}

void TreeViewWidget::set_current_index(const QModelIndex &index)
{
    if(!index.isValid())
        return;

    if(this->model()->property("typeName").toString() == "TreeSortModel"){
        auto sort_model = (TreeSortModel*)this->model();
        auto sort_index = sort_model->mapFromSource(index);
        setCurrentIndex(sort_index);
    }else
        setCurrentIndex(index);
}

QModelIndex TreeViewWidget::get_index(const QModelIndex &sort_index) const
{
    if(!sort_index.isValid())
        return {};
    if(this->model()->property("typeName").toString() == "TreeSortModel"){
        auto sort_model = (TreeSortModel*)this->model();
        return sort_model->mapToSource(sort_index);
    }else
        return sort_index;
}

void TreeViewWidget::insert(const json &object, const QModelIndex &parent)
{
    auto model = get_model();
    if(!model)
        return;
    auto index = model->find(QUuid::fromString(object.value("ref", "").c_str()), QModelIndex());
    if(!index.isValid())
        model->add(object, parent);
}

void TreeViewWidget::remove(const QString &uuid)
{
    auto model = get_model();
    if(!model)
        return;
    auto index = model->find(QUuid::fromString(uuid), QModelIndex());
    if(index.isValid()){
        model->remove(index);
    }
}

void TreeViewWidget::set_drag_exceptions(const QMap<int, QList<QVariant>>& values)
{
    m_drag_exceptions = values;
}

void TreeViewWidget::set_drop_exceptions(const QMap<int, QList<QVariant>>& values)
{
    m_drop_exceptions = values;
}

void TreeViewWidget::set_hierarchy_list(bool value)
{
    m_hierarchy_list = value;

    if(m_hierarchy_list){
        this->resetIndentation();
    }else
        this->setIndentation(0);

}

void TreeViewWidget::setModel(QAbstractItemModel *model)
{
    if(!model){
        return QTreeView::setModel(model);
    }
    if(model->property("typeName").toString() != "TreeSortModel"){
        m_sort_model->setSourceModel(model);
        QTreeView::setModel(model);
        auto model_ = qobject_cast<TreeModel*>(model);
        m_hierarchy_list = model_->hierarchical_list();
        if(m_toolBar){
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_add_item).c_str(), true);
            if(!model_->hierarchical_list()){
                m_toolBar->setButtonVisible(arcirk::enum_synonym(table_add_group).c_str(), false);
                m_toolBar->setButtonVisible(arcirk::enum_synonym(table_move_to_item).c_str(), false);
            }
            if(!m_inners_dialogs)
                m_toolBar->setButtonVisible(arcirk::enum_synonym(table_move_to_item).c_str(), false);
        }

        foreach(const auto& column, default_column_hidden){
            if(model_->columns().indexOf(column) != -1){
                hideColumn(model_->column_index(column));
            }
        }
        for (auto itr = model_->get_conf()->columns().begin(); itr != model_->get_conf()->columns().end() ; ++itr) {
            if(itr->not_public){
                hideColumn(model_->column_index(itr->name.c_str()));
            }
        }
        connect(model_, &TreeModel::fetch, this, &TreeViewWidget::onTreeFetch);
    }
    if(m_toolBar){
        m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_add_item).c_str(), true);
        m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_add_group).c_str(), true);
    }
    //return QTreeView::setModel(m_sort_model);
}

void TreeViewWidget::setEnabled(bool value)
{
    if(m_toolBar)
        m_toolBar->setEnabled(value);
}

int TreeViewWidget::content_width(int column)
{
    auto model = get_model();
    int result = 0;
    for (int i = 0; i < model->rowCount(); ++i) {
        QFontMetrics fm(fontMetrics());
        auto text = model->index(i, column).data().toString();
        result = std::max(fm.horizontalAdvance(text), result);
        auto index = model->index(i, column, QModelIndex());
        result = std::max(text_width(column, index), result);
    }
    return result;
}

void TreeViewWidget::clear()
{
    m_sort_model->setSourceModel(nullptr);
}

void TreeViewWidget::setTableToolBar(TableToolBar *value)
{
    m_toolBar = value;
    connect(m_toolBar, &TableToolBar::itemClicked, this, &TreeViewWidget::onToolBarItemClicked);
    auto model = get_model();
    if(!model || !m_toolBar)
        return;
    m_hierarchy_list = model->hierarchical_list();
    m_toolBar->setHierarchyState(m_hierarchy_list);
    m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_add_item).c_str(), true);
    m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_add_group).c_str(), true);
    m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_to_item).c_str(), false);
    m_toolBar->setButtonVisible(arcirk::enum_synonym(table_add_group).c_str(), m_hierarchy_list);
    m_toolBar->setButtonVisible(arcirk::enum_synonym(table_move_to_item).c_str(), m_hierarchy_list);


}

TableToolBar *TreeViewWidget::toolBar() const
{
    return m_toolBar;
}

void TreeViewWidget::set_inners_dialogs(bool value)
{
    m_inners_dialogs = value;
}

void TreeViewWidget::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    auto index= get_index(current);

    if(m_toolBar){
        auto model = get_model();
        if(!model){
            return QTreeView::currentChanged(current, previous);;
        }

        m_hierarchy_list = model->hierarchical_list();

        if(index.isValid()){
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_edit_item).c_str(), true);
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_delete_item).c_str(), true);
            if(model->is_group(index)){
                m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_add_group).c_str(), true);
                m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_add_item).c_str(), true);
            }else{
                m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_add_group).c_str(), false);
                if(m_hierarchy_list)
                    m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_add_item).c_str(), false);
            }

            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_to_item).c_str(), true);
            if(!model->hierarchical_list()){
                if(index.row() == model->rowCount() - 1)
                    m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_down_item).c_str(), false);
                else if(index.row() < model->rowCount() - 1)
                    m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_down_item).c_str(), true);

                if(index.row() != 0)
                    m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_up_item).c_str(), true);
                else
                    m_toolBar->setButtonEnabled("move_up_item", false);
            }
        }else{
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_add_group).c_str(), false);
            if(m_hierarchy_list)
                m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_add_item).c_str(), false);
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_to_item).c_str(), false);
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_delete_item).c_str(), false);
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_edit_item).c_str(), false);
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_down_item).c_str(), false);
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_up_item).c_str(), false);
        }
    }

    emit itemClicked(current);
    return QTreeView::currentChanged(current, previous);
}

void TreeViewWidget::hide_default_columns()
{
    auto model = get_model();
    if(!model)
        return;

    for(auto const& f: model->predefined_fields()){
        hideColumn(model->column_index(f.c_str()));
    }

}

void TreeViewWidget::close_editor()
{
    auto w = this->indexWidget(currentIndex());
    if(w){
        this->commitData(w);
        this->closeEditor(w, TreeItemDelegate::EndEditHint::SubmitModelCache);
    }

}

void TreeViewWidget::openNewItemDialog()
{
    auto model = get_model();
    if(!model)
        return;
    auto current_index = this->current_index();
    if(current_index.isValid()){
        if (model->is_group(current_index)){
            editRowInDialog(QModelIndex(), current_index);
        }else
            editRowInDialog(QModelIndex(), current_index.parent());
    }else
        editRowInDialog(QModelIndex(), QModelIndex());

//    json row_data = model->empty_data();
//    m_hierarchy_list = model->hierarchical_list();
//
//    if(m_only_groups_in_root || m_hierarchy_list){
//        if (!current_index.isValid() || !model->is_group(current_index))
//            return;
//        row_data["is_group"] = 0;
//        row_data["ref"] = "";
//        row_data["parent"] = uuid_to_string(model->ref(current_index)).toStdString();
//    }else
//        row_data["parent"] = NIL_STRING_UUID;
//
//    User_Data m_udata = model->user_data_values();
//    QString m_parent_name = "";
//    if(row_data["parent"] != NIL_STRING_UUID){
//        auto indexParent = model->find(QUuid::fromString(row_data["parent"].get<std::string>().c_str()));
//        if(indexParent.isValid()){
//            auto obj = model->to_object(indexParent);
//            m_parent_name = obj.value("first", "").c_str();
//            if(m_parent_name.isEmpty())
//                m_parent_name = obj.value("name", "").c_str();
//        }
//    }
//    std::string m_current_parent = row_data.value("parent", NIL_STRING_UUID);
//    auto dlg = RowDialog(row_data, m_udata, this, model->columns_aliases(),
//                         QList<QString>{"data_type", "_id", "path"},
//                         model->columns_order(),
//                         m_parent_name);
//    dlg.setIcon(model->rows_icon(item_icons_enum::Item));
//    if(dlg.exec()){
//        auto data = dlg.data();
//        if(data["parent"].get<std::string>() != m_current_parent)
//            current_index = QModelIndex();
//        auto n_index = model->add(dlg.data(), current_index);
//        model->set_row_inner_roles(n_index.row(), dlg.inner_roles());
//        auto itr = m_udata.find(tree::RepresentationRole);
//        if(itr != m_udata.end()){
//            for (int i = 0; i < itr.value().size(); ++i) {
//                auto index_ = model->index(n_index.row(), i, n_index.parent());
//                model->setData(index_, itr.value()[model->column_name(i)], tree::RepresentationRole);
//            }
//
//        }
//        emit addTreeItem(n_index, dlg.data());
//    }

}


void TreeViewWidget::openNewGroupDialog()
{
    auto model = get_model();
    if(!model)
        return;
    auto current_index = this->current_index();
    if(current_index.isValid()){
        if (model->is_group(current_index)){
            editRowInDialog(QModelIndex(), current_index, true);
        }else
            editRowInDialog(QModelIndex(), current_index.parent(), true);
    }else
        editRowInDialog(QModelIndex(), QModelIndex(), true);
//    json row_data = model->empty_data();
//
//    row_data["is_group"] = 1;
//    row_data["ref"] = "";
//    if(current_index.isValid() && model->is_group(current_index)){
//        row_data["parent"] = uuid_to_string(model->ref(current_index)).toStdString();
//    }else{
//        if(!model->is_group(current_index))
//            return;
//        row_data["parent"] = NIL_STRING_UUID;
//    }
//
//    if(m_add_group_in_root_only)
//        row_data["parent"] = NIL_STRING_UUID;
//
//    User_Data m_udata = model->user_data_values();
//    QString m_parent_name = "";
//    std::string m_current_parent = row_data.value("parent", NIL_STRING_UUID);
//    if(row_data["parent"] != NIL_STRING_UUID){
//        auto indexParent = model->find(QUuid::fromString(row_data["parent"].get<std::string>().c_str()));
//        if(indexParent.isValid()){
//            auto obj = model->to_object(indexParent);
//            m_parent_name = obj.value("first", "").c_str();
//            if(m_parent_name.isEmpty())
//                m_parent_name = obj.value("name", "").c_str();
//        }
//    }
//    QString path = "/";
//    if(current_index.isValid())
//        path = model->path(current_index);
//
//    auto dlg = RowDialog(row_data, m_udata, this, model->columns_aliases(), QList<QString>{"data_type", "_id", "path"},
//                         model->columns_order(),
//                         m_parent_name, {}, path);
//    dlg.setIcon(model->rows_icon(item_icons_enum::ItemGroup));
//    if(dlg.exec()){
//        auto data = dlg.data();
//        if(!m_add_group_in_root_only){
//            if(data["parent"].get<std::string>() != m_current_parent)
//                current_index = QModelIndex();
//        }else
//            current_index = QModelIndex();
//        auto n_index = model->add(dlg.data(), current_index);
//        emit addTreeItem(n_index, dlg.data());
//    }
}

//void TreeViewWidget::openOpenEditDialog()
//{
//    auto model = get_model();
//    if(!model)
//        return;
//    auto current_index = this->current_index();
//    json row_data = model->empty_data();
//
//    if(!current_index.isValid())
//        return;
//    row_data = model->to_object(current_index);
//    QString m_parent_name = "";
//    if(row_data["parent"] != NIL_STRING_UUID){
//        auto indexParent = model->find(QUuid::fromString(row_data["parent"].get<std::string>().c_str()));
//        if(indexParent.isValid()){
//            auto obj = model->to_object(indexParent);
//            m_parent_name = obj.value("first", "").c_str();
//            if(m_parent_name.isEmpty())
//                m_parent_name = obj.value("name", "").c_str();
//        }
//    }
//    User_Data m_udata = model->user_data_values(current_index);
//    auto dlg = RowDialog(row_data, m_udata, this, model->columns_aliases(), QList<QString>{"data_type", "_id", "path"}, model->columns_order(), m_parent_name,
//                         model->row_inner_roles(current_index.row(), current_index.parent()));
//    if(dlg.exec()){
//        model->set_object(current_index, dlg.data());
//        model->set_row_inner_roles(current_index.row(), dlg.inner_roles());
//        auto itr = m_udata.find(tree::RepresentationRole);
//        if(itr != m_udata.end()){
//            for (int i = 0; i < itr.value().size(); ++i) {
//                auto index_ = model->index(current_index.row(), i, current_index.parent());
//                model->setData(index_, itr.value()[model->column_name(i)], tree::RepresentationRole);
//            }
//        }
//        emit editTreeItem(current_index, dlg.data());
//    }
//}

//void TreeViewWidget::openOpenMoveToDialog()
//{
//    auto model = get_model();
//    if(!model)
//        return;
//    auto current_index = this->current_index();
//    json row_data = model->empty_data();
//
//    if(!current_index.isValid())
//        return;
//
//    auto table = model->to_table_model(QModelIndex(),true, true);
//    auto gr_model = new TreeItemModel(this);
//    gr_model->set_columns_order(model->columns_order());
//    gr_model->set_column_aliases(gr_model->columns_aliases());
//    gr_model->set_table(table);
//
//    auto dlg = SelectItemDialog(gr_model, this);
//    dlg.setWindowTitle("Выбрать группу");
//    if(dlg.exec()){
//        auto obj = dlg.result();
//        auto parent_index = model->find(QUuid::fromString(obj.value("ref", NIL_STRING_UUID).c_str()));
//        current_index = this->current_index();
//        auto current_object = model->to_object(current_index);
//        if(parent_index.isValid()){
//            auto verify = model->belongsToItem(parent_index, current_index);
//            if(verify){
//                QMessageBox::critical(this, "Ошибка", "В выбранную группу перемещение не возможно!");
//                return;
//            }
//            if(current_index.parent() == parent_index)
//                return;
//            model->move_to(current_index, parent_index);
//            auto n_index = model->find(QUuid::fromString(current_object["ref"].get<std::string>().c_str()), parent_index);
//            if(n_index.isValid())
//                    emit editTreeItem(n_index, model->to_object(n_index));
//        }
//
//    }
//}

//void TreeViewWidget::deleteItemCommand()
//{
//    auto model = get_model();
//    if(!model)
//        return;
//    auto current_index = this->current_index();
//    json row_data = model->empty_data();
//
//    if(!current_index.isValid())
//        return;
//    if(QMessageBox::question(this, "Удаление", "Удалить выбранную строку?") == QMessageBox::Yes){
//        auto obj = model->to_object(current_index);
//        model->remove(current_index);
//        emit deleteTreeItem(obj);
//    }
//}

void TreeViewWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        startPos = event->pos();
    return QTreeView::mousePressEvent(event);
}

void TreeViewWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        int distance = (event->pos() - startPos).manhattanLength();
        if (distance >= QApplication::startDragDistance())
            performDrag();
    }
    return QTreeView::mouseMoveEvent(event);
}

void TreeViewWidget::dragMoveEvent(QDragMoveEvent *event)
{
    //qDebug() << __FUNCTION__;
    return QTreeView::dragMoveEvent(event);
}

void TreeViewWidget::dropEvent(QDropEvent *event)
{
//    auto model = get_model();
//    if(!model)
//        return;
//    if(!model->enable_drop())
//        return;
//    auto text = event->mimeData()->text();
//    auto sender = event->mimeData()->property("typeName");
//    if(json::accept(text.toStdString())){
//        auto empty = model->empty_data();
//        tree::fill_property_values(empty, json::parse(text.toStdString()));
//        if(m_behavior == behaviorDefault){
//            auto index = model->add(empty);
//            emit doDropEvent(index, sender.toString());
//        }else
//                emit doDropEventJson(empty, sender.toString());
//    }
    return QTreeView::dropEvent(event);
}

void TreeViewWidget::dragEnterEvent(QDragEnterEvent *event)
{
//    //qDebug() << __FUNCTION__;
//    auto model = get_model();
//    if(!model)
//        return;
//    if(!model->enable_drag())
//        return;
//    event->acceptProposedAction();
//    setDropIndicatorShown(true);
//    setDragDropMode(QAbstractItemView::InternalMove);
    return QTreeView::dragEnterEvent(event);
}

void TreeViewWidget::changeEvent(QEvent *event)
{
    return QTreeView::changeEvent(event);
}

void TreeViewWidget::performDrag() {

//    //    qDebug() << __FUNCTION__;
//
//    auto model = get_model();
//    if(!model)
//        return;
//    auto index = current_index();
//    if(!m_drag_groups){
//        if(model->rowCount(index)> 0)
//            return;
//    }
//    if(!m_drag_exceptions.empty()){
//        for (auto itr = m_drag_exceptions.begin(); itr != m_drag_exceptions.end(); ++itr) {
//            QVariant val = model->index(index.row(), itr.key(), index.parent()).data();
//            QList<QVariant> lst = itr.value();
//            if(lst.indexOf(val) != -1)
//                return;
//        }
//    }
//    auto *mimeData = new QMimeData;
//    mimeData->setProperty("typeName", this->property("typeName"));
//    if(m_behavior == behaviorDefault || m_behavior == behaviorEmit )
//        mimeData->setText(model->to_object(index).dump().c_str());
//    else if(m_behavior == behaviorText){
//        if(m_drop_column.isEmpty()){
//            QString text;
//            if(data_is_base64_)
//                text = QByteArray::fromBase64(index.data().toString().toUtf8());
//            else
//                text = index.data().toString();
//            mimeData->setText(text);
//        }else{
//            int i = model->column_index(m_drop_column);
//            if(i != -1){
//                auto text = model->index(index.row(), i, index.parent()).data().toString();
//                if(data_is_base64_)
//                    text = QByteArray::fromBase64(text.toUtf8());
//                mimeData->setText(text);
//            }
//        }
//    }
//    auto *drag = new QDrag(this);
//    drag->setMimeData(mimeData);
//    drag->exec(Qt::MoveAction);

}

int TreeViewWidget::text_width(int column, const QModelIndex &parent, const int& result)
{
    auto model = get_model();
    if(!model)
        return 0;
    int res = result;
    for (int itr = 0; itr < model->rowCount(parent); ++itr) {
        QString text = model->data(model->index(itr, column, parent)).toString();
        if(!text.isEmpty()){
            QFontMetrics fm(fontMetrics());
            res = std::max(fm.horizontalAdvance(text), res);

        }
        res = text_width(column, model->index(itr, column, parent), res);
    }
    return res;
}

void TreeViewWidget::onItemClicked(const QModelIndex &index)
{
    if(!index.isValid())
        return;
    auto index_ = get_index(index);
    emit itemClicked(index_);
}

void TreeViewWidget::onItemDoubleClicked(const QModelIndex &index)
{
    if(!index.isValid())
        return;
    auto index_ = get_index(index);
    emit itemDoubleClicked(index_);
}

void TreeViewWidget::onSourceModelChanged()
{
    auto model_ = get_model();
    if(model_){
        set_hierarchy_list(model_->hierarchical_list());
        auto delegate = (TreeItemDelegate*)this->itemDelegate();
        if(delegate)
            delegate->setGridLine(!model_->hierarchical_list());
//        if(!m_not_sort)
//            setSortingEnabled(!model_->hierarchical_list());
//        else
        setSortingEnabled(m_not_sort);
    }
}

void TreeViewWidget::onTreeFetch(const QModelIndex &parent)
{
    emit fetch(parent);
}

void TreeViewWidget::onToolBarItemClicked(const QString &buttonName)

{
    json b = buttonName.toStdString();
    auto btn = b.get<table_toolbar_buttons>();
    if(!m_inners_dialogs){
        if(m_allow_def_commands){
            if(btn == table_add_item){
                addRow();
            }else if(btn == table_add_group){
                //
            }else if(btn == table_delete_item){
               // deleteItemCommand();
            }else if(btn == table_edit_item){
                editRow();
            }else if(btn == table_move_to_item){
                //
            }else if(btn == table_move_up_item){
                moveUp();
            }else if(btn == table_move_down_item){
                moveDown();
            }else
                    emit toolBarItemClicked(buttonName);
        }else
                emit toolBarItemClicked(buttonName);

    }else{
        if(btn == table_add_item){
            openNewItemDialog();
        }else if(btn == table_add_group){
            openNewGroupDialog();
        }else if(btn == table_delete_item){
            //deleteItemCommand();
        }else if(btn == table_edit_item){
            //openOpenEditDialog();
        }else if(btn == table_move_to_item){
            //openOpenMoveToDialog();
        }else if(btn == table_move_up_item){
            moveUp();
        }else if(btn == table_move_down_item){
            moveDown();
        }else
                emit toolBarItemClicked(buttonName);
    }
}

void TreeViewWidget::addRow()
{
    auto model = get_model();
    if(!model)
        return;
    auto current_index = this->current_index();
    json row_data = model->empty_data();
    m_hierarchy_list = model->hierarchical_list();

    if(m_only_groups_in_root || m_hierarchy_list){
        if (!current_index.isValid() || !model->is_group(current_index))
            return;
    }

    auto n_index = model->add(row_data, current_index);

    auto proxy_index = m_sort_model->mapFromSource(n_index);

    if(proxy_index.isValid()){
        setCurrentIndex(proxy_index);
    }

}

void TreeViewWidget::editRow()
{
    auto current_index = this->current_index();
    if(current_index.isValid()){
        this->edit(this->currentIndex(), AllEditTriggers, new QEvent(QEvent::Type::MouseButtonDblClick));
    }
}

void TreeViewWidget::moveUp()
{
    auto model = get_model();
    if(!model)
        return;
    auto current_index = this->current_index();

    if(current_index.isValid())
        model->move_up(current_index);

    auto index = this->current_index();
    if(!model->hierarchical_list()){
        if(index.row() == model->rowCount() - 1)
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_down_item).c_str(), false);
        else if(index.row() < model->rowCount() - 1)
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_down_item).c_str(), true);

        if(index.row() != 0)
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_up_item).c_str(), true);
        else
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_up_item).c_str(), false);
    }
}

void TreeViewWidget::moveDown()
{
    auto model = get_model();
    if(!model)
        return;
    auto current_index = this->current_index();

    if(current_index.isValid())
        model->move_down(current_index);

    auto index = this->current_index();
    if(!model->hierarchical_list()){
        if(index.row() == model->rowCount() - 1)
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_down_item).c_str(), false);
        else if(index.row() < model->rowCount() - 1)
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_down_item).c_str(), true);

        if(index.row() != 0)
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_up_item).c_str(), true);
        else
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_up_item).c_str(), false);
    }
}

void TreeViewWidget::hide_not_ordered_columns() {
    auto model = get_model();
    if(!model)
        return;
    const auto& columns_ordered = model->get_conf()->columns_order();
    for (auto column = model->get_conf()->columns().begin(); column != model->get_conf()->columns().end(); ++column) {
        if(columns_ordered.indexOf(column->name.c_str()) == -1)
            hideColumn(model->column_index(column->name.c_str()));
    }
}

void TreeViewWidget::set_only_groups_in_top_level(bool value) {
    m_only_groups_in_root = value;
}

bool TreeViewWidget::only_groups_in_top_level() {
    return m_only_groups_in_root;
}

void TreeViewWidget::editRowInDialog(const QModelIndex &index, const QModelIndex &parent, bool isGroup ) {

    auto model = get_model();
    bool is_new = false;
    if(!model)
        return;
    json data;
    if(index.isValid()){
        data = model->row(index);
    }else{
        data = model->empty_data();
        is_new = true;
        data["is_group"] = isGroup;
        data["parent"] = to_byte(to_binary(model->row_uuid(parent)));
    }

    auto ordered = model->columns_order();

    auto dlg = TableRowDialog(data, ordered, model->get_conf(), this);
    if(dlg.exec()){
        if(is_new){
            const auto result = dlg.result();
            auto n_index = model->add(result, parent);
            if(model->use_database())
                model->onRowChanged(n_index);
            emit rowChanged(n_index.row());
        }else{
            model->set_object(index, dlg.result());
            if(model->use_database())
                model->onRowChanged(index);
            emit rowChanged(index.row());
        }
    }
}

#endif
