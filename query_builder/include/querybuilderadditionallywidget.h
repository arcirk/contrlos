#ifndef QUERYBUILDERADDITIONALLYWIDGET_H
#define QUERYBUILDERADDITIONALLYWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QSpinBox>
#include <QRadioButton>
#include <QUuid>
#include "sql/metadata.hpp"
#include <tablewidget.h>
#include <tabletoolbar.h>
#include <itable.hpp>
#include <QLabel>

using namespace arcirk::database;
using namespace arcirk::database::builder;
using namespace arcirk::widgets;

typedef ITable<arcirk::database::ibase_object_structure> ITableIBaseModel;

namespace Ui {
class QueryBuilderAdditionallyWidget;
}

namespace arcirk::query_builder_ui {
    class QueryBuilderAdditionallyWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit QueryBuilderAdditionallyWidget(QWidget *parent = nullptr, const QUuid& m_package_uuid = {});
        ~QueryBuilderAdditionallyWidget();

        void save_to_database();

        void set_database_structure(ITree<ibase_object_structure>* meta){m_structure = meta;};

    private slots:
        void onChkLimitToggled(bool checked);
        void onRatioClicked();
        void onToolBarFieldItemClicked(const QString& buttonName);
        void onToolBarIndexItemClicked(const QString& buttonName);

    private:
        Ui::QueryBuilderAdditionallyWidget *ui;
        QUuid m_package_uuid;
        int m_current_type;
        QMap<int, QRadioButton*> m_radio;
        TableWidget* m_table;
        TableWidget* m_table_indexes;
//        TreeViewWidget* treeView;
//        TreeViewWidget* treeViewIndexes;
        TableToolBar* m_toolBarField;
        TableToolBar* m_toolBarIndices;
        QLabel* lblTop;
        QLabel* lblBottom;
        ITree<ibase_object_structure>* m_structure;

        void form_control(QWidget* btn);
        ITableIBaseModel* create_model(const QString& table_name);
        void hide_columns(TableWidget* tree);
        void onBtnAddField();
        void onBtnEditField();

    signals:
        void selectedQueryType(sql_global_query_type index);
    };
}
#endif // QUERYBUILDERADDITIONALLYWIDGET_H
