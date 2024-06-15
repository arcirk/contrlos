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
#include <memory>
#include "header_item.h"

namespace arcirk::widgets {

//    struct CONTROLS_EXPORT header_item{
//
//        std::string name;
//        std::string alias;
//        std::string format;
//        arcirk::BJson selection_list{};
//        int default_type = -1;
//        arcirk::BJson default_value{};
//        bool marked = false;
//        bool not_public = false;
//        bool override_buttons = false;
//        bool select_type = false;
//        int use = 0;
//        std::string validate_text;
//
//        void from_json(const json& details){
//            auto item = arcirk::secure_serialization<header_item_wrapper>(details);
//            this->name = item.name;
//            this->alias = item.alias;
//            this->format = item.format;
//            this->selection_list = BJson (item.selection_list.size());
//            std::copy(item.selection_list.begin(), item.selection_list.end(), this->selection_list.begin());
//            this->default_type = item.default_type;
//            this->default_value = BJson(item.default_value.size());
//            std::copy(default_value.begin(), default_value.end(), item.default_value.begin());
//            this->marked = item.marked;
//            this->not_public = item.not_public;
//            this->override_buttons = item.override_buttons;
//            this->use = item.use;
//            this->validate_text = item.validate_text;
//        }
//
//        json to_json() const{
//            auto item = header_item_wrapper();
//            item.validate_text = validate_text;
//            item.use = use;
//            item.override_buttons = override_buttons;
//            item.not_public = not_public;
//            item.marked = marked;
//            item.default_value = BJson(default_value.size());
//            std::copy(default_value.begin(), default_value.end(), item.default_value.begin());
//            item.selection_list = BJson(selection_list.size());
//            std::copy(selection_list.begin(), selection_list.end(), item.selection_list.begin());
//            item.format = format;
//            item.alias = alias;
//            item.name = name;
//            return pre::json::to_json(item);
//        }
//
//        explicit header_item(){
//            this->name = "";
//            this->alias = "";
//            this->format = "";
//            this->selection_list = {};
//            this->default_type = -1;
//            this->default_value = {};
//            this->marked = false;
//            this->not_public = false;
//            this->override_buttons = false;
//            this->use = 0;
//            this->validate_text = "";
//        }
//        explicit header_item(const std::string& name, const std::string& alias){
//            this->name = name;
//            this->alias = alias;
//        }
//
//        explicit header_item(const json& details){
//            auto item = arcirk::secure_serialization<header_item_wrapper>(details);
//            this->name = item.name;
//            this->alias = item.alias;
//            this->format = item.format;
//            this->selection_list = BJson (item.selection_list.size());
//            std::copy(item.selection_list.begin(), item.selection_list.end(), this->selection_list.begin());
//            this->default_type = item.default_type;
//            this->default_value = BJson(item.default_value.size());
//            std::copy(default_value.begin(), default_value.end(), item.default_value.begin());
//            this->marked = item.marked;
//            this->not_public = item.not_public;
//            this->override_buttons = item.override_buttons;
//            this->use = item.use;
//            this->validate_text = item.validate_text;
//        }
//        explicit header_item(const header_item_wrapper& item){
//            this->name = item.name;
//            this->alias = item.alias;
//            this->format = item.format;
//            this->selection_list = BJson (item.selection_list.size());
//            std::copy(item.selection_list.begin(), item.selection_list.end(), this->selection_list.begin());
//            this->default_type = item.default_type;
//            this->default_value = BJson(item.default_value.size());
//            std::copy(item.default_value.begin(), item.default_value.end(), this->default_value.begin());
//            this->marked = item.marked;
//            this->not_public = item.not_public;
//            this->override_buttons = item.override_buttons;
//            this->use = item.use;
//            this->validate_text = item.validate_text;
//        }
//    };
//
//    typedef std::vector<std::shared_ptr<header_item>> HeaderItems;
//    typedef std::shared_ptr<header_item> HeaderItem;

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
        virtual HeaderItems& columns(){return m_columns;};
        virtual header_item * header_column(const QString& name){return nullptr;};
        [[nodiscard]] virtual QString column_name(int index, bool alias = false) const {return {};};
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
        [[nodiscard]] virtual QList<QString> predefined_list() const{return {};};

    protected:
        QSize                               m_size;
        HeaderItems                         m_columns;
        QMap<QString, QString>              m_column_aliases;
        QList<QString>                      m_order_columns;
        json                                m_root_data;
        bool                                m_read_only{};
        bool                                m_enable_rows_icons{};
        QPair<QIcon, QIcon>                 m_logical_icon_def;

        virtual void set_columns_order(const QList<QString>& names){};
    };

}
#endif //CONTROLSPROG_ICONF_HPP
