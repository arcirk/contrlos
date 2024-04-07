#ifndef TREEITEMTEXTEDIT_H
#define TREEITEMTEXTEDIT_H

#include "treeitemwidget.h"
#include <QToolButton>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QPainter>
#include <QPaintEngine>

namespace arcirk::widgets {

    class CONTROLS_EXPORT TreeItemTextEdit : public TreeItemWidget
    {
        Q_OBJECT
    public:
        explicit TreeItemTextEdit(int row, int column, QWidget *parent = nullptr);
        explicit TreeItemTextEdit(QWidget *parent = nullptr);

        void setText(const QString& text) override;
        QString text() const override;
        void setRole(editor_inner_role role) override;

        void setFrame(bool value) override;

        void setReadOnly(bool value) override;

        void setAutoMarkIncomplete(bool value) override;

        void setValue(const QVariant& value) override;
        QVariant value() const override;


    private:
        TextEdit* m_text_line;

        void init();

        void onSelectClicked() override;
        void onSaveClicked() override{};
        void onEraseClicked() override;
        void onCheckBoxClicked(bool /*state*/) override{};

    private slots:

    };

}

#endif // TREEITEMTEXTEDIT_H
