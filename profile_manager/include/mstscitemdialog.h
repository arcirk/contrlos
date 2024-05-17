//
// Created by admin on 16.05.2024.
//

#ifndef CONTROLSPROG_MSTSCITEMDIALOG_H
#define CONTROLSPROG_MSTSCITEMDIALOG_H

#include <QDialog>
#include <treeitemipaddress.h>
#include "global.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MstscItemDialog; }
QT_END_NAMESPACE

namespace arcirk::widgets {
    class MstscItemDialog : public QDialog {
    Q_OBJECT

    public:
        explicit MstscItemDialog(arcirk::database::mstsc_item& item, QWidget *parent = nullptr);

        ~MstscItemDialog() override;

        void accept() override;

    private:
        Ui::MstscItemDialog *ui;
        TreeItemIPEdit* txtAddress;
        arcirk::database::mstsc_item& _item;
        QVector<QPair<int,int>> screens;
        QMenu *contextMenu;

        void init();
        void createContextMenu();
        void formControl();

    private slots:
        void onChkDefaultPortToggled(bool checked);
        void onBtnSelectHostClicked();
        void onContextMenuTriggered();
        void onChkWindowModeToggled(bool checked);

    signals:
        void selectHost();
    };

}
#endif //CONTROLSPROG_MSTSCITEMDIALOG_H
