//
// Created by admin on 13.01.2024.
//
#ifndef CONTROLSPROG_TABLECONF_H
#define CONTROLSPROG_TABLECONF_H

#include "iconf.hpp"

namespace arcirk::widgets {

    class CONTROLS_EXPORT TableConf: public IViewsConf {

    public:
        explicit TableConf();

        [[nodiscard]] QSize size() const override;
        [[nodiscard]] HeaderItems& columns() override;
        header_item * header_column(const QString& name) override;
        [[nodiscard]] QString column_name(int index, bool alias = false) const override;
        int column_index(const QString& name) override;
        void set_columns_aliases(const QMap<QString, QString> &aliases) override;
        [[nodiscard]] QMap<QString, QString> columns_aliases() const override;
        [[nodiscard]] QList<QString> columns_order() const override;
        void reset_columns(const json& arr) override;
        void reorder_columns(const QList<QString>& param) override;
        void set_column_role(const QString& column, editor_inner_role role) override;
        void set_column_not_public(const QString& column, bool value) override;
        void set_column_not_public(const QList<QString>& columns, bool value) override;
        void set_column_select_type(const QString& column, bool value) override;
        [[nodiscard]] bool read_only() const override;
        void set_read_only(bool value) override;
        void display_icons(bool value) override;
        [[nodiscard]] bool is_display_icons() const override;
        [[nodiscard]] QIcon default_icon() const;
        void set_default_icon(const QIcon& ico);
        [[nodiscard]] QList<QString> predefined_list() const override;

    protected:
        void set_columns_order(const QList<QString>& names) override;
    private:
        QIcon                               m_row_icon;

    };

}
#endif //CONTROLSPROG_TABLECONF_H
