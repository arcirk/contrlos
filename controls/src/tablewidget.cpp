//
// Created by admin on 14.01.2024.
//
#include "../include/tablewidget.h"
#include "../include/tablemodel.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include "../include/tableitemdelegate.h"
#include "../include/tablerowdialog.h"
#include <QAction>

using namespace arcirk::widgets;

TableWidget::TableWidget(QWidget *parent, const QString &typeName)
        : QTableView(parent)
{

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    auto header = verticalHeader();
    header->setSectionResizeMode(QHeaderView::Fixed);
    header->setDefaultSectionSize(22);

    auto delegate = new TableItemDelegate(this);
    setItemDelegate(delegate);
    setEditTriggers(QAbstractItemView::AllEditTriggers);
    delegate->setParent(this);

    connect(this, &QAbstractItemView::clicked, this, &TableWidget::onItemClicked);

    m_toolBar = nullptr;
    m_inners_dialogs = false;
    m_allow_def_commands = true;
    m_standard_context_menu = true;
    m_standard_menu = nullptr;

    horizontalHeader()->setStretchLastSection(true);

    connect(delegate, &TableItemDelegate::selectValue, this, &TableWidget::onSelectValue);
    connect(delegate, &TableItemDelegate::mouseButtonDblClick, this, &TableWidget::onItemDoubleClicked);
    connect(delegate, &TableItemDelegate::mouseButtonRightClick, this, &TableWidget::onMouseRightItemClick);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotCustomMenuRequested(QPoint)));
}

void TableWidget::setModel(QAbstractItemModel* model) {

    QTableView::setModel(model);

    auto inner_model = qobject_cast<TableModel*>(model);
    if(inner_model){
        if(inner_model->columns().indexOf("ref") != -1){
            hideColumn(inner_model->column_index("ref"));
        }
        for (auto itr = inner_model->get_conf()->columns().begin(); itr != inner_model->get_conf()->columns().end() ; ++itr) {
            if(itr->not_public){
                hideColumn(inner_model->column_index(itr->name.c_str()));
            }
        }
    }
    if(m_toolBar){
        m_toolBar->setEnabled(true);
        m_toolBar->setHierarchyState(false);
        if(m_standard_context_menu)
            m_standard_menu = m_toolBar->context_menu();
    }
}

void TableWidget::onItemClicked(const QModelIndex &index) {
    if(!index.isValid())
        return;
    emit itemClicked(index);
}

void TableWidget::onItemDoubleClicked(const QModelIndex &index) {
    if(!index.isValid())
        return;
    if(!m_inners_dialogs) {
        emit itemDoubleClicked(index);
    }else{
        editRowInDialog(index);
    }
}

void TableWidget::currentChanged(const QModelIndex &current, const QModelIndex &previous) {

    auto index= current;
    auto model = this->model();
    if(!model){
        return QTableView::currentChanged(current, previous);;
    }
    if(m_toolBar){
        if(index.isValid()){
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_edit_item).c_str(), true);
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_delete_item).c_str(), true);
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_add_item).c_str(), true);
            if(index.row() == model->rowCount() - 1)
                m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_down_item).c_str(), false);
            else if(index.row() < model->rowCount() - 1)
                m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_down_item).c_str(), true);

            if(index.row() != 0)
                m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_up_item).c_str(), true);
            else
                m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_up_item).c_str(), false);
        }else{
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_delete_item).c_str(), false);
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_edit_item).c_str(), false);
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_down_item).c_str(), false);
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_up_item).c_str(), false);
            m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_add_item).c_str(), true);
        }
        emit tableCurrentChanged(current, previous);
    }
    QTableView::currentChanged(current, previous);
}

void TableWidget::setTableToolBar(TableToolBar *value) {
    m_toolBar = value;
    connect(m_toolBar, &TableToolBar::itemClicked, this, &TableWidget::onToolBarItemClicked);
    auto model = this->model();
    if(!model || !m_toolBar)
        return;
    m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_add_item).c_str(), true);
//    m_toolBar->setButtonVisible(arcirk::enum_synonym(table_add_group).c_str(), false);
//    m_toolBar->setButtonVisible(arcirk::enum_synonym(table_move_to_item).c_str(), false);
    m_toolBar->setHierarchyState(false);
    if(m_standard_context_menu)
        m_standard_menu = m_toolBar->context_menu();
}

void TableWidget::onToolBarItemClicked(const QString &buttonName) {
    json b = buttonName.toStdString();
    auto btn = b.get<table_toolbar_buttons>();
    if(!m_inners_dialogs){
        if(m_allow_def_commands){
            if(btn == table_add_item) {
                addRow();
            }else if(btn == table_delete_item){
                removeRow();
            }else if(btn == table_edit_item){
                editRow();
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
            editRowInDialog(QModelIndex());
        }else if(btn == table_delete_item){
            removeRow();
        }else if(btn == table_edit_item){
            editRowInDialog(currentIndex());
        }else if(btn == table_move_up_item){
            moveUp();
        }else if(btn == table_move_down_item){
            moveDown();
        }else
            emit toolBarItemClicked(buttonName);
    }
}

void TableWidget::set_inners_dialogs(bool value) {
    m_inners_dialogs = value;
}

void TableWidget::addRow() {

    auto model = qobject_cast<TableModel*>(this->model());
    if(!model)
        return;
    auto row_data = model->empty_data();
    auto index = model->add(row_data);
    if(index.isValid()){
        setCurrentIndex(index);
    }
    if(model->use_database())
        model->onRowChanged(index);
}

void TableWidget::editRow() {
    auto current_index = this->currentIndex();
    if(current_index.isValid()){
        this->edit(this->currentIndex(), AllEditTriggers, new QEvent(QEvent::Type::MouseButtonDblClick));
    }
    auto model = qobject_cast<TableModel*>(this->model());
    if(model->use_database())
        model->onRowChanged(current_index);
}

void TableWidget::moveUp() {
    auto model = qobject_cast<TableModel*>(this->model());
    if(!model)
        return;
    auto current_index = this->currentIndex();

    int row = current_index.row() - 1;

    if(current_index.isValid())
        model->move_up(current_index);


    if(row== model->rowCount() - 1)
        m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_down_item).c_str(), false);
    else if(row < model->rowCount() - 1)
        m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_down_item).c_str(), true);

    if(row != 0 && model->rowCount() > 1)
        m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_up_item).c_str(), true);
    else
        m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_up_item).c_str(), false);

//    if(model->use_database())
//        model->updateRowPosition();

    emit rowMove();
}

void TableWidget::moveDown() {

    auto model = qobject_cast<TableModel*>(this->model());
    if(!model)
        return;
    auto current_index = this->currentIndex();

    int row = current_index.row() + 1;

    if(current_index.isValid())
        model->move_down(current_index);

    if(row== model->rowCount() - 1)
        m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_down_item).c_str(), false);
    else if(row < model->rowCount() - 1)
        m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_down_item).c_str(), true);

    if(row != 0 && model->rowCount() > 1)
        m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_up_item).c_str(), true);
    else
        m_toolBar->setButtonEnabled(arcirk::enum_synonym(table_move_up_item).c_str(), false);

//    if(model->use_database())
//        model->updateRowPosition();

    emit rowMove();
}

void TableWidget::removeRow() {
    auto model = qobject_cast<TableModel*>(this->model());
    if(!model)
        return;
    auto current_index = this->currentIndex();
    json row_data = model->empty_data();

    if(!current_index.isValid())
        return;
    if(QMessageBox::question(this, "Удаление", "Удалить выбранную строку?") == QMessageBox::Yes){
        auto object = model->row(current_index);
        model->remove(current_index);
        if(model->use_database())
            model->removeRow(object);
        emit removeTableItem(object);
    }
}

TableToolBar *TableWidget::toolBar() {
    if(m_toolBar)
        return m_toolBar;
    else
        return nullptr;
}

void TableWidget::editRowInDialog(const QModelIndex &index) {
    auto model = qobject_cast<TableModel*>(this->model());
    bool is_new = false;
    if(!model)
        return;
    json data;
    if(index.isValid()){
        data = model->row(index);
    }else{
        data = model->empty_data();
        is_new = true;
    }
    auto ordered = model->columns_order();
    auto dlg = TableRowDialog(data, ordered, model->get_conf(), this);
    if(dlg.exec()){
        if(is_new){
            const auto result = dlg.result();
            auto n_index = model->add(result);
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

void TableWidget::allow_default_command(bool value) {
    m_allow_def_commands = value;
}

void TableWidget::header_visible(Qt::Orientation orientation, bool value) {
    if(orientation == Qt::Horizontal)
        this->horizontalHeader()->setVisible(value);
    else
        this->verticalHeader()->setVisible(value);
}

void TableWidget::close_editor() {
    auto w = this->indexWidget(currentIndex());
    if(w){
        this->commitData(w);
        this->closeEditor(w, TableItemDelegate::EndEditHint::SubmitModelCache);
    }
}

void TableWidget::clear() {
    auto model = qobject_cast<TableModel*>(this->model());
    if(model){
        model->clear();
    }
}

void TableWidget::onSelectValue(int row, int col, const table_command& type) {
    if(type == selectDirectory || type == selectFile){
        QString result;
        if(type == selectDirectory)
            result = QFileDialog::getExistingDirectory(this, "Выбор каталога");
        else
            result = QFileDialog::getOpenFileName(this, "Выбор файла", "", "Все файлы (*.*)");
        if(!result.isEmpty()){
            //auto value = qvariant_cast<QString>(result);
            auto model = qobject_cast<TableModel*>(this->model());
            if(model){
                model->setData(model->index(row, col), result);
            }
        }
    }else if(type == selectReadFile){
        auto result = QFileDialog::getOpenFileName(this, "Выбор файла");
        if(!result.isEmpty()){
            QFileInfo info(result);
            auto model = qobject_cast<TableModel*>(this->model());
            if(model) {
                ByteArray ba{};
#ifdef Q_OS_WINDOWS
                arcirk::read_file(result.toLocal8Bit().toStdString(), ba);
#else
                arcirk::read_file(result.toStdString(), ba);
#endif
                auto j_ba = to_byte(to_binary(ba, subtypeByte));
                model->setData(model->index(row, col), byte_to_qbyte(j_ba));
            }
        }
    }else if(type == selectWriteFile){
        auto model = qobject_cast<TableModel*>(this->model());
        if(model) {
            auto data = model->data(model->index(row, col), TABLE_DATA);
            if (data.isValid() && data.typeId() == QMetaType::QByteArray) {
                auto var = item_data();
                var.from_json(qbyte_to_byte(data.toByteArray()));
                auto result = QFileDialog::getSaveFileName(this, "Сохранить как...", var.representation().c_str());
                if(!result.isEmpty()){
                    if(!var.data()->data.empty())
#ifdef Q_OS_WINDOWS
                        arcirk::write_file(result.toLocal8Bit().toStdString(), var.data()->data);
#else
                        arcirk::write_file(result.toStdString(), var.data()->data);
#endif
                }
            }
        }

    }
}

void TableWidget::set_standard_context_menu(bool value) {
    m_standard_context_menu = value;
    if(m_standard_context_menu)
        m_standard_menu = m_toolBar->context_menu();
    else
        m_standard_menu = nullptr;
}

void TableWidget::onMouseRightItemClick(const QModelIndex &index) {

}

void TableWidget::slotCustomMenuRequested(QPoint pos) {
    if(m_standard_context_menu && m_standard_menu){
        m_standard_menu->popup(viewport()->mapToGlobal(pos));
    }
}
