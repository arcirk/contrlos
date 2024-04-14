#ifndef CONTROLSPROG_TREECONF_H
#define CONTROLSPROG_TREECONF_H

#include "../controls_global.h"
#include <boost/noncopyable.hpp>
#include <QSize>
#include <QList>
#include <QMap>
#include <QIcon>
#include <QVariant>

namespace arcirk::widgets {

    class CONTROLS_EXPORT TreeConf : boost::noncopyable {

    public:
        explicit TreeConf();

        [[nodiscard]] QSize size() const;
        [[nodiscard]] std::vector<header_item>& columns();
        [[nodiscard]] QString column_name(int index, bool alias = false) const;
        int column_index(const QString& name);
        void set_columns_aliases(const QMap<QString, QString> &aliases);
        [[nodiscard]] QMap<QString, QString> columns_aliases() const;
        [[nodiscard]] QList<QString> columns_order() const;
        void reset_columns(const json& arr);
        void reorder_columns(const QList<QString>& param);
        void set_column_role(const QString& column, editor_inner_role role);
        void set_column_not_public(const QString& column, bool value);
        void set_column_select_type(const QString& column, bool value);
        [[nodiscard]] bool read_only() const;
        void set_read_only(bool value);
        void display_icons(bool value);
        [[nodiscard]] bool is_display_icons() const;
        QIcon default_icon(tree_rows_icons state) const;

    private:
        QSize                               m_size;
        std::vector<header_item>            m_columns;
        QList<QString>                      m_order_columns;
        QMap<QString, QString>              m_column_aliases;
        json                                m_root_data;
        bool                                m_read_only;
        bool                                m_enable_rows_icons;
        QPair<QIcon, QIcon>                 m_logical_icon_def;
        QMap<tree_rows_icons, QIcon>        m_row_icon;

        void set_columns_order(const QList<QString>& names);
        void init_default_icons();
    };
}

#endif //CONTROLSPROG_TREECONF_H