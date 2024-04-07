#ifndef TREEITEMCHECKBOX_H
#define TREEITEMCHECKBOX_H

#ifndef IS_OS_ANDROID

#include "../controls_global.h"
#include <QObject>
#include <QWidget>
#include "treeitemwidget.h"
#include <QCheckBox>

namespace arcirk::widgets {

    class CONTROLS_EXPORT TreeItemCheckBox : public TreeItemWidget
    {
        Q_OBJECT
    public:
        explicit TreeItemCheckBox(int row, int column, QWidget *parent = nullptr);
        explicit TreeItemCheckBox(QWidget *parent = nullptr);

        QVariant value() const override;
        void setValue(const QVariant& value) override;
        void setText(const QString& text) override;
        QString text() const override;
        void setRole(editor_inner_role role) override {m_raw->set_role(role);};

        void setAlignment(Qt::Alignment alignment);

    private:
        QCheckBox* m_check;
        void init();

        void onSelectClicked() override{};
        void onSaveClicked() override{};
        void onEraseClicked() override{};
        void onCheckBoxClicked(bool state) override;

    signals:
        void checkBoxClicked(int row, int column, bool state);
    };
}

#endif
#endif // TREEITEMCHECKBOX_H
