#ifndef TREEITEMCOMBOBOX_H
#define TREEITEMCOMBOBOX_H

#ifndef IS_OS_ANDROID

#include "treeitemwidget.h"
#include <QWidget>
#include <QComboBox>
#include "pairmodel.h"

namespace arcirk::widgets {

    class CONTROLS_EXPORT TreeItemComboBox : public TreeItemWidget
    {
        Q_OBJECT
    public:
        explicit TreeItemComboBox(int row, int column, QWidget *parent = nullptr);
        explicit TreeItemComboBox(QWidget *parent = nullptr);

        void setText(const QString& text) override;
        [[nodiscard]] QString text() const override;
        void setRole(editor_inner_role role) override {m_raw->set_role(role);};

        void setModel(PairModel* model);
        void addItems(const QStringList& lst);

        void setCurrentIndex(int index);
        int currentIndex();
        [[nodiscard]] QVariant currentData() const;

    private:
        QComboBox* m_combo;
        void init();

        void onSelectClicked() override{};
        void onSaveClicked() override{};
        void onEraseClicked() override{};
        void onCheckBoxClicked(bool /*state*/) override{};
        void onComboIndexChanged(int index) override;

    private slots:

    signals:

    };
}

#endif
#endif // TREEITEMCOMBOBOX_H
