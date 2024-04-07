#ifndef QUERY_BUILDER_HPP
#define QUERY_BUILDER_HPP

#include "../global.hpp"

#include <map>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/format.hpp>
#include <exception>

#ifdef IS_USE_QT_LIB
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include "../variant/item_data.h"
#endif

#ifdef _WINDOWS
#pragma warning(disable:4100)
#pragma warning(disable:4267)
#endif

//#define TYPE_REQUEST_TABLE "DataTable"
//#define TYPE_REQUEST_ARBITRARY "ArbitraryRequest"

using namespace arcirk;
using namespace arcirk::strings;
using json = nlohmann::json;

inline std::vector<std::string> sqlite_type_names{
    "INTEGER",
    "BIGINT",
    "DATE",
    "DATETIME",
    "DECIMAL",
    "DOUBLE",
    "INT",
    "NONE",
    "NUMERIC",
    "REAL",
    "STRING",
    "TEXT",
    "TIME",
    "VARCHAR",
    "CHAR"
};

namespace arcirk::database::builder {

    enum sql_global_query_type{
        sqlTypeSelect = 0,
        sqlTypeCreateTempTable,
        sqlTypeDeleteTempTable,
        sqlTypeCreateTable,
        sqlTypeDeleteTable
    };

    enum sqlite_types{
        typeINTEGER,
        typeBIGINT,
        typeDATE,
        typeDATETIME,
        typeDECIMAL,
        typeDOUBLE,
        typeINT,
        typeNONE,
        typeNUMERIC,
        typeREAL,
        typeSTRING,
        typeTEXT,
        typeTIME,
        typeVARCHAR,
        typeCHAR,
        typeBLOB,
        typeINVALID = -1
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(sqlite_types, {
       {typeINVALID, nullptr},
       {typeINTEGER, "INTEGER"},
       {typeBIGINT,"BIGINT"},
       {typeDATE,"DATE"},
       {typeDATETIME,"DATETIME"},
       {typeDECIMAL,"DECIMAL"},
       {typeDOUBLE,"DOUBLE"},
       {typeINT,"INT"},
       {typeNONE,"NONE"},
       {typeNUMERIC,"NUMERIC"},
       {typeREAL,"REAL"},
       {typeSTRING,"STRING"},
       {typeTEXT,"TEXT"},
       {typeTIME,"TIME"},
       {typeVARCHAR,"VARCHAR"},
       {typeCHAR,"CHAR"},
       {typeBLOB,"BLOB"}
    });

    enum sql_index_compare{
        compareRTRIM,
        compareNOCASE,
        compareBINARY,
        compareINVALID = -1
    };
    NLOHMANN_JSON_SERIALIZE_ENUM(sql_index_compare, {
        {compareINVALID, nullptr},
        {compareRTRIM, "RTRIM"},
        {compareNOCASE,"NOCASE"},
        {compareBINARY,"BINARY"},
    });

    enum sql_order_type{
        dbASC,
        dbDESC,
        dbINVALID =-1
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(sql_order_type, {
        {dbINVALID, nullptr},
        {dbASC, "ASC"},
        {dbDESC,"DESC"},
    });

    enum sql_foreign_key_event{
        on_delete_cascade,
        on_delete_no_action,
        on_delete_set_null,
        on_delete_set_default,
        on_delete_set_restrict,
        on_update_cascade,
        on_update_no_action,
        on_update_set_null,
        on_update_set_default,
        on_update_set_restrict,
        math_somple,
        math_full,
        math_partial,
        foreign_key_event_invalid = -1
    };
    NLOHMANN_JSON_SERIALIZE_ENUM(sql_foreign_key_event, {
        {foreign_key_event_invalid, nullptr},
        {on_delete_cascade, "ON DELETE CASCADE"},
        {on_delete_no_action,"ON DELETE NO ACTION"},
        {on_delete_set_null,"ON DELETE SET NULL"},
        {on_delete_set_default,"ON DELETE SET DEFAULT"},
        {on_delete_set_restrict,"ON DELETE RESTRICT"},
        {on_update_cascade, "ON UPDATE CASCADE"},
        {on_update_no_action,"ON UPDATE NO ACTION"},
        {on_update_set_null,"ON UPDATE SET NULL"},
        {on_update_set_default,"ON UPDATE SET DEFAULT"},
        {on_update_set_restrict,"ON UPDATE RESTRICT"},
        {math_somple,"MATCH SIMPLE"},
        {math_full,"MATCH [FULL]"},
        {math_partial,"MATCH PARTIAL"},
    });

    enum sql_value_type{
        valTypeField,
        valTypeFunction,
        valTypeInnerQuery
    };
}

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database::builder), sql_value,
    (std::string, field_name)
    (std::string, alias)
    (nlohmann::json, value)
    (arcirk::database::builder::sql_value_type, type)
);

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database::builder), sql_foreign_key,
    (std::string, field)
    (std::string, refereces_table)
    (ByteArray, events)
    (std::string, refereces_field)
    (std::string, query)
    (std::string, name)
    (std::string, deferrable)
    (std::string, deferred)
    )

typedef std::vector<arcirk::database::builder::sql_foreign_key> foreign_keys_arr;

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database::builder), sql_field,
    (std::string, name)    
    (arcirk::database::builder::sqlite_types, type)
    (nlohmann::json, default_value)
    (int, size)
    (bool, autoincrement)
    (bool, prymary_key)
    (bool, not_null)
    (bool, unique)
    (arcirk::database::builder::sql_foreign_key, foreign_keys)
);

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database::builder), sql_index,
    (std::string, name)
    (std::string, table_name)
    (arcirk::database::builder::sql_order_type, order)
    (arcirk::database::builder::sql_index_compare, compare)
    (bool, unique)
    (std::vector<std::string>, fields)
    );

typedef std::vector<arcirk::database::builder::sql_value> sql_values;

namespace arcirk::database::builder {

#ifdef IS_USE_SOCI
using namespace soci;
#endif

    inline bool is_blob_value(const json& value){
        if(!value.is_array())
            return false;
        try {
            auto item = arcirk::widgets::item_data(value);
            return item.data()->subtype != subtypeNull;
        }catch (const std::exception&){
            return false;
        }

        return false;
    }

    template<class T>
    inline std::vector<sql_field> from_structure(const T& e, const std::string& f_autoincrement = "_id", std::map<std::string, sql_foreign_key> foreign_keys = {}){
        std::vector<sql_field> result;
        auto item = pre::json::to_json(e);
        bool is_id = false;
        if(item.find("ref") == item.end())
            item["ref"] = ByteArray(0);

        for (auto itr = item.items().begin(); itr != item.items().end(); ++itr) {
            sql_field f;
            f.name = itr.key();
            f.default_value = itr.value();
            if(itr.key() == f_autoincrement){
                is_id = true;
                f.autoincrement = true;
                f.prymary_key = true;
            }
            if(f.default_value.is_number_float()){
                f.type = typeDOUBLE;
            }else if(f.default_value.is_string()){
                f.type = typeTEXT;
            }else if(f.default_value.is_boolean()){
                f.type = typeINTEGER;
            }else if(f.default_value.is_number()){
                f.type = typeINTEGER;
            }else if(f.default_value.is_binary()){
                f.type = typeBLOB;
            }else if(f.default_value.is_array()){
                f.type = typeBLOB;
            }else if(f.default_value.is_object()){
                f.type = typeBLOB;
            }
            if(itr.key() == "ref"){
                f.unique = true;
                f.not_null = true;
            }
            if(itr.key() == "parent"){
                f.not_null = true;
            }
            if(itr.value() == NIL_STRING_UUID)
                f.size = 36;
            if(foreign_keys.find(itr.key()) != foreign_keys.end()){
                f.foreign_keys = foreign_keys[itr.key()];
            }
            result.push_back(std::move(f));
        }
        if(!is_id)
            result.insert(result.begin(), sql_field("_id", typeINTEGER, 0, 0, true, true, false, false, {}));

        return result;
    }

    inline std::vector<sql_field> from_object_structure(const json& e, const std::string& f_autoincrement = "_id", std::map<std::string, sql_foreign_key> foreign_keys = {}){
        std::vector<sql_field> result;
        json item = e;
        bool is_id = false;
        if(item.find("ref") == item.end())
            item["ref"] = ByteArray();

        for (auto itr = item.items().begin(); itr != item.items().end(); ++itr) {
            sql_field f;
            f.name = itr.key();
            f.default_value = itr.value();
            if(itr.key() == f_autoincrement){
                is_id = true;
                f.autoincrement = true;
                f.prymary_key = true;
            }
            if(f.default_value.is_number_float()){
                f.type = typeDOUBLE;
            }else if(f.default_value.is_string()){
                f.type = typeTEXT;
            }else if(f.default_value.is_boolean()){
                f.type = typeINTEGER;
            }else if(f.default_value.is_number_integer()){
                f.type = typeINTEGER;
            }else if(f.default_value.is_binary()){
                f.type = typeBLOB;
            }else if(f.default_value.is_array()){
                f.type = typeBLOB;
            }else if(f.default_value.is_object()){
                f.type = typeBLOB;
            }
            if(itr.key() == "ref"){
                f.unique = true;
                f.not_null = true;
            }
            if(itr.key() == "parent"){
                f.not_null = true;
            }
            if(itr.value() == NIL_STRING_UUID)
                f.size = 36;
            if(foreign_keys.find(itr.key()) != foreign_keys.end()){
                f.foreign_keys = foreign_keys[itr.key()];
            }
            result.push_back(std::move(f));
        }
        if(!is_id)
            result.insert(result.begin(), sql_field("_id", typeINTEGER, 0, 0, true, true, false, false, {}));

        return result;
    }

    inline json sql_value_to_json(const sql_value val){
        auto result = json::object();
        result["field_name"] = val.field_name;
        result["alias"] = val.alias;
        auto str = val.value.dump();
        result["value"] = arcirk::string_to_byte_array(str);
        return result;
    }

    inline sql_value sql_value_from_json(const json& value){
        sql_value result(value["field_name"].get<std::string>(), value["alias"].get<std::string>(), json{}, valTypeField);
        auto ba = value["value"].get<ByteArray>();
        auto str = arcirk::byte_array_to_string(ba);
        if(json::accept(str))
            result.value = json::parse(str);
        return result;
    }

    enum sql_query_type{
        Select,
        Update,
        Delete,
        Insert,
        Create,
        query_type_INVALID = -1
    };
    NLOHMANN_JSON_SERIALIZE_ENUM(sql_query_type, {
        {query_type_INVALID, nullptr},
        {Insert, "insert"},
        {Update, "update"},
        {Delete, "delete"},
        {Select, "select"},
        {Create, "create"}
    });

    enum sql_database_type{
        type_Sqlite3 = 0,
        type_ODBC
    };

    enum sql_type_of_comparison{
        Equals,
        Not_Equals,
        More,
        More_Or_Equal,
        Less_Or_Equal,
        Less,
        On_List,
        Not_In_List
    };

    enum sql_join_type{
        joinCross = 0,
        joinLeft,
        joinRight,
        joinInner,
        joinFull
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(sql_join_type, {
        {joinCross, "cross"},
        {joinLeft, "left"},
        {joinRight, "right"},
        {joinInner, "inner"},
        {joinFull, "full"}
    });

    typedef std::map<sql_type_of_comparison, std::string> comparation_string;
    struct A{
        static comparation_string compare_template(){ return {
                std::pair<sql_type_of_comparison, std::string>(Equals, "%1%=%2%"),
                std::pair<sql_type_of_comparison, std::string>(Not_Equals, "not %1%=%2%"),
                std::pair<sql_type_of_comparison, std::string>(More, "%1%>%2%"),
                std::pair<sql_type_of_comparison, std::string>(More_Or_Equal, "%1%>=%2%"),
                std::pair<sql_type_of_comparison, std::string>(Less_Or_Equal, "%1%<=%2%"),
                std::pair<sql_type_of_comparison, std::string>(Less, "%1%<%2%"),
                std::pair<sql_type_of_comparison, std::string>(On_List, "%1% in (%2%)"),
                std::pair<sql_type_of_comparison, std::string>(Not_In_List, "not %1% in (%2%)")
            };
        }
    };

    const comparation_string sql_compare_template = A::compare_template();

    typedef struct sql_compare_value{

        std::string key;
        sql_type_of_comparison compare;
        json value;
        std::string alias;

        sql_compare_value() {
            compare = Equals;
        };
        explicit sql_compare_value(const json& object){
            key = object.value("key", "");
            compare = object.value("compare", Equals);
            value = object.value("value", json{});
        }
        explicit sql_compare_value(const std::string& key_, const json& value_, sql_type_of_comparison compare_ = Equals, const std::string& alias_ = ""){
            key = key_;
            compare = compare_;
            value = value_;
            alias = alias_;
        }

        template<class T>
        T get() {
            return sql_compare_value::get<T>(value);
        }

        template<class T>
        static T get(const nlohmann::json& json_object) {
            T object;
            pre::json::detail::dejsonizer dejsonizer(json_object);
            dejsonizer(object);
            return object;
        }

        static std::string array_to_string(const nlohmann::json& json_object, bool quotation_marks = true){

            if(!json_object.is_array())
                return {};

            std::string result;
            int i = 0;
            for (const auto& value : json_object) {
                if(quotation_marks)
                    result.append("'");
                if(value.is_string())
                    result.append(get<std::string>(value));
                if(quotation_marks)
                    result.append("'");
                i++;
                if(i != (int)json_object.size())
                    result.append(",");
            }

            return result;
        }

        [[nodiscard]] std::string to_string(bool quotation_marks = true) const{
            std::string template_value = sql_compare_template.at(compare); //sql_compare_template[compare];
            std::string result;
            if(!value.is_array()){
                bool is_blob = false;
                if(value.is_string()){
                    auto val = value.get<std::string>();
                    is_blob = left(val, 1) == ":" || left(val, 1) == "?";
                }
                if(!is_blob){
                    std::string s_val = quotation_marks ? "'%1%'" : "%1%";
                    std::string v;
                    if(value.is_number_integer()){
                        v = std::to_string(get<long long>(value));
                    }else if(value.is_number_float()){
                        v = std::to_string(get<double>(value));
                    }else if(value.is_string()){
                        v = get<std::string>(value);
                    }
                    result.append(str_sample(template_value, key, str_sample(s_val, v)));
                }else
                    result.append(str_sample(template_value, key, value.get<std::string>()));
            }else{
                result.append(str_sample(template_value, key, array_to_string(value, quotation_marks)));
            }
            return result;
        }

        [[nodiscard]] json to_object() const{
            json result = {
                {"key", key},
                {"compare", compare},
                {"value", value},
                {"alias", alias}
            };
            return result;
        }

    }sql_compare_value;

    class sql_where_values{
    public:
        sql_where_values() {m_list = {};};
        sql_where_values(const std::string& key, const json& value, sql_type_of_comparison compare = Equals) {
            m_list = {};
            add(key, value, compare);
        };
        void add(const std::string& key, const json& value, sql_type_of_comparison compare = Equals){
            m_list.push_back(std::make_shared<sql_compare_value>(key, value, compare));
        }
        void clear(){
            m_list.clear();
        }

        [[nodiscard]] json to_json_object() const{
            json result = {};
            if(!m_list.empty()){
                for (const auto& v: m_list) {
                    result.push_back(v->to_object());
                }
            }
            return result;
        }

    private:
        std::vector<std::shared_ptr<sql_compare_value>> m_list;
    };

    class query_builder{

    public:

        explicit
            query_builder(){
            databaseType = sql_database_type::type_Sqlite3;
            queryType = Select;
            format_text_ = true;
        };
        ~query_builder()= default;

        static std::shared_ptr<query_builder> create(){
            return std::make_shared<query_builder>();
        }

        query_builder& create_table(const std::string& name, const std::vector<sql_field> fields){
            result = "CREATE TABLE IF NOT EXISTS " + name + "(";
            for (auto itr = fields.begin(); itr != fields.end(); ++itr) {
                result.append("\n");
                result.append(" ");
                result.append(itr->name);
                result.append(" ");
                result.append(enum_synonym(itr->type));
                if(itr->size > 0){
                    result.append(" (");
                    result.append(std::to_string(itr->size));
                    result.append(")");
                }
                if(itr->unique)
                    result.append(" UNIQUE");
                if(itr->prymary_key)
                    result.append(" PRIMARY KEY");
                if(itr->autoincrement)
                    result.append(" AUTOINCREMENT");
                if(itr->not_null)
                    result.append(" NOT NULL");
                if(!itr->default_value.empty()){
                    result.append(" DEFAULT");
                    if(itr->default_value.is_number_float()){
                        result.append(" (");
                        result.append(std::to_string(itr->default_value.get<double>()));
                        result.append(")");
                    }else if(itr->default_value.is_number() || itr->default_value.is_number_integer()){
                        result.append(" (");
                        result.append(std::to_string(itr->default_value.get<int>()));
                        result.append(")");
                    }else if(itr->default_value.is_string()){
                        auto str = itr->default_value.get<std::string>();
                        if(str.empty())
                            result.append(" \"\"");
                        else{
                            result.append(" [");
                            result.append(str);
                            result.append("]");
                        }
                    }else if(itr->default_value.is_boolean()){
                        result.append(" (0)");
                    }

                }
                if(!itr->foreign_keys.field.empty()){
                    if(itr->foreign_keys.refereces_table.empty() || itr->foreign_keys.refereces_field.empty()){
                        throw NativeException("Не верный клч foreign key");
                    }
                    result.append("\n");
                    result.append("REFERENCES ");
                    result.append(itr->foreign_keys.refereces_table + " (");
                    result.append(itr->foreign_keys.refereces_field + ")");
                    auto s = byte_array_to_string(itr->foreign_keys.events);
                    if(json::accept(s)){
                        auto events = json::parse(s);
                        for (auto f_event = events.begin(); f_event != events.end(); ++f_event) {
                            json m_key = *f_event;
                            sql_foreign_key_event f_e = m_key.get<sql_foreign_key_event>();
                            result.append(enum_synonym(f_e));
                            if(f_event != --events.end())
                                result.append("\n");
                        }
                    }
                }
                if(itr != --fields.end())
                    result.append(",");
            }
            result.append("\n");
            result.append(");");

            return *this;
        }

        query_builder& create_index(const sql_index& opt){
            result = "CREATE";
            if(opt.unique)
                result.append(" UNIQUE");
            result.append(" INDEX IF NOT EXISTS ");
            result.append(opt.name);
            result.append(" ON ");
            result.append(opt.table_name);
            result.append(" (");

            for (auto itr = opt.fields.begin(); itr != opt.fields.end(); ++itr) {
                result.append("\n   ");
                result.append(itr->c_str());
                if(opt.compare != compareINVALID){
                    result.append(" COLLATE ");
                    result.append(enum_synonym(opt.compare));
                }
                if(opt.order != dbINVALID){
                    result.append(" ");
                    result.append(enum_synonym(opt.order));
                }
                if(itr != --opt.fields.end())
                    result.append(",");
            }
            result.append("\n");
            result.append(");");

            return *this;
        }

        static bool is_select_query(const std::string& query_text){

            std::string tmp(query_text);
            trim(tmp);
            to_lower(tmp);
            if(tmp.length() < 6)
                return false;
            return tmp.substr(6) == "select";

        }

        query_builder& select(const std::vector<builder::sql_value>& fields) {
            use(fields);
            return select();
        }

        query_builder& select(const json& fields) {
            use(fields);
            return select();
        }

        query_builder& select(){
            queryType = Select;
            result = format_text("select ", format_text_);
            result.append(prepare_values(m_list));
            if(m_list.size() == 0){
                result.append("* ");
            }
            return *this;
        }

        static std::string prepare_values(const sql_values& lst, bool use_aliases = true, bool format_ = true){
            std::string res;
            for (auto itr = lst.cbegin(); itr != lst.cend() ; ++itr) {
                sql_value val = *itr;

                if(!val.field_name.empty()){
                    res.append(val.field_name);
                }else{
                    std::string value;
                    if(val.value.is_string()){
                        value.append("'");
                        value = val.value.get<std::string>();
                        value.append("'");
                    }else if(val.value.is_number_float())
                        value = std::to_string(val.value.get<double>());
                    else if(val.value.is_number_integer())
                        value = std::to_string(val.value.get<long long>());

                    res.append(value);
                }
                if(use_aliases && !val.alias.empty() && val.field_name != val.alias)
                    res.append(format_text(" as ", format_) + val.alias);

                if(itr != (--lst.cend())){
                    res.append(",\n\t");
                }
            }

            return res;
        }

        query_builder& select_as(const std::string& alias){
            queryType = Select;
            result = format_text("select ", format_text_);
            for (auto itr = m_list.cbegin(); itr != m_list.cend() ; ++itr) {
                sql_value val = *itr;
                if(!val.field_name.empty()){
                    result.append(alias + "." + val.field_name);
                }else{
                    std::string value;
                    if(val.value.is_string()){
                        value.append("'");
                        value = val.value.get<std::string>();
                        value.append("'");
                    }else if(val.value.is_number_float())
                        value = std::to_string(val.value.get<double>());
                    else if(val.value.is_number_integer())
                        value = std::to_string(val.value.get<long long>());

                    result.append(value);
                    //result.append("'");
                }
                if(!val.alias.empty() && val.field_name != val.alias)
                    result.append(format_text(" as ", format_text_) + val.alias);

                if(itr != (--m_list.cend())){
                    result.append(",\n");
                }
            }
            if(m_list.empty()){
                result.append("* ");
            }
            return *this;
        }

        query_builder& row_count() {
            use(nlohmann::json{format_text("count(*)", format_text_)});
            return select();
        }

        query_builder& join(const std::string& table_name, const json& fields, sql_join_type join_type, const json& join_options = {}){

            if(table_name_.empty())
                throw NativeException("Используйте 'from' сначала для первой таблицы.");

            std::string table_first_alias = table_name_ + "First";
            std::string table_second_alias = table_name + "Second";

            result = format_text("select ", format_text_);

            if(fields.is_object()){
                auto items_ = fields.items();
                for (auto itr = items_.begin(); itr != items_.end(); ++itr) {
                    result.append(table_second_alias + "." + itr.key() + format_text(" as " , format_text_) + table_second_alias + "_" + itr.key());
                    result.append(",\n");
                }
            }else if(fields.is_array()){
                for (auto itr = fields.begin(); itr != fields.end(); ++itr) {
                    std::string key = *itr;
                    result.append(table_second_alias + "." + key + format_text(" as ", format_text_) + table_second_alias + "_" +  key);
                    result.append(",\n");
                }
            }

            if(m_list.empty()){
                result.append(table_name_ + ".*");
            }else{
                //result = "select ";
                for (auto itr = m_list.begin(); itr != m_list.end() ; ++itr) {
                    result.append(table_first_alias + "." + itr->field_name + format_text(" as ", format_text_) + table_first_alias + "_" + itr->field_name);
                    if(itr != --m_list.end())
                        result.append(",\n");
                }
            }
            result.append("\nfrom " + table_name_ + format_text(" as ", format_text_) + table_first_alias + " ");
            result.append(enum_synonym(join_type));
            result.append(" join " + table_name + format_text(" as ", format_text_) + table_second_alias);

            if(join_type != sql_join_type::joinCross && join_options.empty()){
                throw NativeException("Не заданы параметры соединения таблиц!");
            }

            std::vector<std::pair<std::string, std::string>> inner_join;
            if(join_options.is_object()){
                auto items = join_options.items();
                for (auto itr = items.begin(); itr != items.end(); ++itr) {
                    inner_join.emplace_back(itr.key(), itr.value().get<std::string>());
                }
            }else if(join_options.is_array()){
                for (auto itr = fields.begin(); itr != fields.end(); ++itr) {
                    std::string key = *itr;
                    inner_join.emplace_back(key, key);
                }
            }else
                throw NativeException("Не верный тип массива параметров соединения!");

            result.append(" on ");
            for (auto itr = inner_join.begin(); itr != inner_join.end(); ++itr) {
                result.append(table_first_alias);
                result.append(".");
                result.append(itr->first);
                result.append("=");
                result.append(table_second_alias);
                result.append(".");
                result.append(itr->second);
                if(itr != --inner_join.end()){
                    result.append("\nand ");
                }
            }

            //меняем на псевдоним
            table_name_ = table_first_alias;

            return *this;
        }

        query_builder& from(const std::string& table_name, const std::string& alias = ""){
            table_name_ = table_name;
            result.append(format_text("\nfrom ", format_text_));
            result.append(table_name);
            if(!alias.empty())
                result.append(format_text(" as ", format_text_) + alias);
            return *this;
        }

        query_builder& from(const query_builder& subquery, const std::string& alias = ""){
            result.append(format_text("\nfrom ", format_text_));
            result.append("(" + subquery.prepare() + ")");
            if(!alias.empty())
                result.append(format_text(" as ", format_text_) + alias);
            return *this;
        }

        query_builder& where(const json& values, std::vector<std::tuple<std::string, arcirk::BJson>>& blobs, bool position_parameter = false){

            if(values.is_null() || values.empty())
                return *this;

            std::vector<sql_compare_value> where_values;

            auto items_ = values.items();
            for (auto itr = items_.begin(); itr != items_.end(); ++itr) {
                sql_compare_value val{};
                val.key = itr.key();
                const json& f_value = itr.value();
                if(!f_value.is_object()){
                    if(f_value.is_array()){
                        val.compare = On_List;
                        if(!is_blob_value(f_value))
                            val.value = f_value;
                        else{
                            val.compare = Equals;
                            std::string column_name = "?";
                            if(!position_parameter){
                                column_name = ":";
                                column_name.append("where_");
                                column_name.append(itr.key());
                                val.value = column_name;
                            }else
                                val.value = "?";
                            blobs.emplace_back(std::make_tuple(column_name, f_value.get<BJson>()));
                        }

                    }else{
                        val.compare = Equals;
                        val.value = f_value;
                    }
                }else{
                    val.compare = (sql_type_of_comparison)f_value.value("compare", 0);
                    val.value = f_value.value("value", json{});
                }
                where_values.push_back(val);
            }

            if(where_values.empty())
                return *this;

            result.append("\nwhere ");

            for (auto itr = where_values.begin(); itr != where_values.end() ; ++itr) {
                result.append(table_name_ + ".");
                result.append(itr->to_string(true));
                if(itr != --where_values.end())
                    result.append("\nand\n");
            }
            return *this;
        }

        query_builder& where(const json& values, bool use_values = true, bool use_table_name = true){

            if(values.is_null() || values.empty())
                return *this;

            std::vector<sql_compare_value> where_values;

            auto items_ = values.items();
            for (auto itr = items_.begin(); itr != items_.end(); ++itr) {
                sql_compare_value val{};
                val.key = itr.key();
                const json& f_value = itr.value();
                if(!f_value.is_object()){
                    if(f_value.is_array()){
                        val.compare = On_List;
                        if(use_values)
                            val.value = f_value;
                        else
                            val.value = "?";
                    }else{
                        val.compare = Equals;
                        if(use_values)
                            val.value = f_value;
                        else
                            val.value = "?";
                    }
                }else{
                    val.compare = (sql_type_of_comparison)f_value.value("compare", 0);
                    if(use_values)
                        val.value = f_value.value("value", json{});
                    else
                        val.value = "?";
                }
                where_values.push_back(val);
            }

            if(where_values.empty())
                return *this;

            result.append("\nwhere ");

            for (auto itr = where_values.begin(); itr != where_values.end() ; ++itr) {
                if(use_table_name)
                    result.append(table_name_ + ".");
                result.append(itr->to_string(use_values));
                if(itr != --where_values.end())
                    result.append("\nand\n");
            }
            return *this;
        }

        query_builder& with_temp_table(){
            result.insert(0, "with temp_table as(");
            result.append(")");
            return *this;
        }

        query_builder& join_temp_table(sql_join_type join_type){
            result.append(enum_synonym(join_type));
            result.append(" join temp_table");
            return *this;
        }

        query_builder& where_join(const json& values, const std::string& join_table_name, bool use_values){
            std::vector<sql_compare_value> where_values;
            auto items_ = values.items();
            for (auto itr = items_.begin(); itr != items_.end(); ++itr) {
                sql_compare_value val{};
                val.key = itr.key();
                const json& f_value = itr.value();
                if(!f_value.is_object()){
                    val.compare = Equals;
                    if(use_values)
                        val.value = f_value;
                    else
                        val.value = "?";
                }else{
                    val.compare = (sql_type_of_comparison)f_value.value("compare", 0);
                    if(use_values)
                        val.value = f_value.value("value", json{});
                    else
                        val.value = "?";
                }
                where_values.push_back(val);
            }
            result.append("\nand ");
            for (auto itr = where_values.begin(); itr != where_values.end() ; ++itr) {
                result.append(join_table_name + ".");
                result.append(itr->to_string(use_values));
                if(itr != --where_values.end())
                    result.append("\nand\n");
            }

            return *this;
        }

        query_builder& use(const json& source){
            m_list.clear();
            if(source.is_object()){
                auto items_ = source.items();
                for (auto itr = items_.begin(); itr != items_.end(); ++itr) {
                    auto sql_v = sql_value();
                    sql_v.alias = itr.key();
                    if(itr.value().is_string())
                        sql_v.field_name = itr.value().get<std::string>();
                    else
                        sql_v.field_name = sql_v.alias;
                    if(itr.value().is_boolean())
                        sql_v.value = itr.value().get<bool>() ? 1 : 0;
                    else
                        sql_v.value = itr.value();
                    m_list.emplace_back(sql_v);
                }
            }else if(source.is_array()){
                for (auto itr = source.begin(); itr != source.end(); ++itr) {
                    auto sql_v = sql_value();
                    const nlohmann::json& v = *itr;
                    sql_v.value = v;
                    if(v.is_string()){
                        sql_v.field_name = v.get<std::string>();
                        sql_v.alias = sql_v.field_name;
                    }else if(v.is_object()){
                        auto items = v.items();
                        for (auto const& val : items) {
                            sql_v.alias = val.key();
                            if(val.value().is_string())
                                sql_v.field_name = val.value().get<std::string>();
                            else
                                sql_v.field_name = sql_v.alias;
                            sql_v.value = val.value();
                        }
                    }
                    m_list.emplace_back(sql_v);
                }
            }

            return *this;

        }

        query_builder& use(const std::vector<sql_value>& values){
            m_list.clear();
            m_list =std::vector<sql_value>(values.size());
            std::copy(values.begin(),values.end(), m_list.begin());
            return *this;
        }

        [[nodiscard]] std::string ref() const{
            for (auto itr = m_list.cbegin(); itr != m_list.cend(); ++itr) {
                if(itr->field_name == "ref"){
                    return itr->value.get<std::string>();
                }

            }
            return {};
        }

        query_builder& group_by(const json& fields){
            result +="\ngroup by ";
            std::vector<std::string> m_grope_list;
            if(fields.is_object()){
                auto items_ = fields.items();
                for (auto itr = items_.begin(); itr != items_.end(); ++itr) {
                    m_grope_list.emplace_back(itr.key());
                }
            }else if(fields.is_array()){
                for (auto itr = fields.begin(); itr != fields.end(); ++itr) {
                    m_grope_list.emplace_back(*itr);
                }
            }else
                m_grope_list.emplace_back(fields.get<std::string>());

            for (auto itr = m_grope_list.cbegin(); itr != m_grope_list.cend() ; ++itr) {
                result.append(*itr);
                if(itr != (--m_grope_list.cend())){
                    result.append(",\n");
                }
            }
            return *this;

        }

        //0=asc, 1=desc
        //fields = {{"field1", 0}, {"field2", 1}}
        query_builder& order_by(const json& fields){

            result +="\norder by ";
            std::vector<std::pair<std::string, sql_order_type>> m_order_list;
            if(fields.is_object()){
                auto items_ = fields.items();
                for (auto itr = items_.begin(); itr != items_.end(); ++itr) {
                    if(itr.value().is_number_integer()){
                        int order = itr.value().get<int>();
                        auto t = sql_order_type::dbASC;
                        if(order <= 1 && order >= 0)
                            t = (sql_order_type)order;
                        m_order_list.emplace_back(itr.key(), t);
                    }else
                        m_order_list.emplace_back(itr.key(), sql_order_type::dbASC);
                }
            }else if(fields.is_array()){
                for (auto itr = fields.begin(); itr != fields.end(); ++itr) {
                    m_order_list.emplace_back(*itr, sql_order_type::dbASC);
                }
            }else
                m_order_list.emplace_back(fields.get<std::string>(), sql_order_type::dbASC);

            for (auto itr = m_order_list.cbegin(); itr != m_order_list.cend() ; ++itr) {
                result.append(itr->first);
                result.append(" ");
                result.append(enum_synonym(itr->second));
                if(itr != (--m_order_list.cend())){
                    result.append(",\n");
                }
            }
            return *this;
        }

        query_builder& update(const std::string& table_name, bool use_values = true, bool skip_id = true){
            queryType = Update;
            table_name_ = table_name;
            result = str_sample("update %1% set ", table_name);
            for (auto itr = m_list.cbegin(); itr != m_list.cend() ; ++itr) {
                if(itr->alias == "_id" && skip_id)
                    continue;
                result.append("[" + itr->alias + "]");
                if(use_values){
                    std::string value;
                    auto val = itr->value;
                    if(val.is_string())
                        value = val.get<std::string>();
                    else if(val.is_number_float())
                        value = std::to_string(val.get<double>());
                    else if(val.is_number_integer())
                        value = std::to_string(val.get<long long>());

                    if(value.empty())
                        result.append("=''");
                    else
                        result.append(str_sample("='%1%'", value));
                }else
                    result.append("=?");
                if(itr != (--m_list.cend())){
                    result.append(",\n");
                }
            }

            return *this;
        }

        query_builder& update(const std::string& table_name, std::vector<std::tuple<std::string, arcirk::BJson>>& blobs){
            queryType = Update;
            table_name_ = table_name;
            result = str_sample("update %1% set ", table_name);
            for (auto itr = m_list.cbegin(); itr != m_list.cend() ; ++itr) {
                std::string column = itr->alias;
                if(itr->alias.empty())
                    column = itr->field_name;
                if(column == "_id")
                    continue;
                result.append("[" + column + "]");
                std::string value;
                auto val = itr->value;
                if(val.is_string())
                    value = val.get<std::string>();
                else if(val.is_number_float()){
                    value = std::to_string(val.get<double>());
                    auto index = value.find(',');
                    if(index > 0)
                        value.replace(index, 1, ".");
                }else if(val.is_number_integer())
                    value = std::to_string(val.get<long long>());
                else if(val.is_boolean()){
                    auto m_v = val.get<bool>();
                    value = std::to_string(m_v ? 1 : 0);
                }else if(val.is_array()){
                    std::string f_name = ":";
                    f_name.append(column);
                    try{
                        auto ba = val.get<BJson>();
                        blobs.emplace_back(f_name, ba);
                    } catch (const std::exception &) {
                        blobs.emplace_back(f_name, BJson());
                    }
                    value = f_name;
                }

                boost::erase_all(value, "'");

                if(value.empty())
                    result.append("=''");
                else
                    if(left(value, 1) != ":")
                        result.append(str_sample("='%1%'", value));
                    else
                        result.append(str_sample("=%1%", value));

                if(itr != (--m_list.cend())){
                    result.append(",\n");
                }
            }

            return *this;
        }
        query_builder& insert(const std::string& table_name, std::vector<std::tuple<std::string, arcirk::ByteArray>>& blobs){

            queryType = Insert;
            table_name_ = table_name;
            result = str_sample("insert into %1% (", table_name);
            std::string string_values;
            for (auto itr = m_list.cbegin(); itr != m_list.cend() ; ++itr) {
                std::string column = itr->alias;
                if(itr->alias.empty())
                    column = itr->field_name;
                if(column == "_id")
                    continue;
                result.append("[" + column + "]");
                auto val = itr->value;
                std::string value;
                if(val.is_string())
                    value = val.get<std::string>();
                else if(val.is_number_float()){
                    value = std::to_string(val.get<double>());
                    auto index = value.find(',');
                    if(index > 0)
                        value.replace(index, 1, ".");
                }else if(val.is_number_integer())
                    value = std::to_string(val.get<long long>());
                else if(val.is_boolean()){
                    auto m_v = val.get<bool>();
                    value = std::to_string(m_v ? 1 : 0);
                }else if(val.is_array()){
                    std::string f_name = ":";
                    f_name.append(column);
                    try{
                        auto ba = val.get<ByteArray>();
                        blobs.emplace_back(f_name, ba);
                    } catch (const std::exception &) {
                        blobs.emplace_back(f_name, ByteArray());
                    }
                    value = f_name;
                }

                boost::erase_all(value, "'");

                if(value.empty())
                    string_values.append("''");
                else
                    if(left(value, 1) != ":")
                        string_values.append(str_sample("'%1%'", value));
                    else
                        string_values.append(value);

                if(itr != (--m_list.cend())){
                    result.append(",\n");
                    string_values.append(",\n");
                }
            }
            result.append(")\n");
            result.append("values(");
            result.append(string_values);
            result.append(")");

            return *this;
        }

        query_builder& insert(const std::string& table_name, bool use_values = true, bool skip_id = true){
            queryType = Insert;
            table_name_ = table_name;
            result = str_sample("insert into %1% (", table_name);
            std::string string_values;
            for (auto itr = m_list.cbegin(); itr != m_list.cend() ; ++itr) {
                std::string column = itr->alias;
                if(itr->alias.empty())
                    column = itr->field_name;
                if(column == "_id" && skip_id)
                    continue;
                result.append("[" + column + "]");
                auto val = itr->value;
                if(use_values){
                    std::string value;
                    if(val.is_string())
                        value = val.get<std::string>();
                    else if(val.is_number_float())
                        value = std::to_string(val.get<double>());
                    else if(val.is_number_integer())
                        value = std::to_string(val.get<long long>());
                    else if(val.is_boolean()){
                        auto m_v = val.get<bool>();
                        value = std::to_string(m_v ? 1 : 0);
                    }
                    if(value.empty())
                        string_values.append("''");
                    else
                        string_values.append(str_sample("'%1%'", value));
                }else
                    string_values.append("?");

                if(itr != (--m_list.cend())){
                    result.append(",\n");
                    string_values.append(",\n");
                }
            }
            result.append(")\n");
            result.append("values(");
            result.append(string_values);
            result.append(")");

            return *this;
        }

        query_builder& remove(){
            queryType = Delete;
            result = "delete ";
            return *this;
        }

        query_builder& union_all(const query_builder& nestedQuery, bool all = true){
            result.append(str_sample("\nunion%1%\n", all ? " all" : ""));
            result.append(nestedQuery.prepare());
            return *this;
        }

        [[nodiscard]] std::string prepare(const json& where_not_exists = {}, bool use_values = false) const{
            if (where_not_exists.empty())
                return result;
            else{
                using namespace boost::algorithm;
                bool odbc = databaseType == type_ODBC;
                if(queryType == Insert){
                    std::string query_sample;
                    if (odbc) {
                        query_sample = "IF NOT EXISTS\n"
                                       "(%1%)\n"
                                       "BEGIN\n"
                                       "%2%\n"
                                       "END";
                    } else {
                        query_sample = "%2% \n"
                                       "WHERE NOT EXISTS (%1%)";
                    }
                    std::string tmp;
                    if(databaseType == type_Sqlite3){
                        tmp = replace_first_copy(result, "values(", "select ");
                        if(tmp[tmp.length() -1] == ')')
                            tmp = tmp.substr(0, tmp.length()-1);
                    }
                    else
                        tmp = result;
                    auto q = std::make_shared<query_builder>();
                    std::string where_select = q->select(json{"*"}).from(table_name_).where(where_not_exists, use_values).prepare();
                    return str_sample(query_sample, where_select, tmp);
                }
            }

            return result;
        }

        sql_database_type database_type(){
            return databaseType;
        }

        void set_databaseType(sql_database_type dbType){
            databaseType = dbType;
        }
    #ifdef IS_USE_SOCI
        soci::rowset<soci::row> exec(soci::session& sql, const json& exists = {}, bool use_values = false) const{
            std::string q = prepare(exists, use_values);
            //std::cout << q << std::endl;
            return (sql.prepare << q);
        }

        void execute(soci::session& sql, const json& exists = {}, bool use_values = false) const{
            std::string q = prepare(exists, use_values);
            soci::statement st = (sql.prepare << q);
            st.execute(true);
        }

        static void execute(const std::string& query_text, soci::session& sql){
            soci::statement st = (sql.prepare << query_text);
            st.execute(true);
        }

        template<typename T>
        static T get_value(soci::row const& row, const std::size_t& column_index){
            //не знаю как правильно проверить на null поэтому вот так ...
            try {
                return row.get<T>(column_index);
            }catch (...){
                return {};
            }
        }

        template<typename T>
        std::vector<T> rows_to_array(soci::session& sql){
            if(!is_valid())
                return std::vector<T>{};
            soci::rowset<T> rs = (sql.prepare << result);
            std::vector<T> m_vec;
            for (auto it = rs.begin(); it != rs.end(); it++) {
                T user_data = *it;
                m_vec.emplace_back(user_data);
            }
            return m_vec;
        }

        static void execute(const std::string& query_text, soci::session& sql, json& result_table, const std::vector<std::string>& column_ignore = {}){

            soci::rowset<soci::row> rs = (sql.prepare << query_text);

            // std::cout << query_text << std::endl;

            json columns = {"line_number"};
            json roms = {};
            int line_number = 0;

            for (rowset<row>::const_iterator it = rs.begin(); it != rs.end(); ++it)
            {
                line_number++;
                row const& row = *it;
                json j_row = {{"line_number", line_number}};

                for(std::size_t i = 0; i != row.size(); ++i)
                {
                    const column_properties & props = row.get_properties(i);
                    std::string column_name = props.get_name();

                    if((columns.size() -1) != row.size()){
                        columns.push_back(column_name);
                    }

                    if(std::find(column_ignore.begin(), column_ignore.end(), column_name) != column_ignore.end()){
                        j_row += {column_name, ""};
                        continue;
                    }

                    switch(props.get_data_type())
                    {
                    case dt_string:{
                        auto val = get_value<std::string>(row, i);
                        j_row += {column_name, val};
                    }
                    break;
                    case dt_double:{
                        auto val = get_value<double>(row, i);
                        j_row += {column_name, val};
                    }
                    break;
                    case dt_integer:{
                        auto val = get_value<int>(row, i);
                        j_row += {column_name, val};
                    }
                    break;
                    case dt_long_long:{
                        auto val = get_value<long long>(row, i);
                        j_row += {column_name, val};
                    }
                    break;
                    case dt_unsigned_long_long:{
                        auto val = get_value<unsigned long long>(row, i);
                        j_row += {column_name, val};
                    }
                    break;
                    case dt_date:
                        //std::tm when = r.get<std::tm>(i);
                        break;
                    case dt_blob:
                        break;
                    case dt_xml:
                        break;
                    }

                }

                roms += j_row;
            }

            result_table = {
                {"columns", columns},
                {"rows", roms}
            };

        }
    #endif


        bool is_valid(){
            return !result.empty();
        }

        void clear(){
            result = "";
            m_list.clear();
            table_name_ = "";
        }

    #ifdef IS_USE_QT_LIB

        template<typename T>
        static T object(QSqlDatabase& sql, const QString& query){
            QSqlQuery rs(sql);
            rs.exec(query);
            nlohmann::json j_row{};
            while (rs.next()) {
                QSqlRecord row = rs.record();
                for (int i = 0; i < row.count(); ++i) {
                    std::string column_name = row.fieldName(i).toStdString();
                    QVariant val = row.field(i).value();
                    if(val.userType() == QMetaType::QString)
                        j_row[column_name] = val.toString().toStdString();
                    else if(val.userType() == QMetaType::Double)
                        j_row[column_name] = val.toDouble();
                    else if(val.userType() == QMetaType::Int)
                        j_row[column_name] = val.toInt();
                    else if(val.userType() == QMetaType::LongLong)
                        j_row[column_name] = val.toLongLong();
                    else if(val.userType() == QMetaType::ULongLong)
                        j_row[column_name] = val.toULongLong();
                    else if(val.userType() == QMetaType::QByteArray)
                        j_row[column_name] = "<бинарные данные>";
                }
                break;
            }

            return secure_serialization<T>(j_row);
        }

        template<typename T>
        std::vector<T> rows_to_array(QSqlDatabase& sql){

            if(!is_valid() || !sql.isOpen())
                return std::vector<T>{};

            QSqlQuery rs(sql);
            rs.exec(result.c_str());
            std::vector<T> m_vec;
            if(rs.lastError().type() == QSqlError::NoError){
                while (rs.next()) {
                    T user_data = T();
                    auto verify = pre::json::to_json(user_data);
                    QSqlRecord row = rs.record();
                    for (int i = 0; i < row.count(); ++i) {
                        std::string column_name = row.fieldName(i).toStdString();
                        if(verify.find(column_name) == verify.end())
                            continue;
                        QVariant val = row.field(i).value();
//
//                        std::cout << val.typeId() << " " << column_name << " " << verify[column_name].type_name() << std::endl;

                        if(val.typeId() == QMetaType::QString) {
                            if (verify[column_name].is_string())
                                verify[column_name] = val.toString().toStdString();
                        }else if(val.typeId() == QMetaType::Double) {
                            if (verify[column_name].is_number())
                                verify[column_name] = val.toDouble();
                        }else if(val.typeId() == QMetaType::Int) {
                            if (verify[column_name].is_number())
                                verify[column_name] = val.toInt();
                        }else if(val.typeId() == QMetaType::LongLong) {
                            if (verify[column_name].is_number())
                                verify[column_name] = val.toLongLong();
                            else if(verify[column_name].is_boolean())
                                verify[column_name] = val.toLongLong() > 0 ? true : false;
                        }else if(val.typeId() == QMetaType::ULongLong) {
                            if (verify[column_name].is_number())
                                verify[column_name] = val.toULongLong();
                        }else if(val.typeId() == QMetaType::QByteArray) {
                            if (verify[column_name].is_array()) {
                                auto q_ba = val.toByteArray();
                                auto ba = BJson(q_ba.size());
                                std::copy(q_ba.begin(), q_ba.end(), ba.begin());
                                verify[column_name] = ba;
                            }
                        }
                    }
                    //user_data = secure_serialization<T>(verify);
                    user_data = pre::json::from_json<T>(verify);
                    m_vec.emplace_back(user_data);
                }
            }else
                std::cerr << rs.lastError().text().toStdString() << std::endl;

            return m_vec;
        }

        json to_table(QSqlDatabase& sql){
            if(!is_valid() || !sql.isOpen())
                return {};
            QSqlQuery rs(sql);
            rs.exec(result.c_str());
            auto m_rows = json::array();
            auto columns = json::array();
            while (rs.next()) {
                auto user_data = json::object();
                QSqlRecord row = rs.record();
                for (int i = 0; i < row.count(); ++i) {
                    std::string column_name = row.fieldName(i).toStdString();
                    if(columns.find(column_name) == columns.end())
                        columns += column_name;
                    QVariant val = row.field(i).value();

                    if(val.userType() == QMetaType::QString)
                        user_data[column_name] = val.toString().toStdString();
                    else if(val.userType() == QMetaType::Double)
                        user_data[column_name] = val.toDouble();
                    else if(val.userType() == QMetaType::Int)
                        user_data[column_name] = val.toInt();
                    else if(val.userType() == QMetaType::LongLong)
                        user_data[column_name] = val.toLongLong();
                    else if(val.userType() == QMetaType::ULongLong)
                        user_data[column_name] = val.toULongLong();
                    else if(val.userType() == QMetaType::QByteArray){
                        auto q_ba = val.toByteArray();
                        auto ba = ByteArray(q_ba.size());
                        std::copy(q_ba.begin(), q_ba.end(), ba.begin());
                        user_data[column_name] = ba;
                    }

                }
                m_rows += user_data;
            }

            auto table = json::object();
            table["columns"] = columns;
            table["rows"] = m_rows;

            return table;
        }

        template<typename T>
        static std::vector<T> array(QSqlDatabase& sql, const QString& query_text){
            QSqlQuery rs(sql);
            rs.exec(query_text);
            std::vector<T> m_vec;
            while (rs.next()) {
                T user_data = T();
                QSqlRecord row = rs.record();
                nlohmann::json j_row{};
                for (int i = 0; i < row.count(); ++i) {
                    std::string column_name = row.fieldName(i).toStdString();
                    QVariant val = row.field(i).value();

                    if(val.userType() == QMetaType::QString)
                        j_row[column_name] = val.toString().toStdString();
                    else if(val.userType() == QMetaType::Double)
                        j_row[column_name] = val.toDouble();
                    else if(val.userType() == QMetaType::Int)
                        j_row[column_name] = val.toInt();
                    else if(val.userType() == QMetaType::LongLong)
                        j_row[column_name] = val.toLongLong();
                    else if(val.userType() == QMetaType::ULongLong)
                        j_row[column_name] = val.toULongLong();
                    else if(val.userType() == QMetaType::QByteArray){
                        auto q_ba = val.toByteArray();
                        auto ba = ByteArray(q_ba.size());
                        std::copy(q_ba.begin(), q_ba.end(), ba.begin());
                        j_row[column_name] = ba;
                    }
                }
                try {
                    user_data = secure_serialization<T>(j_row);
                } catch (...) {
                }
                m_vec.emplace_back(user_data);
            }
            return m_vec;
        }

    #endif
        query_builder& add_alias(const std::string& alias){
            result.append(alias);
            return *this;
        }

        std::string with_recursive(const std::string& table, const sql_values& values, const nlohmann::json& where_root, const nlohmann::json& where_childs, const std::string& order = ""){
            result = "WITH RECURSIVE tc (\n";
            result.append(prepare_values(values, false));
            result.append("n\")");

            auto arr_cols = sql_values();
            for (auto itr = values.begin(); itr != values.end(); ++itr) {
                auto col = sql_value();
                col.alias = itr->field_name;
                col.value = itr->field_name == "ref" ? "parent" : "";
                if(itr->field_name == "is_group")
                    col.value = 1;
                arr_cols.push_back(col);
            }

            result.append(query_builder().select(arr_cols).from(table).where(where_childs, true).group_by(json::array({"parent"})).prepare());
            result.append("\nUNION\n");
            result.append(query_builder().select(values).from(table, "tab1").add_alias(", tc").where(where_childs, true).prepare());
            if(!where_childs.empty())
                result.append("AND \n");
            result.append("tab1.parent = tc.ref");
            result.append("\n)\n");
            result.append(query_builder().select().from("tc").where(where_root, true).prepare());
            if(!order.empty())
                result.append("\nORDER BY tc." + order);
            return result;
        }

        query_builder& limit(int value, int offset_value = 0){
            result.append("\nLIMIT ");
            result.append(std::to_string(value));
            if(offset_value > 0){
                result.append(" OFFSET ");
                result.append(std::to_string(offset_value));
            }
            return *this;
        }

        query_builder& add_field(sql_value val){
            m_list.push_back(val);
            return *this;
        }

    private:
        std::string result;
        sql_values m_list;
        sql_query_type queryType;
        sql_database_type databaseType;
        std::string table_name_;
        bool format_text_;

        static std::string format_text(const std::string& text, bool format_){
            if(!format_){
                return text;
            }else{
                std::string str = text;
                to_upper(str);
                return str;
            }
        }
    };

}

#endif // QUERY_BUILDER_HPP
