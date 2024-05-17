//
// Created by admin on 16.05.2024.
//
#include "../include/treeitemipaddress.h"
#include <QHBoxLayout>
#include <QFileDialog>
#include <QLineEdit>
#include <QToolButton>
#include <QRegularExpression>
#include <QHostAddress>

using namespace arcirk::widgets;

TreeItemIPEdit::TreeItemIPEdit(int row, int column, QWidget *parent) :
        TreeItemWidget(row, column, parent)
{
    init();
    connect(m_text_line, &LineEdit::textChanged, this, &TreeItemIPEdit::onTextChanged);
}

TreeItemIPEdit::TreeItemIPEdit(QWidget *parent) :
        TreeItemWidget(parent)
{
    init();
    connect(m_text_line, &LineEdit::textChanged, this, &TreeItemIPEdit::onTextChanged);
}

void TreeItemIPEdit::init()
{
    m_raw->set_role(editorIpAddress);
    m_raw->set_value("0.0.0.0");
    m_text_line = qobject_cast<LineEdit*>(createEditorTextLine());
    m_frame = false;
    m_text_line->setStyleSheet("border: 1px  solid rgb(255, 255, 255); border-radius: 0px;");
}

void TreeItemIPEdit::onSelectClicked()
{
    //
}

void TreeItemIPEdit::onEraseClicked()
{
    m_value = "";
    m_text_line->setText("");
    emit valueChanged(row(), column(), m_value);
}

void TreeItemIPEdit::setValue(const QVariant &value)
{
    m_value = value.toString();
}

QVariant TreeItemIPEdit::value() const
{
    return m_value;
}

void TreeItemIPEdit::setText(const QString &text)
{
    m_text_line->setText(text);
    m_value = text;
}

QString TreeItemIPEdit::text() const
{
    return m_text_line->text();
}

void TreeItemIPEdit::setRole(editor_inner_role role)
{
    m_raw->set_role(role) ;
}

void TreeItemIPEdit::setFrame(bool value)
{
    m_frame = value;
    QString textColor = isValid() ? "" : "color: rgb(255, 0, 0);";
    if(!value)
        m_text_line->setStyleSheet(QString("border: 1px  solid rgb(255, 255, 255); border-radius: 0px; %1").arg(textColor));
    else
        m_text_line->setStyleSheet(QString("%1").arg(textColor));
}

void TreeItemIPEdit::setReadOnly(bool value)
{
    m_text_line->setReadOnly(value);
}

void TreeItemIPEdit::setAutoMarkIncomplete(bool value)
{
    m_auto_mark = value;
    m_text_line->setAutoMarkIncomplete(value);
    setFrame(m_frame);
    m_text_line->update();
}

bool TreeItemIPEdit::ipValidator(QString ip)
{
    static QRegularExpression ipPattern("^((25[0-5]|2[0-4][0-9]|[01]?[0-9]{1,2})\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9]{1,2})$");
    QRegularExpressionMatch match = ipPattern.match(ip);
    if(!match.hasMatch())
        return false;

    QStringList parts = ip.split('.');

    for (int i = 0; i < parts.size(); ++i)
    {
        const QString &part = parts.at(i);
        bool ok;
        int num = part.toInt(&ok);
        if (!ok || num < 0 || num > 255 || (part.length() > 1 && part.startsWith("0") && !part.startsWith("0.")))
            return false;
    }

    //if you wish, you could end immediately simply here
    //return true;


    //If you want to add one more control at the end you can add this
    QHostAddress ipAddress(ip);
    if(ipAddress.protocol() == QAbstractSocket::IPv4Protocol)
        return true;
    else
        return false;
}

void TreeItemIPEdit::onTextChanged(const QString &text) {
    _isValid = ipValidator(text);
    setFrame(m_frame);
}

bool TreeItemIPEdit::isValid() {
    return _isValid;
}
