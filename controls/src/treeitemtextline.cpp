#include "../include/treeitemtextline.h"
#include "../include/texteditdialog.h"
#include <QHBoxLayout>
#include <QFileDialog>
#include <QLineEdit>
#include <QToolButton>

using namespace arcirk::widgets;

TreeItemTextLine::TreeItemTextLine(int row, int column, QWidget *parent) :
    TreeItemWidget(row, column, parent)
{
    init();
}

TreeItemTextLine::TreeItemTextLine(QWidget *parent) :
    TreeItemWidget(parent)
{
    init();
}

void TreeItemTextLine::init()
{
    m_raw->set_role(editorText);
    m_raw->set_value("");
    m_text_line = qobject_cast<LineEdit*>(createEditorTextLine());
    m_frame = false;
    m_text_line->setStyleSheet("border: 1px  solid rgb(255, 255, 255); border-radius: 0px;");
}

void TreeItemTextLine::onSelectClicked()
{
   if(m_raw->role() == editorText){
        auto dlg = TextEditDialog(this);
        dlg.setText(m_text_line->text());
        if(dlg.exec()){
            m_text_line->setText(dlg.text());
        }
   }else if(m_raw->role() == editorFilePath){
        QString filter;
        if(m_inner_param.isValid()){
            filter = m_inner_param.toString();
        }
        auto result = QFileDialog::getOpenFileName(this, "Выбор файла", m_text_line->text(), filter);
        if(!result.isEmpty()){
            m_text_line->setText(result);
            m_value = result;
            m_raw->set_value(result.toStdString());
            //m_raw.data = to_byte(to_binary(result.toStdString(), subtypeDump));
        }
   }else if(m_raw->role() == editorDirectoryPath){
        auto result = QFileDialog::getExistingDirectory(this, "Выбор каталога", m_text_line->text());
        if(!result.isEmpty()){
            m_text_line->setText(result);
            m_value = result;
            m_raw->set_value(result.toStdString());
            //m_raw.data = to_byte(to_binary(result.toStdString(), subtypeDump));
            //variant_p_set_data(m_value, m_raw, (tree_editor_inner_role) m_raw.role);
        }
   }else if(m_raw->role() == editorColor){

   }
}

void TreeItemTextLine::onEraseClicked()
{
   m_value = "";
   m_text_line->setText("");
   emit valueChanged(row(), column(), m_value);
}

void TreeItemTextLine::setValue(const QVariant &value)
{
    m_value = value.toString();
}

QVariant TreeItemTextLine::value() const
{
    return m_value;
}

void TreeItemTextLine::setText(const QString &text)
{
    if(m_raw->role() == editor_inner_role::editorColor ||
        m_raw->role() == editor_inner_role::editorDirectoryPath ||
        m_raw->role() == editor_inner_role::editorFilePath){

        m_raw->set_value(text.toStdString());
//        m_raw.representation = text.toStdString();
    }
    m_text_line->setText(text);
    m_value = text;
}

QString TreeItemTextLine::text() const
{
    return m_text_line->text();
}

void TreeItemTextLine::setRole(editor_inner_role role)
{
    m_raw->set_role(role) ;
    if(role == editor_inner_role::editorColor ||
        role == editor_inner_role::editorDirectoryPath ||
        role == editor_inner_role::editorFilePath){
        m_raw->set_value("");
//        m_raw.data = {};
//        setButtonVisible(SelectButton, true);
    }
}

void TreeItemTextLine::setFrame(bool value)
{
    m_frame = value;
    if(!value)
        m_text_line->setStyleSheet("border: 1px  solid rgb(255, 255, 255); border-radius: 0px;");
    else
        m_text_line->setStyleSheet("");
}

void TreeItemTextLine::setReadOnly(bool value)
{
    m_text_line->setReadOnly(value);
}

void TreeItemTextLine::setAutoMarkIncomplete(bool value)
{
    m_auto_mark = value;
    m_text_line->setAutoMarkIncomplete(value);
    setFrame(m_frame);
    m_text_line->update();
}
