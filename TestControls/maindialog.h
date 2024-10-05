#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <QDebug>
#include <treeitemtextline.h>
#include <treeitemcombobox.h>
#include <treeitemvariant.h>
//#include <ui/treeviewwidget.h>
#include <tablewidget.h>
#include <treewidget.h>
#include <itable.hpp>
#include <tablelistwidget.h>
#include "../global/variant/item_data.h"

using namespace arcirk::widgets;
//using namespace arcirk::widgets::;

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::widgets), test_table_struct,
    (std::string, string_field)
    (bool, bool_field)
    (int, number_field)
    (arcirk::BJson, list_field)
    (arcirk::BJson, byte_field)
)

QT_BEGIN_NAMESPACE
namespace Ui {
class MainDialog;
}
QT_END_NAMESPACE

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    MainDialog(QWidget *parent = nullptr);
    ~MainDialog();

private:
    Ui::MainDialog *ui;
    TreeItemTextLine * text;
    TreeItemComboBox * combo;
    TreeItemVariant* variant;
    TreeViewWidget* treeView;
    TableWidget* tableView;
    TableListWidget* tableList;

private slots:
    void onCheckBoxClicked(int row, int col, bool state);
    void onComboBoxValueChanged(int row, int col, const QVariant& value);
    void onTableItemClicked(const QModelIndex& index);
};
#endif // MAINDIALOG_H
