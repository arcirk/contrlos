//
// Created by admin on 14.01.2024.
//
#include <QStylePainter>
#include <QApplication>
#include <QStyleOption>
#include <QStyle>
#include <QDebug>
#include "../include/tableitemdelegate.h"
#include "../include/treeitemvariant.h"
#include "../include/treeitemcheckbox.h"
#include <QFileDialog>
#include <QMouseEvent>
#include <QEvent>

using namespace arcirk::widgets;

TableItemDelegate::TableItemDelegate(QObject * parent)
    : QStyledItemDelegate{parent}
{
    m_parent = nullptr;
    m_current_index = std::make_pair(0, 0);
    connect(this, &TableItemDelegate::startEdit, this, &TableItemDelegate::onStartEdit);
    connect(this, &TableItemDelegate::endEdit, this, &TableItemDelegate::onEndEdit);
}

QWidget *TableItemDelegate::createEditor(QWidget * parent,
    const QStyleOptionViewItem &option,
    const QModelIndex &index
) const
{
    emit startEdit(index.row(), index.column());
    return new TreeItemVariant(index.row(), index.column(), parent);
}

void TableItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    //qDebug() << __FUNCTION__ << " TABLE_DATA=" << TABLE_DATA;
    auto data = index.data(TABLE_DATA);
    auto role = (editor_inner_role)index.data(TABLE_ITEM_ROLE).toInt();
    auto select_type = (editor_inner_role)index.data(TABLE_ITEM_SELECT_TYPE).toInt();
    if(data.isValid() && data.typeId() == QMetaType::QByteArray){
        auto control = qobject_cast<TreeItemVariant*>(editor);
        if(control){
            if(role == editor_inner_role::editorBoolean)
                control->checkBox(true);
            control->setRole(role);
            control->setData(data);
            control->selectType(select_type);
            control->setFrame(false);
            connect(control, &TreeItemVariant::selectObject, this, &TableItemDelegate::onSelectObject);

        }else
            return QStyledItemDelegate::setEditorData(editor, index);
    }else
        return QStyledItemDelegate::setEditorData(editor, index);
}

void TableItemDelegate::onItemValueChanged(int /*row*/, int /*col*/, const QVariant &/*value*/) {

}

void TableItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    auto control = qobject_cast<TreeItemVariant*>(editor);
    if(control){
        model->setData(index, control->data(), TABLE_DATA);
        disconnect(control);
    }else
        QStyledItemDelegate::setModelData(editor, model, index);

    emit endEdit(index.row(), index.column());
}

void TableItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

//    if(m_current_index == std::make_pair(index.row(),index.column())){
//        auto item_subtype = (variant_subtype)index.data(TABLE_ITEM_SUBTYPE).toInt();
//        if(item_subtype == subtypeByte)
//            return;
//        else
//            return QStyledItemDelegate::paint(painter, option, index);
//    }

    QStyle* style = qApp->style();
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    auto value = index.data(Qt::DisplayRole);
    auto item_role = index.data(TABLE_ITEM_ROLE).toInt();
    //auto item_subtype = (variant_subtype)index.data(TABLE_ITEM_SUBTYPE).toInt();

    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt);
    textRect.moveLeft(textRect.left() + 4);
    textRect.setWidth(textRect.width() -  8);
    if (opt.state & QStyle::State_Selected &&
        opt.state & QStyle::State_Active){
        painter->setPen(QColor(Qt::white));
    }else{
        painter->setPen(opt.palette.color(QPalette::Text));
    }
    if(item_role != editorBoolean){
        if(item_role == editorNumber)
            painter->drawText(textRect,  Qt::AlignVCenter | Qt::AlignRight, value.toString());
        else
            QStyledItemDelegate::paint(painter, option, index);
    }else{
//        if(value.typeId() == QMetaType::Bool)
//            painter->drawText(textRect,  Qt::AlignVCenter | Qt::AlignLeft, value.toBool() ? "Истина" : "Ложь");
//        else if (value.typeId() == QMetaType::Int)
//            painter->drawText(textRect,  Qt::AlignVCenter | Qt::AlignLeft, value.toInt() > 0 ? "Истина" : "Ложь");
//        else if (value.typeId() == QMetaType::QString)
//            painter->drawText(textRect,  Qt::AlignVCenter | Qt::AlignLeft, value.toString());
//        else
//            QStyledItemDelegate::paint(painter, option, index);
        QStyleOptionButton chk;
        QRect checkbox_rect = style->subElementRect(QStyle::SE_CheckBoxIndicator, &opt);
        checkbox_rect.moveCenter(option.rect.center());
        chk.rect = checkbox_rect;
        auto b = value.toBool();
        chk.state |= b ? QStyle::State_On : QStyle::State_Off;
        chk.state |= QStyle::State_Enabled;
        style->drawControl(QStyle::CE_CheckBox, &chk, painter, 0);
    }


}

QSize TableItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QStyledItemDelegate::sizeHint(option, index);
}

void TableItemDelegate::setParent(QWidget *parent) {
    m_parent = parent;
}

void TableItemDelegate::onSelectObject(int row, int col, const table_command& type) {
    emit selectValue(row, col, type);
}

void TableItemDelegate::onEndEdit(int /*row*/, int /*col*/) {
    m_current_index = std::make_pair(0,0);
}

void TableItemDelegate::onStartEdit(int row, int col) {
    m_current_index = std::make_pair(row,col);
}

bool TableItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                    const QModelIndex &index) {
    if (!index.isValid())
    {
        return false;
    }

    auto mouseEvent = dynamic_cast<QMouseEvent*>(event);

//    // Added this debugging purposes!
//    if (mouseEvent->button() == Qt::RightButton)
//    {
//        qDebug() << "Delegate received right mouse click";
//    }

    // Handling of left mouse button clicks
    if (mouseEvent->button() == Qt::LeftButton)
    {
        if(event->type() == QEvent::MouseButtonDblClick){
            //std::cout << "QEvent::MouseButtonDblClick" << std::endl;
            emit mouseButtonDblClick(index);
        }
    }else if(mouseEvent->button() == Qt::RightButton){
        emit mouseButtonRightClick(index);
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
