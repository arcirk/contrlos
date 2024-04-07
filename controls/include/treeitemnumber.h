#ifndef TREEITEMNUMBER_H
#define TREEITEMNUMBER_H

#ifndef IS_OS_ANDROID

#include "treeitemwidget.h"
#include <QWidget>
#include <QSpinBox>

namespace arcirk::widgets {

    class CONTROLS_EXPORT TreeItemNumber : public TreeItemWidget
    {
        Q_OBJECT
    public:
        explicit TreeItemNumber(int row, int column, QWidget *parent = nullptr);
        explicit TreeItemNumber(QWidget *parent = nullptr);

        void setText(const QString& text) override {Q_UNUSED(text);};
        QString text() const override {return "";};

        void setRole(editor_inner_role role) override {m_raw->set_role(role) ;};

    private:
        QSpinBox * m_spin;
        void onSelectClicked() override{};
        void onSaveClicked() override{};
        void onEraseClicked() override{};
        void onCheckBoxClicked(bool /*state*/) override{};

        void init();
    };
}

#endif
#endif // TREEITEMNUMBER_H
