#ifndef SELECTITEMDIALOG_H
#define SELECTITEMDIALOG_H

#include <QDialog>
#include "treemodel.h"
#include "treewidget.h"

namespace Ui {
    class SelectItemDialog;
    }

namespace arcirk::widgets{

    class CONTROLS_EXPORT SelectItemDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit SelectItemDialog(TreeModel* model, QWidget *parent = nullptr);
        ~SelectItemDialog();

        void accept() override;

        json result();

    private:
        Ui::SelectItemDialog *ui;
        json m_result;
        TreeViewWidget * treeView;
        TreeModel* m_model;
    };
}
#endif // SELECTITEMDIALOG_H
