//
// Created by admin on 16.05.2024.
//

#ifndef CONTROLSPROG_TREEITEMIPADDRESS_H
#define CONTROLSPROG_TREEITEMIPADDRESS_H
#ifndef IS_OS_ANDROID
#include "treeitemwidget.h"
#include <QToolButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPainter>
#include <QPaintEngine>

namespace arcirk::widgets {

    class CONTROLS_EXPORT TreeItemIPEdit : public TreeItemWidget
    {
    Q_OBJECT
    public:
        explicit TreeItemIPEdit(int row, int column, QWidget *parent = nullptr);
        explicit TreeItemIPEdit(QWidget *parent = nullptr);

        void setText(const QString& text) override;
        [[nodiscard]] QString text() const override;
        void setRole(editor_inner_role role) override;

        void setFrame(bool value) override;

        void setReadOnly(bool value) override;

        void setAutoMarkIncomplete(bool value) override;

        void setValue(const QVariant& value) override;

        QVariant value() const override;

        bool isValid();

    private:
        LineEdit* m_text_line;
        bool _isValid;

        void init();

        void onSelectClicked() override;
        void onSaveClicked() override{};
        void onEraseClicked() override;
        void onCheckBoxClicked(bool /*state*/) override{};

        bool ipValidator(QString ip);

    private slots:
        void onTextChanged(const QString &text);
    };

}

#endif

#endif //CONTROLSPROG_TREEITEMIPADDRESS_H
