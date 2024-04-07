#ifndef IS_OS_ANDROID
#include "../include/texteditdialog.h"
#include "../ui/ui_texteditdialog.h"
#include <QPushButton>
#include <QDialogButtonBox>

TextEditDialog::TextEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextEditDialog)
{
    ui->setupUi(this);

    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отмена");
}

TextEditDialog::~TextEditDialog()
{
    delete ui;
}

void TextEditDialog::setText(const QString &text)
{
    ui->plainTextEdit->setPlainText(text);
}

QString TextEditDialog::text() const
{
    return ui->plainTextEdit->toPlainText();
}
#endif
