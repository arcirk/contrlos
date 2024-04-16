#ifndef QUERYBUILDERFOREGINKEYDIALOG_H
#define QUERYBUILDERFOREGINKEYDIALOG_H

#include <QDialog>
#include <itree.hpp>
#include "sql/metadata.hpp"
#include <treewidget.h>

using namespace arcirk::database;
using namespace arcirk::widgets;
using namespace arcirk::database::builder;

namespace Ui {
class QueryBuilderForeginKeyDialog;
}
namespace arcirk::query_builder_ui {
class QueryBuilderForeginKeyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QueryBuilderForeginKeyDialog(QWidget *parent = nullptr);
    ~QueryBuilderForeginKeyDialog();

    void set_database_structure(ITree<ibase_object_structure>* structure);

    void accept() override;

    sql_foreign_key getResult() const{return m_result;};

    void set_object(const sql_foreign_key& object);

private:
    Ui::QueryBuilderForeginKeyDialog *ui;
    ITree<ibase_object_structure>* m_structure;
    QStringList m_tables;
    QStringList m_fields;
    QStringList m_events{"NO ACTION", "SET NULL", "SET DEFAULT", "CASCADE", "RESTRUCT"};
    QStringList m_match_events{"SIMPLE", "FULL", "PARTIAL"};
    QStringList m_deferrable{"", "DEFERRABLE", "NOT DEFERRABLE"};
    QStringList m_deferred{"", "DEFERRED", "IMMEDIATE"};
    sql_foreign_key m_result;


    void form_control();

public slots:

private slots:
    void onCmbTablesCurrentTextChanged(const QString &arg1);
    void onCmbSelectedItem(bool check);
};
}
#endif // QUERYBUILDERFOREGINKEYDIALOG_H
