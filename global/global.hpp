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
#include <boost/uuid/detail/md5.hpp>
#include <boost/algorithm/hex.hpp>

#include <alpaca/alpaca.h>

#ifdef PM_APPLICATION
#define APPLICATION_NAME "ProfileManager"
#endif

using json = nlohmann::json;

#define NIL_STRING_UUID "00000000-0000-0000-0000-000000000000"
#define WS_RESULT_SUCCESS "success"
#define WS_RESULT_ERROR "error"
#define DEFAULT_CHARSET_ "CP866"
#define DEFAULT_CHARSET_WIN "CP1251"
#define CRYPT_KEY "my_key"
#define UNDEFINED std::monostate()

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::https), http_param,
    (std::string, command)
    (std::string, param)    //base64 строка
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::https), http_conf,
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

    inline std::string digest_to_string(const boost::uuids::detail::md5::digest_type &digest)
    {
        const auto charDigest = reinterpret_cast<const char *>(&digest);
        std::string result;
        boost::algorithm::hex(charDigest, charDigest + sizeof(boost::uuids::detail::md5::digest_type), std::back_inserter(result));
        return result;
    }

    inline std::string  to_md5(const std::string& source){
        using boost::uuids::detail::md5;
        md5 hash;
        md5::digest_type digest;

        hash.process_bytes(source.data(), source.size());
        hash.get_digest(digest);
        return digest_to_string(digest);
    }

    inline boost::uuids::uuid md5_to_uuid(const std::string& md5_string){
        using boost::uuids::detail::md5;
        using namespace boost::uuids;
        try {
            std::string source = md5_string;
            source = source.insert(8, "-");
            source = source.insert(13, "-");
            source = source.insert(18, "-");
            source = source.insert(23, "-");
            auto result = string_generator()(source);
            return result;
        } catch(...) {
            return boost::uuids::nil_uuid();
        }
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
            widgetIpAddress,
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
            {widgetIpAddress, "widgetIpAddress"}  ,
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
            editorIpAddress,
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
            {editorIpAddress, "editorIpAddress"}  ,
        });
}

    namespace date{

        static constexpr time_t const NULL_TIME = -1;

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

        inline long int tz_offset(time_t when = NULL_TIME)
        {
            if (when == NULL_TIME)
                when = std::time(nullptr);
            auto const tm = *std::localtime(&when);
            std::ostringstream os;
            os << std::put_time(&tm, "%z");
            std::string s = os.str();
            // s is in ISO 8601 format: "±HHMM"
            int h = std::stoi(s.substr(0,3), nullptr, 10);
            int m = std::stoi(s[0]+s.substr(3), nullptr, 10);

            return (h-1) * 3600 + m * 60;
        }

        inline long int date_to_seconds(const tm& dt = {}, bool offset = false){

            tm current = dt;
            time_t t = time(nullptr);

#ifdef _WIN32
            localtime_s(&current, &t);
#else
            localtime_r(&t, &current);
#endif

            std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(mktime(&current));

            auto i_offset = offset ? tz_offset() : 0;

            return
                    (long int)std::chrono::duration_cast<std::chrono::seconds>(
                            tp.time_since_epoch()).count() + i_offset;

        }

        inline long int add_day(const long int& dt, const int& quantity){
            return dt + (quantity * (60*60*24));
        }

        inline long int start_day(const std::tm& d){
            auto sec = d.tm_sec + (d.tm_min * 60) + (d.tm_hour * 60 * 60);
            return date_to_seconds(d, false) - sec;
        }
        inline long int end_day(const std::tm& d){
            auto sec = 60 * 60 * 24 - 1;
            return start_day(d) + sec;
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

#ifdef USE_BASE64_UTILS
    namespace base64{
        bool byte_is_base64(BYTE c);
        std::string byte_to_base64(BYTE const* buf, unsigned int bufLen);
        ByteArray base64_to_byte(std::string const& encoded_string);
        std::string base64_encode(const std::string &s);
        std::string base64_decode(const std::string &s);
        bool is_base64(const std::string& source);
    }
#endif

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
    (int, use)
)

namespace arcirk::widgets{

    enum attribute_use{
        for_group_and_element = 0,
        for_element,
        for_group
    };

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
        int use = 0;
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
        item.use = 0;
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
        item.use = source.use;
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
                           item.select_type,
                           item.use
                           );
    }

    inline header_item header_item_def(const std::string& name, const std::string& alias = {}){
        return header_item(name, alias, "", {}, -1, {}, false, false, false, true, 0);
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
            csptestGetContainers,
            csptestContainerCopy,
            csptestContainerInfo,
            csptestContainerDelete,
            csptestGetCertificates,
            certutilGetCertificateInfo,
            certmgrInstallCert,
            certmgrExportCert,
            certmgrDeleteCert,
            certmgrGetCertificateInfo,
            cryptcpCopyCert,
            mstscAddUserToConnect,
            mstscEditFile,
            quserList,
            mstscRunAsAdmin,
            cmdCD,
            cmdEXIT,
            wmicUserAccount,
            wmicUsers,
            COMMAND_INVALID=-1,
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
        (arcirk::BJson, client_ref)
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
        (arcirk::BJson, client_ref)
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
        (arcirk::BJson, client_ref)
    );

    BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), database_config,
        (int, _id)
        (std::string, name)
        (arcirk::BJson, ref)
        (int, version)
        (arcirk::BJson, client_ref)
    );

    BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), profile_item,
        (std::string, profile)
        (std::string, name)
        (std::string, url)
        (arcirk::BJson, ref)
        (arcirk::BJson, icon)
        (int, pos)
        (arcirk::BJson, client_ref)
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
        (bool, allow_certificates)
        (bool, allow_mstsc)
        (bool, allow_mstsc_users)
        (arcirk::BJson, client_ref)
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
        (arcirk::BJson, client_ref)
    );
    BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), mstsc_item,
        (arcirk::BJson, ref)
        (std::string, name)
        (std::string, address)
        (std::string, user)
        (int, port)
        (bool, def_port)
        (bool, not_full_window)
        (int, width)
        (int, height)
        (bool, reset_user)
        (arcirk::BJson, parent)
        (bool, is_group)
        (bool, deletion_mark)
        (bool, predefined)
        (int, version)
        (int, pos)
        (arcirk::BJson, client_ref)
    );

    BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), messages,
        (arcirk::BJson, user_uuid)
        (arcirk::BJson, receiver_uuid)
        (arcirk::BJson, ref)
        (std::string, message)
        (std::string, token)
        (int, date)
        (std::string, content_type)
        (int, unread_messages)
        (arcirk::BJson, parent)
        (bool, is_group)
        (bool, deletion_mark)
        (int, version)
    );

    BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), servers,
        (std::string, name)
        (std::string, host)
        (std::string, user)
        (std::string, hash)
        (arcirk::BJson, uuid)
        (arcirk::BJson, ref)
    );

namespace arcirk::server{

    enum server_commands{
        ServerVersion, //Версия сервера
        ServerOnlineClientsList, //Список активных пользователей
        SetClientParam, //Параметры клиента
        ServerConfiguration, //Конфигурация сервера
        UserInfo, //Информация о пользователе (база данных)
        InsertOrUpdateUser, //Обновить или добавить пользователя (база данных)
        CommandToClient, //Команда клиенту (подписчику)
        ServerUsersList, //Список пользователей (база данных)
        ExecuteSqlQuery, //выполнить запрос к базе данных
        GetMessages, //Список сообщений
        UpdateServerConfiguration, //Обновить конфигурацию сервера
        HttpServiceConfiguration, //Получить конфигурацию http сервиса 1С
        InsertToDatabaseFromArray, //Добавить массив записей в базу //устарела удалить
        SetNewDeviceId, //Явная установка идентификатора на устройствах где не возможно его получить
        ObjectSetToDatabase, //Синхронизация объекта 1С с базой
        ObjectGetFromDatabase, //Получить объект типа 1С из базы данных для десериализации
        SyncGetDiscrepancyInData, //Получить расхождения в данных между базами на клиенте и на Сервере
        SyncUpdateDataOnTheServer, //Обновляет данные на сервере по запросу клиента
        WebDavServiceConfiguration, //Получить настройки WebDav
        SyncUpdateBarcode, //синхронизирует на сервере штрихкод и номенклатуру по запросу клиента с сервером 1с
        DownloadFile, //Загружает файл на сервер
        GetInformationAboutFile, //получить информацию о файле
        CheckForUpdates, //поиск фалов обрновления
        UploadFile, //скачать файл
        GetDatabaseTables,
        FileToDatabase, //Загрузка таблицы базы данных из файла
        ProfileDirFileList,
        ProfileDeleteFile,
        DeviceGetFullInfo,
        GetTasks,
        UpdateTaskOptions,
        TasksRestart,
        RunTask,
        StopTask,
        SendNotify,
        GetCertUser,
        VerifyAdministrator,
        UserMessage,
        GetChannelToken,
        IsChannel,
        GetDatabaseStructure,
        Run1CScript,
        CreateDirectories,
        DeleteDirectory,
        bDeleteFile,
        CMD_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(server_commands, {
        {CMD_INVALID, nullptr}    ,
        {ServerVersion, "ServerVersion"}  ,
        {ServerOnlineClientsList, "ServerOnlineClientsList"}    ,
        {SetClientParam, "SetClientParam"}    ,
        {ServerConfiguration, "ServerConfiguration"}    ,
        {UserInfo, "UserInfo"}    ,
        {InsertOrUpdateUser, "InsertOrUpdateUser"}    ,
        {CommandToClient, "CommandToClient"}    ,
        {ServerUsersList, "ServerUsersList"}    ,
        {ExecuteSqlQuery, "ExecuteSqlQuery"}    ,
        {GetMessages, "GetMessages"}    ,
        {UpdateServerConfiguration, "UpdateServerConfiguration"}    ,
        {HttpServiceConfiguration, "HttpServiceConfiguration"}    ,
        {InsertToDatabaseFromArray, "InsertToDatabaseFromArray"}    ,
        {SetNewDeviceId, "SetNewDeviceId"}    ,
        {ObjectSetToDatabase, "ObjectSetToDatabase"}    ,
        {ObjectGetFromDatabase, "ObjectGetFromDatabase"}    ,
        {SyncGetDiscrepancyInData, "SyncGetDiscrepancyInData"}    ,
        {SyncUpdateDataOnTheServer, "SyncUpdateDataOnTheServer"}    ,
        {WebDavServiceConfiguration, "WebDavServiceConfiguration"}    ,
        {SyncUpdateBarcode, "SyncUpdateBarcode"}    ,
        {DownloadFile, "DownloadFile"}    ,
        {GetInformationAboutFile, "GetInformationAboutFile"}    ,
        {CheckForUpdates, "CheckForUpdates"}    ,
        {UploadFile, "UploadFile"}    ,
        {GetDatabaseTables, "GetDatabaseTables"}    ,
        {FileToDatabase, "FileToDatabase"}    ,
        {ProfileDirFileList, "ProfileDirFileList"}    ,
        {ProfileDeleteFile, "ProfileDeleteFile"}    ,
        {DeviceGetFullInfo, "DeviceGetFullInfo"}    ,
        {GetTasks, "GetTasks"}    ,
        {UpdateTaskOptions, "UpdateTaskOptions"}    ,
        {TasksRestart, "TasksRestart"}    ,
        {RunTask, "RunTask"}    ,
        {StopTask, "StopTask"}    ,
        {SendNotify, "SendNotify"}    ,
        {GetCertUser, "GetCertUser"}    ,
        {VerifyAdministrator, "VerifyAdministrator"}    ,
        {UserMessage, "UserMessage"}    ,
        {GetChannelToken, "GetChannelToken"}    ,
        {IsChannel, "IsChannel"}    ,
        {GetDatabaseStructure, "GetDatabaseStructure"}    ,
        {Run1CScript, "Run1CScript"}    ,
        {CreateDirectories, "CreateDirectories"}    ,
        {DeleteDirectory, "DeleteDirectory"}    ,
        {bDeleteFile, "DeleteFile"}    ,
    })

}

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
            crypt_t(text, (unsigned)source_.size(), pass, (unsigned)key_.size());
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

    enum TypeOfStorage{
        storageTypeRegistry,
        storageTypeLocalVolume,
        storageTypeDatabase,
        storageTypeRemoteBase,
        storageTypeUnknown = -1
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(TypeOfStorage, {
        {storageTypeUnknown, "UNKNOWN"},
        {storageTypeRegistry, REGISTRY},
        {storageTypeLocalVolume, FAT12},
        {storageTypeDatabase, DATABASE},
        {storageTypeRemoteBase, REMOTEBASE},
    })

    inline TypeOfStorage type_storage(const std::string& source){
        if(source.empty())
            return storageTypeUnknown;
        else{
            if(index_of(source, FAT12) != -1 || index_of(source, HDIMAGE) != -1)
                return storageTypeLocalVolume;
            else if(index_of(source, REGISTRY) != -1)
                return storageTypeRegistry;
            else if(index_of(source, DATABASE) != -1)
                return storageTypeDatabase;
            else if(index_of(source, REMOTEBASE) != -1)
                return storageTypeDatabase;
            else{
                return storageTypeUnknown;
            }
        }
    }

}
namespace arcirk::database {

    enum roles{
        dbUser,
        dbAdministrator,
        roles_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(roles, {
        {roles_INVALID, nullptr}    ,
        {dbAdministrator, "admin"}  ,
        {dbUser, "user"}  ,
    })

    enum text_type{
        dbText,
        dbHtmlText,
        dbXmlText,
        dbJsonText,
        text_type_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(text_type, {
        {text_type_INVALID, nullptr}    ,
        {dbText, "Text"}  ,
        {dbHtmlText, "HtmlText"}  ,
        {dbXmlText, "XmlText"}  ,
        {dbJsonText, "JsonText"}  ,
    })

    enum devices_type{
        devDesktop,
        devServer,
        devPhone,
        devTablet,
        devExtendedLib,
        dev_INVALID=-1
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(devices_type, {
        {dev_INVALID, nullptr},
        {devDesktop, "Desktop"},
        {devServer, "Server"},
        {devPhone, "Phone"},
        {devTablet, "Tablet"},
        {devExtendedLib, "ExtendedLib"},
    })
    enum tables {
        tbDatabaseConfig,
        tbHttpAddresses,
        tbApplicationConfig,
        tbCertificates,
        tbContainers,
        tbAvailableCertificates,
        tbMstscConnections,
        tbServerConfig,
        tbServers,
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
        {tbMstscConnections, "MstscConnections" },
        {tbServerConfig, "ServerConfig" },
        {tbServers, "Servers" },
    })

    enum views{
        views_INVALID=-1,
    };

}

#endif


#endif //ARCIRK_GROUP_ARCIRK_HPP
