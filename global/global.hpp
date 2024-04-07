#ifndef ARCIRK_GROUP_ARCIRK_HPP
#define ARCIRK_GROUP_ARCIRK_HPP

#include <iostream>
#include <utility>
#include <vector>
#include <fstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <cassert>

#ifdef USE_BOOST_ASIO
#ifdef WIN32
#include <sdkddkver.h>
#include <boost/asio.hpp>
#include <windows.h>
#else
#include <boost/asio.hpp>
#endif //_WINDOWS
#endif

#include <boost/exception/to_string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <boost/fusion/algorithm/auxiliary/copy.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>

#include <nlohmann/json.hpp>
#include <pre/json/from_json.hpp>
#include <pre/json/to_json.hpp>

#include <boost/format.hpp>

#ifdef USE_BOOST_LOCALE
#include <boost/locale.hpp>
#endif

#ifdef USE_BOOST_LOG
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#endif

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <alpaca/alpaca.h>

#ifdef PM_APPLICATION
#define APPLICATION_NAME "ProfileManager"
#endif

using json = nlohmann::json;

#define NIL_STRING_UUID "00000000-0000-0000-0000-000000000000"
#define WS_RESULT_SUCCESS "success"
#define WS_RESULT_ERROR "error"

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::http), http_param,
    (std::string, command)
    (std::string, param)    //base64 строка
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::http), http_conf,
    (std::string, host)
    (std::string, token)
    (std::string, table)
)
BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::client), version_application,
    (int, major)
    (int, minor)
    (int, path)
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), http_query_sql_param,
    (std::string, table_name)
    (std::string, query_type)
)

namespace arcirk{
//#ifdef USE_BOOST_UUIDS
    namespace uuids{

        inline bool is_valid_uuid(std::string const& maybe_uuid, boost::uuids::uuid& result) {
            using namespace boost::uuids;

            try {
                result = string_generator()(maybe_uuid);
                return result.version() != uuid::version_unknown;
            } catch(...) {
                return false;
            }
        }

        inline boost::uuids::uuid random_uuid(){
            return boost::uuids::random_generator()();
        }

        inline boost::uuids::uuid string_to_uuid(const std::string& sz_uuid, bool random_uuid = false) {

            if (sz_uuid == NIL_STRING_UUID && random_uuid)
                return boost::uuids::random_generator()();

            boost::uuids::uuid uuid{};

            if (is_valid_uuid(sz_uuid, uuid)){
                return uuid;
            }

            if (!random_uuid)
                return boost::uuids::nil_uuid();
            else
                return boost::uuids::random_generator()();
        }
    }
//#endif
    typedef unsigned char BYTE;
    typedef std::vector<BYTE> ByteArray;
    typedef std::vector<std::uint8_t> BJson;

    template<typename T>
    inline std::string enum_synonym(T value){
        try {
            return json(value).get<std::string>();
        } catch (...) {
            return std::to_string(value);
        }
    };

    inline std::string byte_array_to_string(const ByteArray& data){
        if(data.empty())
            return "";
        return std::string{data.begin(), data.end()};
    }

    inline ByteArray string_to_byte_array(const std::string& str){
        return ByteArray{str.begin(), str.end()};
    }

    inline void write_file(const std::string& filename, ByteArray& file_bytes){
        std::ofstream file(filename, std::ios::out|std::ios::binary);
        std::copy(file_bytes.cbegin(), file_bytes.cend(),
                  std::ostream_iterator<unsigned char>(file));
    }

    inline void read_file(const std::string &filename, ByteArray &result)
    {
        FILE * fp = fopen(filename.c_str(), "rb");

        fseek(fp, 0, SEEK_END);
        size_t flen= ftell(fp);
        fseek(fp, 0, SEEK_SET);

        std::vector<unsigned char> v (flen);

        fread(&v[0], 1, flen, fp);

        fclose(fp);

        result = v;
    }

    inline std::string type_string(nlohmann::json::value_t t){
        using json = nlohmann::json;
        if(t == json::value_t::null) return "null";
        else if(t == json::value_t::boolean) return "boolean";
        else if(t == json::value_t::number_integer) return "number_integer";
        else if(t == json::value_t::number_unsigned) return "number_unsigned";
        else if(t == json::value_t::number_float) return "number_float";
        else if(t == json::value_t::object) return "object";
        else if(t == json::value_t::array) return "array";
        else if(t == json::value_t::string) return "string";
        else if(t == json::value_t::binary) return "binary";
        else return "undefined";
    }

    inline std::string get_string_from_binary(const json& value){
        std::string result;
        if(value.is_binary()){
            try {
                BJson data = value.get_binary();
                if(!data.empty()){
                    auto str = std::string{data.begin(), data.end()};
                    if(json::accept(str)){
                        auto val = json::parse(str);
                        if(val.is_string())
                            result = val.get<std::string>();
                    }
                }
            } catch (const std::exception &) {
                //
            }
        }else{
            if(value.is_array()){
                try {
                    ByteArray data = value.get<ByteArray>();
                    if(!data.empty()){
                        auto str = std::string{data.begin(), data.end()};
                        if(json::accept(str)){
                            auto val = json::parse(str);
                            if(val.is_string())
                                result = val.get<std::string>();
                        }
                    }
                }        catch (const std::exception &) {
                    //
                }
            }
        }

        return result;
    }

//    inline json array_from_binary(const json& value){
//        if(value.is_binary()){
//            auto data = value.get_binary();
//            return data;
////            try {
////                if(!data.empty()){
////                    auto str = std::string{data.begin(), data.end()};
////                    if(str == "null")
////                        return json::array();
////                    if(json::accept(str)){
////                        auto var = json::parse(str);
////                        if(var.is_array())
////                            return var;
////                        else
////                            return data;
////                    }else
////                        return data;
////                }else
////                    return json::array();
////            }catch(...){
////                return data;
////            }
//        }else
//            return json::array();
//    }

    inline void fill_object(const json& source, json& object){
        if(!source.is_object())
            return;
        if(!object.is_object())
            return;

        for (auto itr = source.items().begin(); itr != source.items().end(); ++itr) {
            if(object.find(itr.key()) != object.end()){
                if(itr.value().type() == object[itr.key()].type())
                    object[itr.key()] = itr.value();
                else{
                    std::cerr << __FUNCTION__  << " key: " << itr.key() << " source type: " <<  type_string(itr.value().type()) << " object type: " << type_string(object[itr.key()].type()) << std::endl;
                    if(object[itr.key()].is_boolean() && itr.value().is_number())
                        object[itr.key()] = itr.value().get<double>() > 0 ? true : false;
                    else if(object[itr.key()].is_number() && itr.value().is_boolean())
                        object[itr.key()] = itr.value().get<bool>() ? 1 : 0;
                    else if(object[itr.key()].is_string() && itr.value().is_number())
                        object[itr.key()] = std::to_string(itr.value().get<double>());
                    else if(object[itr.key()].is_string() && itr.value().is_boolean())
                        object[itr.key()] = std::to_string(itr.value().get<bool>());
                    else if(object[itr.key()].is_string() && itr.value().is_binary()){
                        object[itr.key()] = get_string_from_binary(itr.value());
                    }else if(object[itr.key()].is_string() && itr.value().is_array()){
                        object[itr.key()] = get_string_from_binary(itr.value());
                    }else if(object[itr.key()].is_array() && itr.value().is_binary()){
                        object[itr.key()] =  json::to_cbor(itr.value());// array_from_binary(itr.value());
                    }
                }
            }
        }
    }

    template<typename T>
    inline T secure_serialization(const nlohmann::json &source, const std::string& fun = "")
    {
        using json = nlohmann::json;

        if(!source.is_object())
            return T();

        try {
            auto result = pre::json::from_json<T>(source);
            return result;
        }catch (const std::exception& e){
            std::cerr << __FUNCTION__ << " " << fun << " " << e.what() << std::endl;
        }

        auto object = pre::json::to_json(T());
        fill_object(source, object);
        for (auto itr = object.items().begin(); itr != object.items().end(); ++itr) {
            if(itr.value().is_binary())
                std::cout << " binary: " << itr.key() << std::endl;
        }
        try {
            return pre::json::from_json<T>(object);
        } catch (const std::exception &e) {
            std::cerr << __FUNCTION__  << " " << e.what() << std::endl;
            return T();
        }


//        for (auto it = source.items().begin(); it != source.items().end(); ++it) {
//            if(object.find(it.key()) != object.end()){
//                if(it.value().type() == object[it.key()].type()){
//                    object[it.key()] = it.value();
//                }else{
//                    if(object[it.key()].is_boolean()){
//                        if(it.value().is_number()){
//                            auto val = it.value().get<int>();
//                            object[it.key()] = val == 0 ? false : true; // it.value();
//                        }else
//                            object[it.key()] = false;
//                    }else{
//                        if(it.value().is_number()){
//                            object[it.key()] = it.value();
//                        }else if(it.value().is_binary()){
//                            std::cout << it.key() << std::endl;
//                            if(object[it.key()].is_array())
//                                object[it.key()] = it.value().get_binary();
//                            else{
//                                object[it.key()] = {};
////                                auto ba = it.value().get_binary();
////                                auto v = json::from_cbor(ba);
////                                if(v.type() == object[it.key()].type()){
////                                    object[it.key()] = v;
////                                }
//                            }
//                        }else{
//                            std::cerr << fun << " " << __FUNCTION__ << " Ошибка проверки по типу ключа: " << it.key().c_str() << std::endl;
//                            std::cerr << it.value() << " " << type_string(it.value().type()) << " " << type_string(object[it.key()].type()) <<  std::endl;
//                        }
//                    }
//                }
//            }
//        }
//
//        try {
//            auto r = pre::json::from_json<T>(object);
//            return r;
//        } catch (const std::exception &e) {
//            std::cerr << e.what() << std::endl;
//        }

    }

    template<typename T>
    inline T secure_serialization(const std::string &source, const std::string& fun = "")
    {
        using json = nlohmann::json;
        if(!json::accept(source))
            return T();

        try {
            auto result = secure_serialization<T>(json::parse(source), fun);
            return result;
        } catch (std::exception& e) {
            if(fun.empty())
                std::cerr << __FUNCTION__ << " " << e.what() << std::endl;
            else
                std::cerr << __FUNCTION__ << " " << fun << " " << e.what() << std::endl;
        }
        return T();
    }

    inline int index_of(const std::string& original_string, const std::string& source, int start = 0){
        auto find = original_string.find(source, start);
        if(find == std::string::npos)
            return - 1;
        else
            return (int)find;
    }

    namespace widgets {

        enum item_editor_widget_roles{ //устарела
            widgetLabelRole,
            widgetTextLineRole,
            widgetTextEditRole,
            widgetComboBoxRole,
            widgetCheckBoxRole,
            widgetCompareRole,
            widgetSpinBoxRole,
            widgetVariantRole,
            editorINVALID= -1
        };
        NLOHMANN_JSON_SERIALIZE_ENUM(item_editor_widget_roles, {
            {editorINVALID, nullptr}    ,
            {widgetLabelRole, "widgetLabelRole"}  ,
            {widgetTextLineRole, "widgetTextLineRole"}  ,
            {widgetTextEditRole, "widgetTextEditRole"}  ,
            {widgetComboBoxRole, "widgetComboBoxRole"}  ,
            {widgetCheckBoxRole, "widgetCheckBoxRole"}  ,
            {widgetSpinBoxRole, "widgetSpinBoxRole"}  ,
            {widgetSpinBoxRole, "widgetSpinBoxRole"}  ,
            {widgetVariantRole, "widgetVariantRole"}  ,
        })

        enum editor_inner_role{
            editorNullType,
            editorFilePath,
            editorDirectoryPath,
            editorColor,
            editorText,
            editorMultiText,
            editorByteArray,
            editorNumber,
            editorArray,
            editorBoolean,
            editorDataReference,
            editorInnerRoleINVALID=-1
        };

        NLOHMANN_JSON_SERIALIZE_ENUM(editor_inner_role, {
            {editorInnerRoleINVALID, nullptr}    ,
            {editorNullType, "editorNullType"}  ,
            {editorFilePath, "editorFilePath"}  ,
            {editorDirectoryPath, "editorDirectoryPath"}  ,
            {editorColor, "editorColor"}  ,
            {editorText, "editorText"}  ,
            {editorMultiText, "editorMultiText"}  ,
            {editorByteArray, "editorByteArray"}  ,
            {editorNumber, "editorNumber"}  ,
            {editorArray, "editorArray"}  ,
            {editorBoolean, "editorBoolean"}  ,
            {editorDataReference, "editorDataReference"}  ,
        });

//        enum variant_type{
//            vNull = 0,
//            vJsonDump,
//            vBinary
//        };
//
//        struct variant_p{
//            std::string representation; // представление
//            int  type = 0;  // tree_editor_inner_role
//            int role = 0; // variant_type
//            std::string table; // имя таблицы в базе данных если ссылка в базе данных
//            ByteArray data; // данные
//            //BJson data; // данные
//        };
//
//        inline variant_p variant_p_(){
//            auto v = variant_p();
//            v.representation = "";
//            v.table = "";
//            v.data = {};
//            v.type = 0;
//            v.role = 0;
//            return v;
//        }
//
//        inline ByteArray ref_to_byte(const std::string& uuid = "", const std::string& table = ""){
//            json uuid_{};
//            if(uuid.empty())
//                uuid_ = boost::to_string(boost::uuids::random_generator()());
//            else{
//                auto u = uuids::string_to_uuid(uuid, true);
//                uuid_ = boost::to_string(u);
//            }
//            auto m_raw = variant_p();
//            m_raw.role = tree_editor_inner_role::widgetDataReference;
//            m_raw.type = vJsonDump;
//            m_raw.table = table;
//            m_raw.data = string_to_byte_array(uuid_.dump());
//            m_raw.representation = uuid_.get<std::string>();
//            ByteArray bytes;
//            //BJson bytes;
//            alpaca::serialize(m_raw, bytes);
//            return bytes;
//            //return json::to_bson(pre::json::to_json(m_raw));
//        }
//
//        inline variant_p variant_from_byte(const ByteArray& data){
//            std::error_code ec;
//            auto m_raw = alpaca::deserialize<variant_p>(data, ec);
//            if (!ec) {
//                return m_raw;
//            }else
//                return variant_p_();
//        }

//        inline ByteArray variant_to_byte(const variant_p& value){
//            ByteArray bytes;
//            alpaca::serialize(value, bytes);
//            return bytes;
//        }
//
//        inline ByteArray ref_init(const json& array){
//            ByteArray m_ref;
//            if(array.empty()){
//                m_ref = ref_to_byte();
//            }else{
//                if(array.is_array()){
//                    try {
//                        m_ref = array.get<ByteArray>();
//                        if (sizeof(m_ref.data())  != sizeof(variant_p*)) {
//                            m_ref = ref_to_byte();
//                        }
//                    } catch (const std::exception &) {
//                        m_ref = ref_to_byte();
//                    }
//                }else if(array.is_string()){
//                    auto uuid = uuids::string_to_uuid(array.get<std::string>(), true);
//                    m_ref = ref_to_byte(boost::to_string(uuid));
//                }else
//                    m_ref = ref_to_byte();
//            }
//            return m_ref;
//        }

//        inline variant_p get_variant_p(const json& value){
//            auto m_raw = variant_p_();
//            if(value.is_array()){
//                try {
//                    auto ba = value.get<ByteArray>();
//                    if(sizeof(ba.data()) == sizeof(variant_p*)){
//                        m_raw = variant_from_byte(ba);
//                    }else{
//                        m_raw.role = tree_editor_inner_role::widgetArray;
//                        m_raw.type = vJsonDump;
//                        m_raw.representation = "<array>";
//                    }
//                } catch (const std::exception &) {
//                    m_raw.role = tree_editor_inner_role::widgetArray;
//                    m_raw.type = vJsonDump;
//                    m_raw.representation = "<array>";
//                }
//            }else if (value.is_string()) {
//                auto val = value.get<std::string>();
//                if (index_of(val, "\n") != -1)
//                    m_raw.role = tree_editor_inner_role::widgetMultiText;
//                else
//                    m_raw.role = tree_editor_inner_role::widgetText;
//                m_raw.representation = val;
//            }else if(value.is_number()){
//                m_raw.role = tree_editor_inner_role::widgetNumber;
//                m_raw.representation = boost::to_string(value.get<int>());
//            }else if(value.is_boolean()){
//                m_raw.role = tree_editor_inner_role::widgetBoolean;
//                m_raw.representation = boost::to_string(value.get<bool>());
//            }
//            m_raw.data = string_to_byte_array(value.dump());
//
//            return m_raw;
//        }


}

    namespace date{
        inline tm current_date() {
            using namespace std;
            tm current{};
            time_t t = time(nullptr);
    #ifdef BOOST_WINDOWS
            localtime_s(&current, &t);
    #else
            localtime_r(&t, &current);
    #endif
            return current;
        }

    }

    namespace strings{

        typedef std::string T_str;
        typedef std::vector<T_str> T_vec;
        typedef std::vector<std::pair<T_str, T_str>> T_list;

        inline int split_str_to_vec(const T_str& s, const T_str& DELIM, T_vec& v)
        {
            size_t l, r;

            for (l = s.find_first_not_of(DELIM), r = s.find_first_of(DELIM, l);
                 l != std::string::npos; l = s.find_first_not_of(DELIM, r), r = s.find_first_of(DELIM, l))
                v.push_back(s.substr(l, r - l));
            return (int)v.size();
        }

        inline T_vec split(const T_str& line, const T_str& sep)
        {
            T_vec  v;

            split_str_to_vec(line, sep, v);

            return v;
        }
        inline void trim(std::string& source){ boost::trim(source);};
        inline void to_upper(std::string& source){boost::to_upper(source);};
        inline void to_lower(std::string& source){boost::to_lower(source);};
        inline std::string sample(const std::string& format_string, const std::vector<std::string>& args){
            boost::format f(format_string);
            for (auto it = args.begin(); it != args.end(); ++it) {
                f % *it;
            }
            return f.str();
        }
        template<typename... Arguments>
        inline std::string str_sample(const std::string& format_string, const Arguments&... args){return boost::str((boost::format(format_string) % ... % args));}
        inline std::string left(const std::string &source, const std::string::size_type& count){
            return  source.substr(0, count);
        }
        inline std::string right(const std::string &source, const int& count){
            return  source.substr((int)source.length() - count, count);
        }

#ifdef USE_BOOST_LOCALE
    inline std::string to_utf(const std::string& source, const std::string& charset = "Windows-1251"){
    #ifdef BOOST_WINDOWS
        return boost::locale::conv::to_utf<char>(source, charset);
    #else
        return source;
    #endif
    }
    inline std::string from_utf(const std::string& source, const std::string& charset = "Windows-1251"){
    #ifdef BOOST_WINDOWS
        return boost::locale::conv::from_utf(source, charset);
    #else
        return source;
    #endif
    }
#endif

    }

#ifdef USE_BOOST_LOG
namespace log{
        enum log_level{
            level_info,
            level_warning,
            level_error
        };
        inline void message(const std::string& function, const std::string& text, log_level level){
            switch (level) {
                case level_info:
                    BOOST_LOG_TRIVIAL(info) << function << text;
                case level_warning:
                    BOOST_LOG_TRIVIAL(warning) << function << text;
                case level_error:
                    BOOST_LOG_TRIVIAL(error) << function << text;
                default:
                    BOOST_LOG_TRIVIAL(info) << function << text;
            }

        }
    }
#endif

    class NativeException : public std::exception
    {
    public:
        explicit NativeException(const char *msg) : message(msg) {}
        virtual ~NativeException() throw() {}
        virtual const char *what() const throw() { return message.c_str(); }
    protected:
        const std::string message;
    };
}

#ifdef USE_LIB_TASK

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::tasks), task_options,
    (std::string, uuid)
    (std::string, name)
    (std::string, synonum)
    (bool, predefined)
    (int, start_task)
    (int, end_task)
    (int, interval)
    (int, type_script)
    (bool, allowed)
    (std::string, days_of_week)
    (std::string, days_of_month)
    (arcirk::BJson, script)
    (std::string, script_synonum)
    (std::string, comment)
    (arcirk::BJson, param)
    (std::string, cron_string)
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::plugins), tasks_param,
    (std::string, key)
    (arcirk::BJson, value)
    (int, is_group)
    (std::string, ref)
    (std::string, parent)
)
enum task_script_type{
    script_not_use,
    script_plugin,
    script_python,
    script_onescript,
    script_javascript,
    script_INVALID =-1
};

NLOHMANN_JSON_SERIALIZE_ENUM(task_script_type, {
    {script_INVALID, nullptr},
    {script_not_use, "Не используется"},
    {script_plugin, "Плагин"},
    {script_python, "Python"},
    {script_onescript, "OneScript"},
    {script_javascript, "JavaScript"},
    });

#endif

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::widgets), header_item,
    (std::string, name)
    (std::string, alias)
    (std::string, format)
    (arcirk::BJson, selection_list)
    (int, default_type)
    (arcirk::BJson, default_value)
    (bool, marked)
    (bool, not_public)
    (bool, override_buttons)
    (bool, select_type)
)

namespace arcirk::widgets{

    struct header_item_wrapper{
        std::string name;
        std::string alias;
        std::string format;
        arcirk::BJson selection_list{};
        int default_type = -1;
        arcirk::BJson default_value{};
        bool marked = false;
        bool not_public = false;
        bool override_buttons = false;
        bool select_type = false;
    };

    inline header_item_wrapper h_item_wrapper(){
        auto item = header_item_wrapper();
        item.name = "";
        item.alias = "";
        item.format = "";
        item.selection_list = {};
        item.default_type = -1;
        item.default_value = {};
        item.marked = false;
        item.not_public = false;
        item.override_buttons = false;
        return item;
    }

    inline header_item_wrapper h_item_wrapper(const header_item& source){
        auto item = header_item_wrapper();
        item.name = source.name;
        item.alias = source.alias;
        item.format = source.format;
        item.selection_list = source.selection_list;
        item.default_type = source.default_type;
        item.default_value = source.default_value;
        item.marked = source.marked;
        item.not_public = source.not_public;
        item.override_buttons = source.override_buttons;
        item.select_type = source.select_type;
        return item;
    }

    inline header_item to_header_item(const header_item_wrapper& item){
        return header_item(item.name,
                           item.alias,
                           item.format,
                           item.selection_list,
                           item.default_type,
                           item.default_value,
                           item.marked,
                           item.not_public,
                           item.override_buttons,
                           item.select_type
                           );
    }

    inline header_item header_item_def(const std::string& name, const std::string& alias = {}){
        return header_item(name, alias, "", {}, -1, {}, false, false, false, true);
    }

    template<class T>
    inline auto find_element_for_name(const std::string& name, const std::vector<T>& array){
        return std::find_if(array.begin(), array.end(), [name](const T& itr){
            return itr.name == name;
        });
    }

    template <class T>
    inline size_t index_of_for_name(const std::string& name, const std::vector<T>& array)
    {
        auto it = find_element_for_name(name, array);
        if (it == array.end())
        {
            return -1;
        }
        else
        {
            return std::distance(array.begin(), it);
        }
    }

    template <typename T>
    inline void move_element(std::vector<T>& v, size_t index, size_t new_index)
    {
        auto item = v[index] ;
        v.erase(v.begin() + index);
        v.insert(v.begin() + new_index, item);
    }
}

#ifdef USE_SYSTEM_USER_DATA
    BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::cryptography), win_user_info,
        (std::string, user)
        (std::string, sid)
    )

    BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::cryptography), cert_info,
        (std::string, serial)
        (std::string, issuer)
        (std::string, subject)
        (std::string, not_valid_before)
        (std::string, not_valid_after)
        (arcirk::BJson, data)
        (std::string, sha1)
        (std::string, suffix)
        (std::string, cache)
        (std::string, private_key)
    )

    BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::cryptography), cont_info,
        (arcirk::ByteArray, header_key)
        (arcirk::ByteArray, masks_key)
        (arcirk::ByteArray, masks2_key)
        (arcirk::ByteArray, name_key)
        (arcirk::ByteArray, primary_key)
        (arcirk::ByteArray, primary2_key)
    )

    #define FAT12          "FAT12"
    #define REGISTRY       "REGISTRY"
    #define HDIMAGE        "HDIMAGE"
    #define DATABASE       "DATABASE"
    #define REMOTEBASE     "REMOTEBASE"

    namespace arcirk::command_line {
        enum CmdCommand {
            echoSystem,
            echoUserName,
            wmicGetSID,
            echoGetEncoding,
            csptestGetConteiners,
            csptestContainerCopy,
            csptestContainerFnfo,
            csptestContainerDelete,
            csptestGetCertificates,
            certutilGetCertificateInfo,
            certmgrInstallCert,
            certmgrExportlCert,
            certmgrDeletelCert,
            certmgrGetCertificateInfo,
            cryptcpCopycert,
            mstscAddUserToConnect,
            mstscEditFile,
            quserList,
            mstscRunAsAdmin,
            cmdCD,
            cmdEXIT,
            wmicUserAccount,
            wmicUsers,
            COMMAND_INVALID = -1,
        };
}
    BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), cert_users,
        (std::string, name)
        (std::string, synonym)
        (arcirk::BJson, ref)
        (arcirk::BJson, cache)
        (arcirk::BJson, uuid)
        (std::string, sid)
        (std::string, system_user)
        (std::string, host)
        (arcirk::BJson, parent)
        (bool, is_group)
        (bool, deletion_mark)
        (int, version)
    );
    BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), available_certificates,
        (std::string, name)
        (std::string, synonym)
        (arcirk::BJson, ref)
        (arcirk::BJson, user)
        (arcirk::BJson, cert)
        (bool, is_group)
        (bool, deletion_mark)
        (int, version)
    );

    BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), containers,
        (std::string, name)
        (std::string, synonym)
        (arcirk::BJson, ref)
        (arcirk::BJson, cache)
        (arcirk::BJson, data)
        (std::string, subject)
        (std::string, issuer)
        (std::string, not_valid_before)
        (std::string, not_valid_after)
        (std::string, parent_user)
        (std::string, sha1)
        (arcirk::BJson, parent)
        (bool, is_group)
        (bool, deletion_mark)
        (int, version)
    );

    BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), database_config,
        (int, _id)
        (std::string, name)
        (arcirk::BJson, ref)
        (int, version)
    );

    BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), profile_item,
        (std::string, profile)
        (std::string, name)
        (std::string, url)
        (arcirk::BJson, ref)
        (arcirk::BJson, icon)
        (int, pos)
    );

    BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), application_config,
        (arcirk::BJson, ref)
        (std::string, name)
        (std::string, firefox)
        (std::string, server)
        (std::string, user)
        (std::string, hash)
        (bool, use_sid)
        (bool, def_password)
        (bool, autoreconnect)
        (bool, log_arhive)
    );

    BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), certificates,
        (std::string, name)
        (std::string, synonym)
        (arcirk::BJson, ref)
        (arcirk::BJson, cache)
        (arcirk::BJson, data)
        (std::string, private_key)
        (std::string, subject)
        (std::string, issuer)
        (std::string, not_valid_before)
        (std::string, not_valid_after)
        (std::string, parent_user)
        (std::string, serial)
        (std::string, suffix)
        (std::string, sha1)
        (arcirk::BJson, parent)
        (bool, is_group)
        (bool, deletion_mark)
        (int, version)
    );

namespace arcirk::cryptography{

#ifdef USE_BOOST_LOCALE

    inline void* crypt_t(void* data, unsigned data_size, void* key, unsigned key_size)
    {
        assert(data && data_size);
        if (!key || !key_size) return data;

        auto* kptr = (uint8_t*)key; // начало ключа
        uint8_t* eptr = kptr + key_size; // конец ключа

        for (auto* dptr = (uint8_t*)data; data_size--; dptr++)
        {
            *dptr ^= *kptr++;
            if (kptr == eptr) kptr = (uint8_t*)key; // переход на начало ключа
        }
        return data;
    }
    inline std::string crypt(const std::string &source, const std::string& key) {
        if(source.empty())
            return {};
        try {
#ifdef _WIN32
            std::string s  = arcirk::strings::from_utf(source);
            std::string p  = arcirk::strings::from_utf(key);
            std::vector<char> source_(s.c_str(), s.c_str() + s.size() + 1);
            std::vector<char> key_(p.c_str(), p.c_str() + p.size() + 1);
            void* text = std::data(source_);
            void* pass = std::data(key_);
            crypt_t(text, source_.size(), pass, (unsigned)key_.size());
            std::string result(arcirk::strings::to_utf((char*)text));
            return result;
#else
            //#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))
        std::vector<char> source_(source.c_str(), source.c_str() + source.size() + 1);
        std::vector<char> key_(key.c_str(), key.c_str() + key.size() + 1);
        void* text = std::data(source_);
        void* pass = std::data(key_);
        //crypt_t(text, ARR_SIZE(source.c_str()), pass, ARR_SIZE(key.c_str()));
        crypt_t(text, source_.size(), pass, key_.size());
        std::string result((char*)text);
        return result;
#endif
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            return {};
        }

    }

#endif

    enum TypeOfStorgare{
        storgareTypeRegistry,
        storgareTypeLocalVolume,
        storgareTypeDatabase,
        storgareTypeRemoteBase,
        storgareTypeUnknown = -1
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(TypeOfStorgare, {
        {storgareTypeRegistry, REGISTRY},
        {storgareTypeLocalVolume, FAT12},
        {storgareTypeDatabase, DATABASE},
        {storgareTypeRemoteBase, REMOTEBASE},
    })

    inline TypeOfStorgare type_storgare(const std::string& source){
        if(source.empty())
            return storgareTypeUnknown;
        else{
            if(index_of(source, FAT12) != -1 || index_of(source, HDIMAGE) != -1)
                return storgareTypeLocalVolume;
            else if(index_of(source, REGISTRY) != -1)
                return storgareTypeRegistry;
            else if(index_of(source, DATABASE) != -1)
                return storgareTypeDatabase;
            else if(index_of(source, REMOTEBASE) != -1)
                return storgareTypeDatabase;
            else{
                return storgareTypeUnknown;
            }
        }
    }

}
namespace arcirk::database {

    enum tables {
        tbDatabaseConfig,
        tbHttpAddresses,
        tbApplicationConfig,
        tbCertificates,
        tbContainers,
        tbAvailableCertificates,
        tables_INVALID = -1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(tables, {
        {tables_INVALID, nullptr },
        {tbDatabaseConfig, "DatabaseConfig"}  ,
        {tbHttpAddresses, "HttpAddresses"}  ,
        {tbApplicationConfig, "ApplicationConfig"}  ,
        {tbCertificates, "Certificates" },
        {tbContainers, "Containers" },
        {tbAvailableCertificates, "AvailableCertificates" },
    })

    enum views{
        views_INVALID=-1,
    };

}

//BOOST_FUSION_ADAPT_STRUCT(
//    arcirk::widgets::variant_p,
//    (std::string, representation)
//    (int, type)
//    (int, role)
//    (std::string, table)
//    (arcirk::ByteArray, data)
//)
//
//template<std::size_t...Is, class Tup>
//inline arcirk::widgets::variant_p to_adapt_struct_aux(std::index_sequence<Is...>, Tup&& tup) {
//  using std::get;
//  return {get<Is>(std::forward<Tup>(tup))...};
//}
//
//template<class T, class Tup>
//inline T to_adapt_struct(Tup&& tup) {
//  using t=std::remove_reference_t<Tup>;
//  return to_adapt_struct_aux(
//    std::make_index_sequence<std::tuple_size<T>{}>{},
//    std::forward<Tup>(tup)
//  );
//}
//
//template<class T, std::size_t...is>
//auto to_tuple_aux( std::index_sequence<is...>, T const& f ) {
//    using boost::fusion::at_c;
//    return std::make_tuple(at_c<is>(f)...);
//}
//
//template<class T>
//auto struct_to_tuple(T const& f){
//    using t=std::remove_reference_t<T>;
//    return to_tuple_aux(
//            std::make_index_sequence<boost::fusion::result_of::size<T>::type::value>{},
//            f
//    );
//}

#endif

#endif //ARCIRK_GROUP_ARCIRK_HPP