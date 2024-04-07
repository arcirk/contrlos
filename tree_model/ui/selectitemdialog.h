#ifndef SELECTITEMDIALOG_H
#define SELECTITEMDIALOG_H

#include <QDialog>
#include "../treeitemmodel.h"
#include "treeviewwidget.h"

namespace Ui {
    class TREE_MODEL_EXPORT SelectItemDialog;
    }
    namespace arcirk::tree_widget{
    class SelectItemDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit SelectItemDialog(tree_model::TreeItemModel* model, QWidget *parent = nullptr);
        ~SelectItemDialog();

        void accept() override;

        json result();

    private:
        Ui::SelectItemDialog *ui;
        json m_result;
        arcirk::tree_widget::TreeViewWidget * treeView;
        tree_model::TreeItemModel* m_model;
    };
}
#endif // SELECTITEMDIALOG_H
