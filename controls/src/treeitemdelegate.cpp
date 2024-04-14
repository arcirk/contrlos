#ifndef IS_OS_ANDROID
#include "../include/treeconf.h"
#include "../include/treemodel.h"
#include "../include/treeitemdelegate.h"
#include <QComboBox>

#include <QStylePainter>
#include <QApplication>
#include <QStyleOption>
#include <QStyle>
#include <QDebug>
#include <QLineEdit>
#include <QPair>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QMouseEvent>

#include <QRegularExpressionValidator>
#include "../include/treeitemtextline.h"
#include "../include/treeitemcheckbox.h"
#include "../include/treeitemcombobox.h"
#include "../include/treeitemnumber.h"
#include "../include/treeitemvariant.h"

using json = nlohmann::json;
using namespace arcirk::widgets;

TreeItemDelegate::TreeItemDelegate(bool gline, QObject *parent)
    : QStyledItemDelegate{parent}
{
    m_grid_line = gline;
}

QWidget *TreeItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{

//    auto read_only = index.data(tree::ReadOnlyRole);
//    if(read_only.isValid()){
//        if(read_only.toBool())
//            return QStyledItemDelegate::createEditor(parent, option, index);
//    }
//    auto editorType = index.data(tree::WidgetRole);
//    if(editorType.isValid()){
//        item_editor_widget_roles t_editor = (item_editor_widget_roles)editorType.toInt();
//        if(t_editor == widgetComboBoxRole){
//            return new TreeItemComboBox(index.row(), index.column(), parent);
//        }else if(t_editor == widgetTextLineRole)
//            return new TreeItemTextLine(index.row(), index.column(), parent);
//        else if(t_editor == widgetCompareRole)
//            return QStyledItemDelegate::createEditor(parent, option, index);
//        else if(t_editor == widgetCheckBoxRole){
//            return new TreeItemCheckBox(index.row(), index.column(), parent);
//        }
//        else if(t_editor == widgetSpinBoxRole)
//            return new TreeItemNumber(index.row(), index.column(), parent);
//        else if(t_editor == widgetVariantRole){
//            //qDebug() << __FUNCTION__;
//            return new TreeItemVariant(index.row(), index.column(), parent);
//        }else
//            return QStyledItemDelegate::createEditor(parent, option, index);
//    }else
        return QStyledItemDelegate::createEditor(parent, option, index);

}

void TreeItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{

//    //qDebug() << __FUNCTION__ << qMakePair(index.row(), index.column());
//
//    auto model =(TreeItemModel*)index.model();
//    model->set_widget_address(qMakePair(index.row(), index.column()));
//
//    auto editorType = index.data(tree::WidgetRole);
//    auto editorInnerRole = index.data(tree::WidgetInnerRole);
//
//    if(editorType.isValid()){
//        item_editor_widget_roles t_editor = (item_editor_widget_roles)editorType.toInt();//json(editorType.toInt()).get<item_editor_widget_roles>();
//        if(t_editor == widgetComboBoxRole){
//            TreeItemComboBox* comboBox = qobject_cast<TreeItemComboBox*>(editor);
//            if (!comboBox) {
//                return QStyledItemDelegate::setEditorData(editor, index);
//            }
//            auto editorData = index.data(tree::UserRoleExt);
//            if(editorData.isValid()){
//                QStringList lst = editorData.toStringList();
//                comboBox->addItems(lst);
//                QString value = index.data().toString();
//                int current = lst.indexOf(value);
//                if (current > -1) {
//                    comboBox->setCurrentIndex(current);
//                }
//            }
//            if(editorInnerRole.isValid()){
//                comboBox->setRole((editor_inner_role)editorInnerRole.toInt());
//            }
//            connect(comboBox, &TreeItemComboBox::valueChanged, this, &TreeItemDelegate::onComboBoxCurrentIndexChanged);
//        }else if(t_editor == widgetTextLineRole){
//            TreeItemTextLine* edit = qobject_cast<TreeItemTextLine*>(editor);
//            if (!edit) {
//                return QStyledItemDelegate::setEditorData(editor, index);
//            }
//            auto valid = index.data(tree::ValidateTextRole);
//            QString text = index.model()->data(index, Qt::EditRole).toString();
//            edit->setText(text);
//            edit->setButtonVisible(widgets::SelectButton, true);
//            if(valid.isValid()){
//                QRegularExpression rx(valid.toString());
//                QValidator * validator = new QRegularExpressionValidator(rx);
//                int pos;
//                QString s = text;
//                QString style;
//                if (validator->validate(s, pos) != QValidator::Acceptable){
//                    style = "color: red";
//                }else
//                    style = "";
//                edit->setStyleSheet(style);
//
//                delete validator;
//            }
//            if(editorInnerRole.isValid()){
//                edit->setRole((editor_inner_role)editorInnerRole.toInt());
//            }
//        }else if(t_editor == widgetCheckBoxRole){
//            TreeItemCheckBox* checkBox = qobject_cast<TreeItemCheckBox*>(editor);
//            if (!checkBox) {
//                return QStyledItemDelegate::setEditorData(editor, index);
//            }
//            bool value = index.data().toBool();
//            checkBox->setValue(value);
//            if(editorInnerRole.isValid()){
//                checkBox->setRole((editor_inner_role)editorInnerRole.toInt());
//            }
//            connect(checkBox, &TreeItemCheckBox::checkBoxClicked, this, &TreeItemDelegate::onCheckBoxStateChanged);
//        }else if(t_editor == widgetCompareRole){
//            return QStyledItemDelegate::setEditorData(editor, index);
//        }else if(t_editor == widgetSpinBoxRole){
//            TreeItemNumber* spin = qobject_cast<TreeItemNumber*>(editor);
//            if (!spin) {
//                return QStyledItemDelegate::setEditorData(editor, index);
//            }
//            int value = index.data().toInt();
//            if(editorInnerRole.isValid()){
//                spin->setRole((editor_inner_role)editorInnerRole.toInt());
//            }
//            spin->setValue(value);
//        }else if(t_editor == widgetVariantRole){
//            TreeItemVariant* ctrl = qobject_cast<TreeItemVariant*>(editor);
//            if (!ctrl) {
//                return QStyledItemDelegate::setEditorData(editor, index);
//            }
//            ctrl->setData(index.data(tree::RawDataRole));
//        }else
//            return QStyledItemDelegate::setEditorData(editor, index);
//    }else
        return QStyledItemDelegate::setEditorData(editor, index);
}

void TreeItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
//    //qDebug() << __FUNCTION__ << qMakePair(index.row(), index.column());
//    auto model_ = (TreeItemModel*)model;
//    model_->set_widget_address(qMakePair(-1,-1));
//
//    auto editorType = index.data(tree::WidgetRole);
//    if(editorType.isValid()){
//        item_editor_widget_roles t_editor = (item_editor_widget_roles)editorType.toInt();
//        if(t_editor == widgetComboBoxRole){
//            TreeItemComboBox* comboBox = qobject_cast<TreeItemComboBox*>(editor);
//            if (comboBox) {
//                model->setData(index, comboBox->text());
//                disconnect(comboBox);
//            }else
//                return QStyledItemDelegate::setModelData(editor, model, index);
//        }else if(t_editor == widgetTextLineRole){
//            TreeItemTextLine* edit = qobject_cast<TreeItemTextLine*>(editor);
//            if (!edit) {
//                return QStyledItemDelegate::setModelData(editor, model, index);
//            }
//            auto valid = index.data(tree::ValidateTextRole);
//
//            QString text = edit->text();
//            model->setData(index, text);
//            if(valid.isValid()){
//                QRegularExpression rx(valid.toString());
//                QValidator * validator = new QRegularExpressionValidator(rx);
//                int pos;
//                QString s = text;
//                if (validator->validate(s, pos) != QValidator::Acceptable){
//                    model->setData(index, QColor(Qt::red), tree::TextColorRole);
//                }else
//                    model->setData(index, QVariant(), tree::TextColorRole);
//                delete validator;
//            }
//            disconnect(edit);
//        }else if(t_editor == widgetCheckBoxRole){
//            TreeItemCheckBox* checkBox = qobject_cast<TreeItemCheckBox*>(editor);
//            if (!checkBox) {
//                return QStyledItemDelegate::setModelData(editor, model, index);
//            }
//            model->setData(index, checkBox->value());
//            disconnect(checkBox);
//        }else if(t_editor == widgetCompareRole){
////            CompareWidget* compare = qobject_cast<CompareWidget*>(editor);
////            if (!compare) {
////                return QStyledItemDelegate::setModelData(editor, model, index);
////            }
////            model->setData(index, compare->text());
//        }else if(t_editor == widgetSpinBoxRole){
//            TreeItemNumber* spin = qobject_cast<TreeItemNumber*>(editor);
//            if (!spin) {
//                return QStyledItemDelegate::setModelData(editor, model, index);
//            }
//            //qDebug() << __FUNCTION__ << "widgetSpinBoxRole" << index.row() << index.column();
//            model->setData(index, spin->value());
//        }else if(t_editor == widgetVariantRole){
//            TreeItemVariant* ctrl = qobject_cast<TreeItemVariant*>(editor);
//            if (!ctrl) {
//                return QStyledItemDelegate::setModelData(editor, model, index);
//            }
//            //model_->setData(index, ctrl->currentState(), tree::WidgetStateRole);
//            model_->setData(index, ctrl->data());
//        }else
//            return QStyledItemDelegate::setModelData(editor, model, index);
//    }else
        return QStyledItemDelegate::setModelData(editor, model, index);

}

void TreeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::paint(painter, option, index);
//    QStyle* style = qApp->style();
//    QStyleOptionViewItem opt = option;
//    initStyleOption(&opt, index);
//    //QRect focusRect = style->subElementRect(QStyle::SE_ItemViewItemFocusRect, &opt);
//    QRect decRect = style->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt);
//    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt);
//    QRect fillRect = opt.rect;
//    fillRect.setLeft(fillRect.left() - 1);
//    if(m_grid_line){
//        textRect.moveLeft(textRect.left() + 4);
//        textRect.setWidth(textRect.width() -  8);
//    }
//    else{
//        textRect.moveLeft(textRect.left() + 2);
//        textRect.setWidth(textRect.width() -  2);
//    }
//    QColor foreColor = index.data(Qt::ForegroundRole).value<QColor>(); //цвет текста всей строки
//    QIcon itemIcon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
//    auto value = index.data(Qt::DisplayRole);
//    auto _v = index.data(tree::RowCountRole);
//    bool row_count = false;
//    if(_v.isValid())
//        row_count = _v.toBool();
//    item_editor_widget_roles t_editor = item_editor_widget_roles::editorINVALID;
//    auto editorType = index.data(tree::WidgetRole);
//
//    if(editorType.isValid()){
//        t_editor = (item_editor_widget_roles)editorType.toInt();
//    }
//
//    if(row_count)
//        value = index.row() + 1;
//
//    QColor textColor = index.data(tree::TextColorRole).value<QColor>(); //цвет текста текущей ячейки
//
//    //qDebug() << opt.state << index.row();
//
//    if (opt.state & QStyle::State_Selected &&
//        opt.state & QStyle::State_Active){
//
//        if (foreColor.isValid())
//        {
//            painter->setPen(foreColor);
//        }else
//            painter->setPen(QColor(Qt::white));//opt.palette.color(QPalette::HighlightedText)
//
//
//        //        if(t_editor == widgetCompareCompare){
//        //                qDebug() << opt.state << index.column();
//        //           painter->setBrush(QBrush(Qt::white));
//        //        }else
//        painter->setBrush(opt.palette.highlight());
//    }else{
//        if (foreColor.isValid())
//        {
//            painter->setPen(foreColor);
//        }else
//            painter->setPen(opt.palette.color(QPalette::Text));
//        painter->setBrush(QBrush(Qt::white));// opt.palette.window());
//    }
//
//    if(t_editor != widgetComboBoxRole){
//        painter->fillRect(opt.rect, painter->brush());
//    }else{
//        auto model = (TreeItemModel*)index.model();
//        auto active_widget = model->widget_address();
//        auto p_item = qMakePair(index.row(), index.column());
//        if(active_widget != p_item){
//            painter->fillRect(opt.rect, painter->brush());
//        }else{
//            if(opt.state & QStyle::State_HasFocus){
//                painter->fillRect(opt.rect, painter->brush());
//            }else{
//                painter->save();
//                painter->setBrush(QBrush(Qt::white));
//                painter->fillRect(opt.rect, painter->brush());
//                //qDebug() << "test" << opt.state;
//            }
//        }
//    }
//    painter->save();
//    if(m_grid_line){
//
//        //QRect rc = opt;
//        painter->setPen(QColor(Qt::gray));
//        painter->drawRect(fillRect);
//        painter->restore();
//    }
//
//    if(textColor.isValid()){
//        painter->setPen(textColor);
//    }
//
//
//    //    if(editorType.isValid()){
//
//    //        t_editor = (item_editor_widget_roles)editorType.toInt();//json(editorType.toInt()).get<item_editor_widget_roles>();
//
//    const QWidget *widget = option.widget;
//    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, widget);
//
//    if(t_editor == widgetComboBoxRole){
//        //
//    }else if (t_editor == widgetCheckBoxRole) {
//        QStyleOptionButton chk;
//        QRect checkbox_rect = style->subElementRect(QStyle::SE_CheckBoxIndicator, &opt);
//        checkbox_rect.moveCenter(option.rect.center());
//        chk.rect = checkbox_rect;
//        auto b = value.toBool();
//        chk.state |= b ? QStyle::State_On : QStyle::State_Off;
//        chk.state |= QStyle::State_Enabled;
//        //           chk.text = text;
//        //chk.rect = option.rect;
//        //style->drawComplexControl(QStyle::CC_ComboBox, &opt, painter, 0);
//        style->drawControl(QStyle::CE_CheckBox, &chk, painter, 0);
//
//
////        auto rect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, widget);
////        rect.moveCenter(option.rect.center());
////        //opt.rect = QStyle::alignedRect(opt.direction, index.data(Qt::TextAlignmentRole).value<Qt::Alignment>(), rect.size(), opt.rect);
////        opt.rect = QStyle::alignedRect(opt.direction, Qt::AlignCenter, rect.size(), opt.rect);
////        auto b = value.toBool();
////        opt.state |= b ? QStyle::State_On : QStyle::State_Off;
////        opt.state |= QStyle::State_Enabled;
////        //opt.state = opt.state & ~QStyle::State_HasFocus;
////        style->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &opt, painter, widget);
//    }
//    //}
//
//    //if(t_editor != widgetComboBoxRole && t_editor != widgetCheckBoxRole){
//    if(t_editor != widgetCheckBoxRole){
//
//        if(value.typeId() != QMetaType::Bool){
//            if(value.typeId() == QMetaType::Int || value.typeId() == QMetaType::Double)
//                painter->drawText(textRect,  Qt::AlignVCenter | Qt::AlignRight, value.toString());
//            else{
//                QString str = value.toString();
//                QFontMetrics fm(painter->fontMetrics());
//                QRect t_rect = fm.boundingRect(str);
//                //int pixelsWide = fm.horizontalAdvance(str);
//                if(t_rect.width() > textRect.width()){
//                    auto w = t_rect.width();
//                    if(w < 1000 || str.indexOf("n/") != -1){
//                        while (t_rect.width() > textRect.width() && str.length() > 0) {
//                            str = str.left(str.length() - 1);
//                            t_rect = fm.boundingRect(str);
//                        }
//                        if(str.length() > 3){
//                            str = str.left(str.length()-3) + "...";
//                        }else
//                            str = "...";
//                    }else{
//                        str = "...";
//                    }
//                }
//                painter->drawText(textRect, opt.displayAlignment, str);
//            }
//        }
//        if(!itemIcon.isNull()){
//            const QRect r = decRect; //option.rect;
//            QPixmap pix = itemIcon.pixmap(r.size());
//            if(value.typeId() != QMetaType::Bool){
//                const QPoint p = QPoint(0, (r.height() - pix.height())/2);
//
//                painter->drawPixmap(r.topLeft() + p, pix);
//                //painter->drawPixmap(decRect, pix);
//            }else{
//                const QPoint p = QPoint((r.width() - pix.width())/2, (r.height() - pix.height())/2);
//                painter->drawPixmap(r.topLeft() + p, pix);
//            }
//        }else{
//
//        }
//    }

}

QSize TreeItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

void TreeItemDelegate::onCheckBoxStateChanged(int row, int col, bool state)
{
    //qDebug() << row << col << state;
    emit checkBoxStateChanged(row, col, state);
}

void TreeItemDelegate::onComboBoxCurrentIndexChanged(int row, int column, const QVariant& value)
{
//    //qDebug() << __FUNCTION__ << index;
    auto comboBox = qobject_cast<TreeItemComboBox*>(sender());
    if (!comboBox) {
        return;
    }
//    int row = comboBox->property("row").toInt();
//    int col = comboBox->property("col").toInt();

   // emit selectedItemComboBoxChanged(row, column, comboBox->text(), index);
}

void TreeItemDelegate::onTextLineButtonClicked(int row, int col, const QString &text)
{
    //qDebug() << text;
}
#endif
