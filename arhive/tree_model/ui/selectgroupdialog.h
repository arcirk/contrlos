#ifndef SELECTGROUPDIALOG_H
#define SELECTGROUPDIALOG_H

#ifndef IS_OS_ANDROID

#include <QDialog>
#include "../treeitemmodel.h"
#include "treeviewwidget.h"

namespace Ui {
class TREE_MODEL_EXPORT SelectGroupDialog;
}
namespace arcirk::tree_widget {
    class SelectGroupDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit SelectGroupDialog(arcirk::tree_model::TreeItemModel * model, QWidget *parent = nullptr);
        ~SelectGroupDialog();

        void accept() override;

        json result() const;

    private:
        Ui::SelectGroupDialog *ui;
        json m_result;
        arcirk::tree_widget::TreeViewWidget * treeView;
    };
}
#endif
#endif // SELECTGROUPDIALOG_H
