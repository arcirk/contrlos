//
// Created by admin on 13.01.2024.
//
#ifndef CONTROLSPROG_TABLECONF_H
#define CONTROLSPROG_TABLECONF_H

#include "../controls_global.h"
#include <boost/noncopyable.hpp>
#include <QSize>
#include <QList>
#include <QMap>
#include <QIcon>
#include <QVariant>

namespace arcirk::widgets {

    class CONTROLS_EXPORT TableConf: boost::noncopyable {

    public:
        explicit TableConf();

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
        [[nodiscard]] QIcon default_icon() const;
        void set_default_icon(const QIcon& ico);

    private:
        QSize                               m_size;
        std::vector<header_item>            m_columns;
        QList<QString>                      m_order_columns;
        //QMap<QString, QMap<int, QVariant>>  m_header_data;
        QMap<QString, QString>              m_column_aliases;
        json                                m_root_data;
        bool                                m_read_only;
        bool                                m_enable_rows_icons;
        QPair<QIcon, QIcon>                 m_logical_icon_def;
        QIcon                               m_row_icon;

        void set_columns_order(const QList<QString>& names);

    };

}
#endif //CONTROLSPROG_TABLECONF_H
