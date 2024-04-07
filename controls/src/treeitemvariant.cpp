#ifndef IS_OS_ANDROID
#include "../include/treeitemvariant.h"
#include <QMenu>
#include "../include/araydialog.h"
#include <QFileDialog>
#include <QLabel>
#include <QSpinBox>
#include <QToolButton>
#include "../include/treeitemcombobox.h"
#include "../include/texteditdialog.h"

using namespace arcirk::widgets;

TreeItemVariant::TreeItemVariant(int row, int column, QWidget *parent)
    : TreeItemWidget{row, column, parent}
{
    init();
}

TreeItemVariant::TreeItemVariant(QWidget *parent)
    : TreeItemWidget{parent}
{
    init();
}


void TreeItemVariant::init()
{
    createEditor();
}

void TreeItemVariant::setText(const QString &text)
{
    if(m_raw->role() == editorText || m_raw->role() == editorMultiText){
        m_value = text;
        m_raw->set_value(text.toStdString());
    }
    updateControl();
}

QString TreeItemVariant::text() const
{
    return m_raw->representation().c_str();
}

void TreeItemVariant::setRole(editor_inner_role role)
{
    m_raw->set_role(role);
    reset();
    emit roleChanged(row(), column(), role);
}

void TreeItemVariant::setFrame(bool value)
{
//    if(m_frame == value)
//        return;
    m_frame = value;

    QWidget* widget = qobject_cast<QWidget*>(m_current_widget);
    if(!widget)
        return;

    if(widget->objectName() == "label"){
        auto m_label = qobject_cast<QLabel*>(m_current_widget);
        if(!m_label)
            return;

        if(value){
            m_label->setStyleSheet("QLabel {"
                                   "border-style: solid;"
                                   "border-width: 1px;"
                                   "border-color: grey; "
                                   "}");
        }else
            m_label->setStyleSheet("");//"QLabel{background-color: rgba(255, 85, 127, 80);}");

    }else if(widget->objectName() == "TextEdit" || m_current_widget->objectName() == "LineEdit" || m_current_widget->objectName() == "SpinBox"){
        if(!value)
            widget->setStyleSheet("border: 1px  solid rgb(255, 255, 255); border-radius: 0px;");
        else
            widget->setStyleSheet("");
    }

}

void TreeItemVariant::updateControl()
{
    if(!m_current_widget)
        return;

    auto type = m_raw->data()->subtype;
    auto role = (editor_inner_role)m_raw->role();

    if(type == subtypeDump){
        auto jval = m_raw->json_value();
        if(role == editorText){
            if(m_selection_list.empty()){
                auto m_text = qobject_cast<LineEdit*>(m_current_widget);
                if(m_text){
                    if(jval.is_string()){
                        m_text->setText(jval.get<std::string>().c_str());
                    }else{
                        m_text->setText("");
                    }
                }
            }else{
                auto m_combo = qobject_cast<QComboBox*>(m_current_widget);
                if(m_combo){
                    auto model = qobject_cast<PairModel*>(m_combo->model());
                    if(jval.is_string() && model) {
                        QString val = jval.get<std::string>().c_str();
                        auto index = model->findValue(val);
                        if(index.isValid()){
                            m_combo->setCurrentIndex(index.row());
                        }else{
                            m_combo->setCurrentIndex(0);
                        }
                    }else{
                        if(model){
                            if(model->rowCount() > 0){
                                m_combo->setCurrentIndex(0);
                            }
                        }
                    }
                }
            }
        }else if(role == editorMultiText || role == editorDirectoryPath || role == editorFilePath){
            if(!is_table_item){
                auto m_edit = qobject_cast<TextEdit*>(m_current_widget);
                if(m_edit){
                    if(jval.is_string()){
                        m_edit->setPlainText(jval.get<std::string>().c_str());
                    }else{
                        m_edit->setPlainText("");
                    }
                }
            }else{
                auto m_edit = qobject_cast<LineEdit*>(m_current_widget);
                if(m_edit){
                    if(jval.is_string()){
                        m_edit->setText(jval.get<std::string>().c_str());
                    }else{
                        m_edit->setText("");
                    }
                }
            }
        }else if(role == editorArray){
            if(jval.is_array()){
                auto m_label = qobject_cast<QLabel*>(m_current_widget);
                if(m_label){
                    m_label->setText(m_raw->representation().c_str());
                }
            }
        }else if(role == editorNumber){
            int val = 0;
            if(jval.is_number()){
                val = jval.get<int>();
            }
            auto m_integer = qobject_cast<QSpinBox*>(m_current_widget);
            if(m_integer){
                m_integer->setValue(val);
            }
        }else if(role == editorBoolean){
            bool val = false;
            if(jval.is_number()){
                val = (bool)jval.get<int>();
            }else if(jval.is_boolean()){
                val = jval.get<bool>();
            }
            auto m_combo = qobject_cast<QComboBox*>(m_current_widget);
            if(m_combo){
                m_combo->setCurrentIndex(val ? 1 : 0);
            }
        }
    }else if(type == subtypeByte){
        if(role == editorByteArray){
            auto m_label = qobject_cast<QLabel*>(m_current_widget);
            if(m_raw->representation().empty()){
                if(m_label)
                    m_label->setText("<Бинарные данные>");
            }else
                m_label->setText(m_raw->representation().c_str());
        }
    }

}

void TreeItemVariant::createEditor()
{
    clearLayout(m_hbox);
    auto type = m_raw->data()->subtype;
    if(type== subtypeNull){
        m_select_button = false;
        m_erase_button = false;
        m_save_button = false;
        m_select_type_button = m_select_type;
    }else if(type == subtypeByte){
        m_select_button = true;
        m_save_button = true;
        m_select_type_button = m_select_type;
        m_erase_button = true;
    }else{
        m_select_type_button = m_select_type;
        m_erase_button = true;
        m_select_button = true;
        m_save_button = false;
    }

    if(m_raw->role() == editor_inner_role::editorInnerRoleINVALID || m_raw->role() == editor_inner_role::editorNullType){
        m_current_widget = createEditorNull();
    }else if(m_raw->role() == editor_inner_role::editorArray || m_raw->role() == editor_inner_role::editorByteArray){
        m_current_widget = createEditorLabel(m_raw->role() == editor_inner_role::editorByteArray);
    }else if(m_raw->role() == editor_inner_role::editorText){
        if(m_selection_list.empty())
            m_current_widget = createEditorTextLine();
        else{
            m_current_widget = createComboBox();
            setComboData();
        }
    }else if(m_raw->role() == editor_inner_role::editorMultiText
               || m_raw->role() == editor_inner_role::editorDirectoryPath
               || m_raw->role() == editor_inner_role::editorFilePath){
        if(!is_table_item)
            m_current_widget = createEditorTextBox();
        else
            m_current_widget = createEditorTextLine();
    }else if(m_raw->role() == editor_inner_role::editorNumber){
        m_current_widget = createEditorNumber();
    }else if(m_raw->role() == editor_inner_role::editorBoolean){
        m_current_widget = createBooleanBox();
    }else if(m_raw->role() == editor_inner_role::editorDataReference){
        m_current_widget = createEditorLabel(false);
    }else{
        m_raw->set_role(editor_inner_role::editorInnerRoleINVALID);
        m_current_widget = createEditorNull();
    }
    createButtons(m_current_menu);
}

void TreeItemVariant::setControlType()
{
    if(m_raw->role() == editorInnerRoleINVALID || m_raw->role() == editorNullType){
        if(m_raw->json_value().is_string())
            m_raw->set_role(editorText);
        else if(m_raw->json_value().is_number())
            m_raw->set_role(editorNumber);
        else if(m_raw->json_value().is_boolean())
            m_raw->set_role(editorBoolean);
        else if(m_raw->json_value().is_array() && m_raw->data()->subtype == subtypeArray)
            m_raw->set_role(editorArray);
        else if(m_raw->json_value().is_array() && m_raw->data()->subtype == subtypeByte)
            m_raw->set_role(editorByteArray);
        else if(m_raw->json_value().is_array() && m_raw->data()->subtype == subtypeRef)
            m_raw->set_role(editorDataReference);
    }
}

void TreeItemVariant::onMenuItemClicked()
{
    auto mnu = qobject_cast<QAction*>(sender());
    if(mnu){
        json r = mnu->objectName().toStdString();
        auto role = r.get<editor_inner_role>();
        if(m_raw->role() != role){;
            setRole(role);
            updateControl();
            setFrame(m_frame);
        }
    }
}

void TreeItemVariant::onSelectClicked()
{
    Q_ASSERT(m_current_widget!=0);

    if(m_raw->role() == editorArray){
        auto dlg = ArrayDialog(m_raw->data()->data, this);
        if(dlg.exec()){
            auto result = dlg.result();
            m_raw->set_value(result);
//            auto array = verify_array(result);
//            variant_p_set_array(array, m_raw);
//            m_value = QVariant(m_raw.representation.c_str());
            auto m_label = qobject_cast<QLabel*>(m_current_widget);
            if(m_label){
                m_label->setText(m_raw->representation().c_str());
                emit valueChanged(row(), column(), m_value);
            }
        }
    }else if(m_raw->role() == editorByteArray){
        if(is_table_item) {
            emit selectObject(row(), column(), selectReadFile);
        }else{
            auto result = QFileDialog::getOpenFileName(this, "Выбор файла");
            if(!result.isEmpty()){
                QFileInfo info(result);
                ByteArray ba{};
#ifdef Q_OS_WINDOWS
                arcirk::read_file(result.toLocal8Bit().toStdString(), ba);
#else
                arcirk::read_file(result.toStdString(),ba);
#endif
                m_value = QByteArray(reinterpret_cast<const char*>(ba.data()), qsizetype(ba.size()));
                m_raw->set_value(to_byte(to_binary(ba, subtypeByte)));
                m_raw->set_role(editorByteArray);
                auto m_label = qobject_cast<QLabel*>(m_current_widget);
                if(m_label){
                    m_label->setText(info.fileName());
                    emit valueChanged(row(), column(), m_value);
                }
            }
        }
    }else if(m_raw->role()  == editorDirectoryPath){
        if(is_table_item) {
            emit selectObject(row(), column(), selectDirectory);
        }else{
            auto result = QFileDialog::getExistingDirectory(this, "Выбор каталога");
            if(!result.isEmpty()){
                m_value = qvariant_cast<QString>(result);
                auto m_edit = qobject_cast<TextEdit*>(m_current_widget);
                if(m_edit){
                    m_edit->setText(m_value.toString());
                    emit valueChanged(row(), column(), m_value);
                }
            }
        }
    }else if(m_raw->role()  == editorFilePath){
        if(is_table_item) {
            emit selectObject(row(), column(), selectFile);
        }else {
            auto result = QFileDialog::getOpenFileName(this, "Выбор файла");
            if (!result.isEmpty()) {
                m_value = result;
                auto m_edit = qobject_cast<TextEdit *>(m_current_widget);
                if (m_edit) {
                    m_edit->setText(result);
                    emit valueChanged(row(), column(), m_value);
                }
            }
        }
    }else if(m_raw->role()  == editorText || m_raw->role()  == editorMultiText){
        auto dlg = TextEditDialog(this);
        dlg.setText(text());
        if(dlg.exec()){
            setText(dlg.text());
            emit valueChanged(row(), column(), m_value);
        }
    }
}

void TreeItemVariant::onSaveClicked()
{
    if(m_raw->role()  == editorByteArray && m_raw->data()->subtype == subtypeByte){
        if(is_table_item) {
            emit selectObject(row(), column(), selectWriteFile);
        }else{
            auto result = QFileDialog::getSaveFileName(this, "Сохранить как...", m_raw->representation().c_str());
            if(!result.isEmpty()){
                if(!m_raw->data()->data.empty()){
                    //auto j_ba = arcirk::to_json(m_raw.data);
                    ByteArray ba(m_raw->data()->data.size());
                    std::copy(m_raw->data()->data.begin(), m_raw->data()->data.end(), ba.begin());
#ifdef Q_OS_WINDOWS
                    arcirk::write_file(result.toLocal8Bit().toStdString(), ba);
#else
                    arcirk::write_file(result.toStdString(), ba);
#endif
                }
            }
        }
    }
}

void TreeItemVariant::onEraseClicked()
{
    json value = m_raw->json_value();
    if(value.is_string())
        m_raw->set_value("");
    else if(value.is_number())
        m_raw->set_value(0);
    else if(value.is_array())
        m_raw->set_value(json::array());
    else if(value.is_boolean())
        m_raw->set_value(false);
//    setRole(editor_inner_role::editorNullType);
    setFrame(m_frame);
    emit erase(row(), column());
}

void TreeItemVariant::onCheckBoxClicked(bool /*state*/)
{

}

void TreeItemVariant::reset() {
    setControlType();
    createEditor();
    updateControl();
    setFrame(m_frame);
}

void TreeItemVariant::onComboIndexChanged(int index) {

    auto m_combo = qobject_cast<QComboBox*>(m_current_widget);
    if(!m_combo)
        return;

    auto model = qobject_cast<PairModel*>(m_combo->model());
    if(model)
        m_value = model->data(model->index(index, 0), Qt::UserRole);
    else
        m_value = index;
    m_raw->set_value(from_variant(m_value));
    emit valueChanged(row(), column(), m_value);

}

void TreeItemVariant::selectType(bool value) {
    m_select_type = value;
    m_select_type_button = value;
    auto btn = this->findChild<QToolButton*>(enum_synonym(SelectTypeButton).c_str());
    if(btn && m_raw->data()->subtype != subtypeNull)
        btn->setVisible(value);
    btn = this->findChild<QToolButton*>(enum_synonym(EraseButton).c_str());
    if(btn)
        btn->setVisible(value);
}

void TreeItemVariant::setComboData() {
    auto control = qobject_cast<QComboBox*>(m_current_widget);
    if(!control)
        return;
    if(m_selection_list.empty())
        control->clear();

    QByteArray qba = QByteArray(reinterpret_cast<char*>(m_selection_list.data()), (qsizetype)m_selection_list.size());
    QStringList lst = QString(qba).split(",");
    auto model = new PairModel(this);
    model->setContent(lst);
    auto rep = m_raw->representation();
    auto data = m_raw->data()->to_byte();
    control->setModel(model);
    if(!rep.empty())
        m_raw->data()->from_byte(data);
}

#endif

