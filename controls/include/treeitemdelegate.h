#ifndef TREEITEMDELEGATE_H
#define TREEITEMDELEGATE_H

#ifndef IS_OS_ANDROID
#include <QStyledItemDelegate>
#include "../controls_global.h"

namespace arcirk::widgets {

class CONTROLS_EXPORT TreeItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TreeItemDelegate(bool gline = false, QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    void setGridLine(bool value){m_grid_line = value;}

private:
    bool m_grid_line;
    QPair<int,int> m_current_index;

protected:
    //bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private slots:
    void onCheckBoxStateChanged(int row, int col, bool state);
    void onComboBoxCurrentIndexChanged(int row, int column, const QVariant& value);
    void onTextLineButtonClicked(int row, int col, const QString& text);
signals:
    void checkBoxStateChanged(int row, int col, bool state);
    void selectedItemComboBoxChanged(int row, int col, const QString& value, int index);
};
}

#endif
#endif // TREEITEMDELEGATE_H
