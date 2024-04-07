#ifndef IS_OS_ANDROID
#include "../include/treeitemtextedit.h"
#include "../include/texteditdialog.h"
#include <QHBoxLayout>
#include <QFileDialog>

using namespace arcirk::widgets;


TreeItemTextEdit::TreeItemTextEdit(int row, int column, QWidget *parent) :
    TreeItemWidget(row, column, parent)
{
    init();
}

TreeItemTextEdit::TreeItemTextEdit(QWidget *parent) :
    TreeItemWidget(parent)
{
    init();
}

void TreeItemTextEdit::init()
{
    m_raw->set_value("");
    m_raw->set_role(editorMultiText);
    m_text_line = qobject_cast<TextEdit*>(createEditorTextBox());
    m_frame = false;
    m_text_line->setStyleSheet("border: 1px  solid rgb(255, 255, 255); border-radius: 0px;");
}

void TreeItemTextEdit::onSelectClicked()
{
   if(m_raw->role() == editorText){
        auto dlg = TextEditDialog(this);
        dlg.setText(m_text_line->toPlainText());
        if(dlg.exec()){
            m_text_line->setText(dlg.text());
        }
   }else if(m_raw->role() == editorFilePath){
        QString filter;
        if(m_inner_param.isValid()){
           filter = m_inner_param.toString();
        }
        auto result = QFileDialog::getOpenFileName(this, "Выбор файла", m_text_line->toPlainText(), filter);
        if(!result.isEmpty()){
            m_text_line->setText(result);
            m_value = result;
            m_text_line->textChanged();
        }
   }else if(m_raw->role() == editorDirectoryPath){
        auto result = QFileDialog::getExistingDirectory(this, "Выбор каталога", m_text_line->toPlainText());
        if(!result.isEmpty()){
            m_text_line->setText(result);
            m_value = result;
        }
   }else if(m_raw->role() == editorColor){

   }
}

void TreeItemTextEdit::onEraseClicked()
{
   m_value = "";
   m_text_line->setText("");
   emit valueChanged(row(), column(), m_value);
}

void TreeItemTextEdit::setValue(const QVariant &value)
{
    m_value = value.toString();
}

QVariant TreeItemTextEdit::value() const
{
    return m_value;
}

void TreeItemTextEdit::setText(const QString &text)
{
    if(m_raw->role() == editor_inner_role::editorColor ||
       m_raw->role() == editor_inner_role::editorDirectoryPath ||
       m_raw->role() == editor_inner_role::editorFilePath){
        m_raw->set_value(text.toStdString());
//        m_raw.data = {};
//        m_raw.representation = text.toStdString();
    }
    m_text_line->setText(text);
    m_value = text;
}

QString TreeItemTextEdit::text() const
{
    return m_text_line->toPlainText();
}

void TreeItemTextEdit::setRole(editor_inner_role role)
{
    m_raw->set_role(role);
    if(role == editor_inner_role::editorColor ||
        role == editor_inner_role::editorDirectoryPath ||
        role == editor_inner_role::editorFilePath){
        m_raw->set_value("");
        setButtonVisible(SelectButton, true);
    }
}

void TreeItemTextEdit::setFrame(bool value)
{
    m_frame = value;
    if(!value)
        m_text_line->setStyleSheet("border: 1px  solid rgb(255, 255, 255); border-radius: 0px;");
    else
        m_text_line->setStyleSheet("");
}

void TreeItemTextEdit::setReadOnly(bool value)
{
    m_text_line->setReadOnly(value);
}

void TreeItemTextEdit::setAutoMarkIncomplete(bool value)
{
    m_auto_mark = value;
    m_text_line->setAutoMarkIncomplete(value);
    setFrame(m_frame);
    m_text_line->update();
}

#endif
