//
// Created by admin on 12.05.2024.
//

#ifndef CONTROLSPROG_ICONF_HPP
#define CONTROLSPROG_ICONF_HPP
#include "../controls_global.h"
#include <boost/noncopyable.hpp>
#include <QSize>
#include <QList>
#include <QMap>
#include <QIcon>
#include <QVariant>

namespace arcirk::widgets {

    class CONTROLS_EXPORT IViewsConf: boost::noncopyable {

    public:
        explicit IViewsConf(){
            m_size = {};
            m_columns = {};
            m_column_aliases = {};
            m_order_columns = {};
            m_read_only = false;
            m_enable_rows_icons = true;
            m_logical_icon_def = {};
        }
        [[nodiscard]] virtual QSize size() const{return m_size;};
        virtual std::vector<header_item>& columns(){return m_columns;};
        virtual QString column_name(int index, bool alias = false) const {return {};};
        virtual int column_index(const QString& name){return -1;};
        virtual void set_columns_aliases(const QMap<QString, QString> &aliases){};
        [[nodiscard]] virtual QMap<QString, QString> columns_aliases() const {return m_column_aliases;};
        [[nodiscard]] virtual QList<QString> columns_order() const {return m_order_columns;};
        virtual void reset_columns(const json& arr){};
        virtual void reorder_columns(const QList<QString>& param){};
        virtual void set_column_role(const QString& column, editor_inner_role role){};
        virtual void set_column_not_public(const QString& column, bool value){};
        virtual void set_column_not_public(const QList<QString>& columns, bool value){};
        virtual void set_column_select_type(const QString& column, bool value){};
        [[nodiscard]] virtual bool read_only() const{return m_read_only;};
        virtual void set_read_only(bool value){m_read_only = value;};
        virtual void display_icons(bool value){m_enable_rows_icons = true;};
        [[nodiscard]] virtual bool is_display_icons() const{return m_enable_rows_icons;};
        //[[nodiscard]] virtual QIcon default_icon() const{return m_row_icon;};
        //virtual void set_default_icon(const QIcon& ico){m_row_icon = ico;};

    protected:
        QSize                               m_size;
        std::vector<header_item>            m_columns;
        QMap<QString, QString>              m_column_aliases;
        QList<QString>                      m_order_columns;
        json                                m_root_data;
        bool                                m_read_only{};
        bool                                m_enable_rows_icons{};
        QPair<QIcon, QIcon>                 m_logical_icon_def;
       // QIcon                               m_row_icon;

        virtual void set_columns_order(const QList<QString>& names){};
    };
}
#endif //CONTROLSPROG_ICONF_HPP
