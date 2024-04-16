#ifndef QUERYBUILDERFIELDEXDIALOG_H
#define QUERYBUILDERFIELDEXDIALOG_H


#include <QDialog>
#include <tablewidget.h>
#include <QMenu>
#include <QAction>
#include <itable.hpp>
#include "sql/metadata.hpp"
//#include "sql/query_builder.hpp"

using namespace arcirk::database;
using namespace arcirk::database::builder;
using namespace arcirk::widgets;

namespace Ui {
class QueryBuilderFieldExDialog;
}

namespace arcirk::query_builder_ui {
class QueryBuilderFieldExDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QueryBuilderFieldExDialog(QWidget *parent = nullptr, const QUuid& m_package_uuid = {});
    ~QueryBuilderFieldExDialog();

    void set_database_structure(ITable<ibase_object_structure>* structure);

    ibase_object_structure getField();
    void setData(const ibase_object_structure& value);

private:
    Ui::QueryBuilderFieldExDialog *ui;
    TableWidget* m_table;
    QMenu* mnu_index_type;
    QAction * m_compare;
    QAction * m_ext;
    QAction * m_check;
    ITable<ibase_object_structure>* m_structure;
    QStringList m_tables;
    QUuid m_package_uuid;
    ibase_object_structure m_field;


    std::string get_query(const sql_foreign_key& value);

private slots:
    void onMenuClicked();   
    void onBtnEditClicked();
    void onBtnDeleteClicked();

signals:

};
}
#endif // QUERYBUILDERFIELDEXDIALOG_H
