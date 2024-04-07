#ifndef TREEITEMTEXTLINE_H
#define TREEITEMTEXTLINE_H
#ifndef IS_OS_ANDROID
#include "treeitemwidget.h"
#include <QToolButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPainter>
#include <QPaintEngine>

namespace arcirk::widgets {

    class CONTROLS_EXPORT TreeItemTextLine : public TreeItemWidget
    {
        Q_OBJECT
    public:
        explicit TreeItemTextLine(int row, int column, QWidget *parent = nullptr);
        explicit TreeItemTextLine(QWidget *parent = nullptr);

        void setText(const QString& text) override;
        QString text() const override;
        void setRole(editor_inner_role role) override;

        void setFrame(bool value) override;

        void setReadOnly(bool value) override;

        void setAutoMarkIncomplete(bool value) override;

        void setValue(const QVariant& value) override;
        QVariant value() const override;


    private:
        LineEdit* m_text_line;

        void init();

        void onSelectClicked() override;
        void onSaveClicked() override{};
        void onEraseClicked() override;
        void onCheckBoxClicked(bool /*state*/) override{};

    private slots:

    };

}
#endif
#endif // TREEITEMTEXTLINE_H
