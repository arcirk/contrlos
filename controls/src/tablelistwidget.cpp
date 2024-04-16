//
// Created by admin on 19.01.2024.
//

// You may need to build the project (run Qt uic code generator) to get "ui_TableListWidget.h" resolved
#include "../include/tablelistwidget.h"
#include "../ui/ui_TableListWidget.h"
#include "../../global/variant/item_data.h"

using namespace arcirk::widgets;

TableListWidget::TableListWidget(QWidget *parent, editor_inner_role role) :
    QWidget(parent),
    ui(new Ui::TableListWidget)
{
    ui->setupUi(this);

    m_role = role;
    m_tool_bar = new TableToolBar(this);
    m_table_widget = new TableWidget(this);
    ui->verticalLayout->addWidget(m_tool_bar);
    ui->verticalLayout->addWidget(m_table_widget);
    m_table_widget->setTableToolBar(m_tool_bar);

    m_model = new TableModel(this);
    json table = json::object();
    auto h_item = header_item_def("value", "Значение");
    h_item.default_type = editorText;
    h_item.default_value = to_byte(to_binary(""));
    h_item.select_type = true;
    table["columns"] = json::array({pre::json::to_json(h_item)});
    table["rows"] = json::array();
    m_model->form_json(table);
    m_model->set_read_only(false);
//    m_model->set_columns_aliases(QMap<QString, QString>{
//            qMakePair("value", "Значение")
//    });
    m_table_widget->setModel(m_model);
    m_table_widget->set_inners_dialogs(false);
    m_table_widget->allow_default_command(false);
    m_table_widget->header_visible(Qt::Horizontal, false);
    m_table_widget->header_visible(Qt::Vertical, false);

    connect(m_table_widget, &TableWidget::toolBarItemClicked, this, &TableListWidget::onToolbarButtonClick);

}

TableListWidget::~TableListWidget() {
    delete ui;
}

json TableListWidget::to_array() const {
    return m_model->to_array("value");
}

json TableListWidget::default_value() const {
    return to_byte(to_binary(""));
}

void TableListWidget::onToolbarButtonClick(const QString &button_name) {
    auto name = json(button_name.toStdString()).get<table_toolbar_buttons>();
    if(name == table_toolbar_buttons::table_add_item){
        json object = json::object();
        object["value"] = default_value();
        auto model = qobject_cast<TableModel*>(m_table_widget->model());
        model->add(object);
    }else if(name == table_toolbar_buttons::table_delete_item){
        m_table_widget->removeRow();
    }else if(name == table_toolbar_buttons::table_move_up_item){
        m_table_widget->moveUp();
    }else if(name == table_toolbar_buttons::table_move_down_item){
        m_table_widget->moveDown();
    }else if(name == table_toolbar_buttons::table_edit_item){
        m_table_widget->editRow();
    }
}

void TableListWidget::close_editor() {
    m_table_widget->close_editor();
}

void TableListWidget::set_array(const arcirk::BJson &data) {

    m_model->clear();
    if(!data.empty()){
        //auto str = byte_array_to_string(data);
        auto js = json::from_cbor(data);
        if(js.is_string()){
            auto str = js.get<std::string>();
            if(json::accept(str)){
                auto arr = json::parse(str);
                if(arr.is_array()){
                    for (auto itr = arr.begin(); itr != arr.end() ; ++itr) {
                        auto row = json::object();
                        row["value"] = *itr;
                        m_model->add(row);
                    }
                }
            }
        }

    }
}
