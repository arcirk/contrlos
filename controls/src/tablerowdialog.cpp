#ifndef IS_OS_ANDROID
#include "../include/tablerowdialog.h"
#include "../ui/ui_tablerowdialog.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <alpaca/alpaca.h>


using namespace arcirk::widgets;

TableRowDialog::TableRowDialog(const json& data, TableConf* conf, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TableRowDialog)
{
    ui->setupUi(this);

    m_conf = conf;
    m_data = data;

    if(is_object_empty(m_data)){
        setWindowTitle("Новая строка");
        if(!m_data.is_object())
            m_data = json::object();
        //auto var = to_variant_p(to_binary(QUuid::createUuid()));
        m_data["ref"] = arcirk::to_byte(arcirk::to_binary(QUuid::createUuid()));//serialize(var);
    }else
        setWindowTitle("Изменение строки");


    createControls();

    auto btn = ui->buttonBox->button(QDialogButtonBox::Cancel);
    if(btn)
        btn->setText("Отмена");

    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &TableRowDialog::onButtonBoxClicked);

    hideNotPublicControls();
}

TableRowDialog::~TableRowDialog()
{
    delete ui;
}

json TableRowDialog::data()
{
    return m_data;
}

void TableRowDialog::accept()
{
    bool cancel = false;
    QGridLayout * layout = ui->gridLayout;

    for (int i = 0; i < m_data.size(); ++i) {
        auto w =  layout->itemAtPosition(i,1)->widget();
        if(w){
            auto control = qobject_cast<TreeItemVariant*>(w);
            if(control->autoMarkIncomplete() && control->text().isEmpty()){
                cancel = true;
                QMessageBox::critical(this, "Ошибка заполнения", QString("Не заполнено поле %1").arg(m_conf->column_name(m_conf->column_index(control->objectName()), true)));
            }
            m_data[control->objectName().toStdString()] = control->to_json();
        }
    }

    if(!cancel)
        return QDialog::accept();
}

void TableRowDialog::createControls() {

    if(!m_data.is_object())
        return;

    int row = 0;
    for (auto itr = m_data.items().begin(); itr != m_data.items().end(); ++itr) {
        auto control = createEditor(itr.key());
        //control.second->selectType(m_conf->columns()[m_conf->column_index(itr.key().c_str())].select_type);
        setEditorData(control.first, control.second, itr.value());
        auto lbl = new QLabel(this);
        auto h_data = find_element_for_name(control.first.toStdString(), m_conf->columns());
        if(h_data != m_conf->columns().end()){
            if(h_data->alias.empty())
                lbl->setText(control.first + ":");
            else
                lbl->setText(QString(h_data->alias.c_str()) + ":");
            control.second->selectType(h_data->select_type);
        }else
            lbl->setText(control.first + ":");
        ui->gridLayout->addWidget(lbl, row, 0);
        ui->gridLayout->addWidget(control.second, row, 1);
        if(h_data != m_conf->columns().end()) {
            if (h_data->default_type == editorDirectoryPath || h_data->default_type == editorFilePath || h_data->default_type == editorMultiText) {
                ui->gridLayout->setAlignment(lbl, Qt::AlignLeft | Qt::AlignTop);
            } else {
                ui->gridLayout->setAlignment(lbl, Qt::AlignLeft | Qt::AlignCenter);
            }
        }
        row++;
    }
}

QPair<QString, TreeItemVariant *> TableRowDialog::createEditor(const std::string& key) {
    auto control = new TreeItemVariant(this);
    control->setObjectName(key.c_str());
    auto itr = find_element_for_name(key, m_conf->columns());
    if(itr != m_conf->columns().end()){
        control->setSelectionList(itr->selection_list);
    }
    return qMakePair(key.c_str(), control);
}

void TableRowDialog::setEditorData(const QString &key, TreeItemVariant* control, const json &value) {

    if(value.empty()){
        if(key == "ref"){
            control->setEnabled(false);
        }
        control->selectType(true);
    }else{
        if(key == "ref"){
            control->setEnabled(false);
        }
    }
    auto raw = arcirk::widgets::item_data(value);
    control->setData(raw.to_byte());
    control->setFrame(true);
    auto itr = find_element_for_name(key.toStdString(), m_conf->columns());
    if(itr != m_conf->columns().end()){
        control->setRole((editor_inner_role)itr->default_type);
    }

}

json TableRowDialog::result() const {
    return m_data;
}

void TableRowDialog::onButtonBoxClicked(QAbstractButton *button) {
    if(button == ui->buttonBox->button(QDialogButtonBox::Ok))
        accept();
    else
        this->close();
}

void TableRowDialog::hideNotPublicControls() {

    QGridLayout * layout = ui->gridLayout;

    for (int i = 0; i < m_data.size(); ++i) {
        auto w =  layout->itemAtPosition(i,1)->widget();
        if(w){
            auto control = qobject_cast<TreeItemVariant*>(w);
            if(control){
                if(control->objectName() == "ref"){
                    control->setVisible(false);
                    auto lbl = layout->itemAtPosition(i,0)->widget();
                    if(lbl){
                        auto lbl_control = qobject_cast<QLabel*>(lbl);
                        if(lbl_control)
                            lbl_control->setVisible(false);
                    }
                }else{
                    auto itr = find_element_for_name(control->objectName().toStdString(), m_conf->columns());
                    if(itr != m_conf->columns().end()){
                        if(itr->not_public){
                            control->setVisible(false);
                            auto lbl = layout->itemAtPosition(i,0)->widget();
                            if(lbl){
                                auto lbl_control = qobject_cast<QLabel*>(lbl);
                                if(lbl_control)
                                    lbl_control->setVisible(false);
                            }
                        }
                    }
                }
            }
        }
    }

}

bool TableRowDialog::is_object_empty(const json &object) {

    if(!object.is_object())
        return true;

    if(object.find("ref") == object.end())
        return true;

    if(object["ref"].empty())
        return true;

    auto ref = item_data(object["ref"]);
    auto uuid = QUuid::fromRfc4122(ref.data()->data);
    return uuid.toString(QUuid::WithoutBraces) == NIL_STRING_UUID;

}


#endif
