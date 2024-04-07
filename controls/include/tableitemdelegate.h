//
// Created by admin on 14.01.2024.
//

#ifndef CONTROLSPROG_TABLEITEMDELEGATE_H
#define CONTROLSPROG_TABLEITEMDELEGATE_H

#include "../controls_global.h"
#include <QStyledItemDelegate>

namespace arcirk::widgets {

    class CONTROLS_EXPORT TableItemDelegate : public QStyledItemDelegate
    {
            Q_OBJECT
        public:
            explicit TableItemDelegate(QObject *parent = nullptr);
            QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
            void setEditorData(QWidget *editor, const QModelIndex &index) const override;
            void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
            void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
            QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const override;

            void setParent(QWidget* parent);

        private:
            QPair<int,int> m_current_index;
            QWidget* m_parent;
        private slots:
            void onItemValueChanged(int row, int col, const QVariant& value);
            void onSelectObject(int row, int col, const table_command& type);
            void onEndEdit(int row, int col);
            void onStartEdit(int row, int col);

        signals:
            void selectValue(int row, int col, const table_command& type);
            void startEdit(int row, int col) const;
            void endEdit(int row, int col) const;
    };
}
#endif //CONTROLSPROG_TABLEITEMDELEGATE_H
