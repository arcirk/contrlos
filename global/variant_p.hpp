//
// Created by admin on 06.02.2024.
//

#ifndef CONTROLSPROG_VARIANT_P_HPP
#define CONTROLSPROG_VARIANT_P_HPP

#include "global.hpp"
#include <QVariant>
#include <QUuid>
#include <QByteArray>

using namespace arcirk;
using namespace arcirk::widgets;

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk), variant_p,
    (std::string, representation) //инициализируется при создании или изменении data
    (int, editor_role) //нужен для редактора в диалоге либо в делегате
    (std::string, table) //нужен при явном указании на ссылку в базе данных
    (BJson, data) // данные std::vector<std::uint8_t> в формате bjson
)

typedef std::pair<std::string, arcirk::variant_p> value_pair;
typedef std::vector<value_pair> variant_map;

namespace arcirk{
    enum variant_subtype{
        subtypeNull = 0,
        subtypeDump,
        subtypeArray,
        subtypeByte,
        subtypeRef,
        subtypeObject
    };
    inline json to_binary(const json& value, variant_subtype subtype = variant_subtype::subtypeNull){
        auto ba = json::to_cbor(value);
        return json::binary(ba, subtype);
    }

    inline json to_binary(const QUuid& value){
        auto ba = value.toRfc4122();
        BJson ba_j = BJson(ba.size());
        std::copy(ba.begin(), ba.end(), ba_j.begin());
        return to_binary(ba_j, subtypeRef);
    }

    inline json to_binary(const QString& value){
        return to_binary(value.toStdString(), subtypeDump);
    }

    inline json to_binary(const char* value){
        return to_binary(value, subtypeDump);
    }

    inline json from_binary(const json& value, variant_subtype& subtype){
        if(!value.is_binary())
            return value;
        auto ba_j = value.get_binary();
        subtype = (variant_subtype)ba_j.subtype();
        auto val = json::from_cbor(ba_j);
        return val;
    }

    inline BJson to_byte(const json& value){
        return json::to_cbor(value.dump());
    }

    inline json from_byte(const BJson& value){
        try {
            auto v_ba = json::from_cbor(value);
            auto bi_object = json::parse(v_ba.get<std::string>());
            auto bytes = bi_object["bytes"].get<BJson>();
            auto sub = bi_object["subtype"].get<int>();
            return json::binary(bytes, sub);
        } catch (...) {
            return {};
        }
    }

    inline bool is_binary(const json& value){
        if(value.is_binary())
            return true;
        if(!value.is_array())
            return false;
        BJson ba{};
        try {
            ba = value.get<BJson>();
        } catch (...) {
            return false;
        }
        return !from_byte(ba).empty();
    }


    inline variant_subtype subtype(const json& value){
        if(value.is_binary()){
            return (variant_subtype)value.get_binary().subtype();
        }else if(value.is_array()){
            return subtypeArray;
        }else
            return subtypeDump;
    }

    inline variant_subtype subtype(const arcirk::variant_p& value){
        auto val = from_byte(value.data);
        if(!val.empty() && val.is_binary())
            return (variant_subtype)val.get_binary().subtype();
        else
            return subtypeNull;
    }

    inline std::string array_to_string(const json& value){
        if(!value.is_array())
            return {};
        else{
            std::vector<std::string> result;
            for (auto itr = value.begin(); itr != value.end(); ++itr) {
                auto value_ = *itr;
                if(value_.type() == json::value_t::string){
                    result.push_back(value_.get<std::string>());
                }else if(value_.type() == json::value_type::value_t::boolean){
                    result.push_back(std::to_string(value_.get<bool>()));
                }else if(value_.type() == json::value_type::value_t::number_float){
                    result.push_back(std::to_string(value_.get<double>()));
                }else if(value_.type() == json::value_type::value_t::number_integer){
                    result.push_back(std::to_string(value_.get<int>()));
                }else
                    result.push_back(value_.type_name());
            }
            return boost::join(result, ",");
        }
    }

    inline std::string representation(const json& value){

        if(value.type() == json::value_t::string){
            return value.get<std::string>();
        }else if(value.type() == json::value_type::value_t::boolean){
            return std::to_string(value.get<bool>());
        }else if(value.type() == json::value_type::value_t::number_float){
            return std::to_string(value.get<double>());
        }else if(value.type() == json::value_type::value_t::number_integer){
            return std::to_string(value.get<long long>());
        }else if(value.type() == json::value_type::value_t::array){
            if(is_binary(value)){
                variant_subtype type;
                auto val = from_binary(from_byte(value.get<BJson>()), type);
                if(type == subtypeRef){
                    QUuid uuid = QUuid::fromRfc4122(val.get<BJson>());
                    return uuid.toString(QUuid::WithoutBraces).toStdString();
                }else if(type == subtypeArray)
                    return array_to_string(val);
                else if(type == subtypeByte)
                    return "<binary data>";
                else
                    return {};
            }else{
                return "<array>";
            }
        }else if(value.type() == json::value_type::value_t::object){
            return "<object>";
        }return {};

    }
    inline arcirk::variant_p to_variant_p(const json& value){
        auto result = arcirk::variant_p();
        result.representation = representation(value);
        if(!is_binary(value))
            result.data = to_byte(to_binary(value, subtype(value)));
        else
            result.data = value.get<BJson>();
        return result;
    }

    inline value_pair to_value_pair(const std::string& key, const json& value){
        return std::make_pair(key, to_variant_p(value));
    }

    inline value_pair to_value_pair(const std::string& key, const variant_p& value){
        return std::make_pair(key, value);
    }

    inline QVariant to_variant(const BJson& value){
        auto var = from_byte(value);
        if(var.is_binary()){
            variant_subtype type;
            auto val = from_binary(var, type);
            if(val.is_string())
                return val.get<std::string>().c_str();
            else if(val.is_boolean())
                return val.get<bool>();
            else if(val.is_number_float())
                return val.get<double>();
            else if(val.is_number_integer())
                return val.get<int>();
            else if(val.is_array())
                return QByteArray(reinterpret_cast<const char*>(value.data()), (qsizetype)value.size());
            else if(val.is_binary())
                return QByteArray(reinterpret_cast<const char*>(value.data()), (qsizetype)value.size());
            else if(val.is_object())
                return QByteArray(reinterpret_cast<const char*>(value.data()), (qsizetype)value.size());
            else
                return {};
        }else
            return {};
    }

    inline json to_json(const BJson& value){
        auto var = from_byte(value);
        if(var.is_binary()){
            variant_subtype type;
            auto val = from_binary(var, type);
            if(val.is_string())
                return val.get<std::string>().c_str();
            else if(val.is_boolean())
                return val.get<bool>();
            else if(val.is_number_float())
                return val.get<double>();
            else if(val.is_number_integer())
                return val.get<int>();
            else if(val.is_array())
                return value;
            else if(val.is_binary())
                return value;
            else if(val.is_object())
                return value;
            else
                return {};
        }else
            return {};
    }

    inline variant_map object_to_map(const json& object){
        if(!object.is_object())
            return {};
        variant_map result{};
        for (auto itr = object.items().begin(); itr != object.items().end(); ++itr) {
            result.push_back(to_value_pair(itr.key(), itr.value()));
        }
        return result;
    }

    template<class T>
    inline variant_map struct_to_map(const T& value){
        auto object = pre::json::to_json(value);
        variant_map result{};
        for (auto itr = object.items().begin(); itr != object.items().end(); ++itr) {
            result.push_back(to_value_pair(itr.key(), itr.value()));
        }
        return result;
    }

    template<class T>
    inline T map_to_struct(const variant_map& value){
        auto result = pre::json::to_json(T());
        for (auto itr = result.items().begin(); itr != result.items().end(); ++itr) {
            std::string key = itr.key();
            auto exists = std::find_if(value.begin(), value.end(), [key](const value_pair& it){
                return it.first == key;
            });
            if(exists != value.end()){
                auto var = exists->second;
                if(!var.data.empty()){
                    auto value_ = to_json(var.data);
                    if(result[key].type() == value_.type())
                        result[key] = value_;
                }
            }
        }
        std::cout << result.dump() << std::endl;
        return pre::json::from_json<T>(result);
    }

//    json map_to_object(const variant_map& value){
//
//        for (auto itr = result.items().begin(); itr != result.items().end(); ++itr) {
//            std::string key = itr.key();
//            auto exists = std::find_if(value.begin(), value.end(), [key](const value_pair& it){
//                return it.first == key;
//            });
//            if(exists != value.end()){
//                auto var = exists->second;
//                if(!var.data.empty()){
//                    result[key] = to_json(var.data);
//                }
//            }
//        }
//        return pre::json::from_json<T>(result);
//    }

    inline QVariant to_variant(const arcirk::variant_p& value){
        return to_variant(value.data);
    }

    template<class T>
    inline BJson serialize(const T& value){
        auto var = pre::json::to_json(value);
        return json::to_cbor(var.dump());
    }

    template<class T>
    inline T deserialize(const BJson& value){
        auto var = json::from_cbor(value);
        auto js = json::parse(var.get<std::string>());
        return pre::json::from_json<T>(js);
    }

    template<class T>
    inline T deserialize(const QByteArray& value){
        BJson ba(value.size());
        std::copy(value.begin(), value.end(), ba.begin());
        auto var = json::from_cbor(ba);
        auto js = json::parse(var.get<std::string>());
        return pre::json::from_json<T>(js);
    }

    inline json from_variant(const QVariant& value){
        if(value.typeId() == QMetaType::QString){
            return value.toString().toStdString();
        }else if(value.typeId() == QMetaType::Int){
            return value.toInt();
        }else if(value.typeId() == QMetaType::Double){
            return value.toDouble();
        }else if(value.typeId() == QMetaType::QStringList){
            return to_byte(to_binary(value.toStringList().join(",").toStdString(), subtypeArray));
        }else if(value.typeId() == QMetaType::QByteArray){
            auto qba = value.toByteArray();
            BJson ba(qba.size());
            std::copy(qba.begin(), qba.end(), ba.begin());
            return to_byte(to_binary(ba, subtypeArray));
        }else if(value.typeId() == QMetaType::Bool){
            return value.toBool();
        }else{
            return {};
        }
    }

//    template<class T>
//    inline bool is_lite_type(const T& value){
//        if(typeid(value) != typeid(json)){
//            if(typeid(value) == typeid(std::string) ||
//               typeid(value) == typeid(int) ||
//               typeid(value) == typeid(long long)||
//               typeid(value) == typeid(float)||
//               typeid(value) == typeid(double)
//                    )
//                return true;
//            else
//                return false;
//        }else{
//            json v = value;
//            if(v.is_number() || v.is_string())
//                return true;
//            else
//                return false;
//        }
//    }
//
//    inline std::string uuid_from_data(const json& data){
//        if(data["data"].is_binary()){
//            if(data["data"].get_binary().subtype() == subtypeUUID){
//                auto v = data["data"].get_binary();
//                auto str = std::string{v.begin(), v.end()};
//                if(json::accept(str)){
//                    return json::parse(str).get<std::string>();
//                }
//            }else
//                return "";
//        }
//        return "";
//    }
//    template<class T>
//    inline std::string get_representation(const T& value){
//        if(typeid(value) != typeid(json)){
//            if(typeid(value) == typeid(std::string))
//                return value;
//            else if(typeid(value) == typeid(int)){
//                int v = value;
//                return std::to_string(v);
//            }else if(typeid(value) == typeid(long long)){
//                long long v = value;
//                return std::to_string(v);
//            }else if(typeid(value) == typeid(float)){
//                double v = value;
//                return std::to_string(v);
//            }else if(typeid(value) == typeid(double)){
//                double v = value;
//                return std::to_string(v);
//            }
//            else return "<null>";
//        }else{
//            json j(value);
//            if(j.is_string())
//                return j.get<std::string>();
//            else if(j.is_boolean())
//                return j.get<bool>() ? "Истина" : "Ложь";
//            else if(j.is_number_float())
//                return std::to_string(j.get<double>());
//            else if(j.is_number_integer())
//                return std::to_string(j.get<long long>());
//            else if(j.is_array()){
//                try {
//                    auto var = j.get<BJson>();
//                    auto v = json::from_bson(var);
//                    if(v.find("data") != v.end()){
//                        if(v["data"].is_binary()){
//                            auto res = uuid_from_data(v);
//                            if(res.empty())
//                                return "<Бинарные данные>";
//                            else
//                                return res;
//                        }else
//                            return "<Массив>";
//                    }else
//                        return "<Массив>";
//                } catch (...) {
//                    return "<Массив>";
//                }
//            }else return "<Null>";
//        }
//    }
//
//    template<class T>
//    inline editor_inner_role get_variant_p_type(const T& value){
//        if(is_lite_type(value)){
//            if(typeid(value) == typeid(std::string)){
//                if(value.substr('\n') == std::string::npos)
//                    return editorText;
//                else
//                    return editorMultiText;
//            }else
//                return editorNumber;
//        }else{
//            if(typeid(value) == typeid(BJson)){
//                if(sizeof(value.data()) == sizeof(arcirk::variant_p*)){
//
//                }
//            }else
//                return editorNullType;
//        }
//    }
//
//    template<class T>
//    inline BJson to_byte(const T& value, const json::binary_t::subtype_type& hash){
//        json v = value;
//        auto s = v.dump();
//        auto var = ByteArray{s.begin(), s.end()};
//        json j;
//        j["data"] = json::binary(var, hash);
//        auto result = json::to_bson(j);
//        return result;
//    }
//
//    inline BJson to_data(const json& value, variant_subtype type = subtypeLite){
//        if(value.type() == json::value_type::value_t::binary)
//            return value.get_binary();
//        else if(value.type() == json::value_type::value_t::boolean){
//            return to_byte(value.get<bool>(), type);
//        }else if(value.type() == json::value_type::value_t::number_float){
//            return to_byte(value.get<double>(), type);
//        }else if(value.type() == json::value_type::value_t::number_integer){
//            return to_byte(value.get<long long>(), type);
//        }else if(value.type() == json::value_type::value_t::string){
//            return to_byte(value.get<std::string>(), type);
//        }else if(value.type() == json::value_type::value_t::array){
//            auto ba = json::to_cbor(value);
//            if(type == subtypeLite)
//                return to_byte(ba, subtypeArray);
//            else
//                return to_byte(ba, type);
//        }else{
//            if(type == subtypeLite)
//                return to_byte(json {}, subtypeNull);
//            else
//                return to_byte(json {}, type);
//        }
//
//    }
//
//    inline BJson to_data(const QVariant& value, variant_subtype type = subtypeLite){
//        if(value.typeId() == QMetaType::QString){
//            return to_byte(value.toString().toStdString(), type);
//        }else if(value.typeId() == QMetaType::Int){
//            return to_byte(value.toInt(), type);
//        }else if(value.typeId() == QMetaType::Double){
//            return to_byte(value.toDouble(), type);
//        }else if(value.typeId() == QMetaType::QStringList){
//            return to_byte(arcirk::strings::split(value.toStringList().join(",").toStdString(), ","), subtypeArray);
//        }else if(value.typeId() == QMetaType::QByteArray){
//            auto qba = value.toByteArray();
//            ByteArray ba(qba.size());
//            std::copy(qba.begin(), qba.end(), ba.begin());
//            return to_byte(ba, subtypeArray);
//        }else if(value.typeId() == QMetaType::Bool){
//            return to_byte(value.toBool(), type);
//        }else{
//            if(type == subtypeLite)
//                return to_byte(json {}, subtypeNull);
//            else
//                return to_byte(json {}, type);
//        }
//    }
//
//    inline arcirk::variant_p to_variant_p(const json& value, const std::string& table = ""){
//        if (value.is_array()){
//            try {
//                auto raw = value.get<BJson>();
//                if(!raw.empty()){
//                    auto object = json::from_cbor(raw);
//                    if(object.is_object()){
//                        auto result = pre::json::from_json<variant_p>(object);
//                        return result;
//                    }
//                }
//            } catch (const std::exception &) {
//                //
//            }
//        }
//        auto var = arcirk::variant_p();
//        var.table = table;
//        var.representation = get_representation(value);
//        var.data = to_data(value);
//        return var;
//    }
//
//
//    template<class T>
//    inline std::map<std::string, arcirk::variant_p> to_variant_members(const T& value){
//        auto v = pre::json::to_json(value);
//        std::map<std::string, arcirk::variant_p> result{};
//        for (auto itr = v.items().begin(); itr != v.items().end(); ++itr) {
//            result.insert(std::make_pair(itr.key(), to_variant_p(itr.value())));
//        }
//        return result;
//    }
//
//    inline std::map<std::string, arcirk::variant_p> to_variant_members(const json& value){
//        if(!value.is_object())
//            return {};
//        std::map<std::string, arcirk::variant_p> result{};
//        for (auto itr = value.items().begin(); itr != value.items().end(); ++itr) {
//            result.insert(std::make_pair(itr.key(), to_variant_p(itr.value())));
//        }
//        return result;
//    }
//
//    inline QVariant to_variant(const arcirk::variant_p& value, bool data_only = false){
//        if(!data_only){
//            auto j = pre::json::to_json(value);
//            BJson bytes = json::to_cbor(j);
//            return QByteArray(reinterpret_cast<const char*>(bytes.data()), qsizetype(bytes.size()));
//        }else{
//            if(value.data.empty())
//                return {};
//            else{
//                return QByteArray(reinterpret_cast<const char*>(value.data.data()), qsizetype(value.data.size()));
//            }
//        }
//    }
//
//    inline QVariant to_variant(const json &value)
//    {
//            if(value.is_string())
//                return QString::fromStdString(value.get<std::string>());
//            else if(value.is_number_float())
//                return value.get<double>();
//            else if(value.is_number_integer())
//                return value.get<int>();
//            else if(value.is_boolean())
//                return value.get<bool>();
//            else if(value.is_array()){
//                auto ba = json::to_cbor(value);
//                return QByteArray(reinterpret_cast<const char*>(ba.data()), qsizetype(ba.size()));
//            }else
//                return {};
//    }
//
//    inline json get_value(const BJson& bytes){
//        try {
//            auto v = json::from_bson(bytes);
//            if(v.find("data") != v.end()){
//                if(v["data"].is_binary()){
//                    auto ba = v["data"].get_binary();
//                    if(ba.subtype() != subtypeByte && ba.subtype() != subtypeArray && ba.subtype() != subtypeNull && ba.subtype() != subtypeUUID){
//                        auto str = std::string(ba.begin(), ba.end());
//                        if(json::accept(str))
//                            return json::parse(str);
//                        else
//                            return bytes;
//                    }else
//                        return bytes;
//                }else
//                    return bytes;
//            }else
//                return bytes;
//        } catch (...) {
//            return bytes;
//        }
//    }
//
//    inline json to_json(const std::map<std::string, arcirk::variant_p>& values){
//        json result;
//        for (const auto& itr : values) {
//            result[itr.first] = get_value(itr.second.data);
//        }
//        return result;
//    }
//
//    inline arcirk::variant_p from_variant(const QVariant& value){
//
//        auto ba = value.toByteArray();
//        if(ba.isEmpty())
//            return {};
//
//        BJson bytes(ba.size());
//        std::copy(ba.begin(), ba.end(), bytes.begin());
//        try {
//            auto ba_ = json::from_cbor(bytes);
//            auto m_raw = pre::json::from_json<arcirk::variant_p>(ba_);
//            return m_raw;
//        } catch (...) {
//
//        }
//
//        auto var = arcirk::variant_p();
//        var.data = BJson(ba.size());
//        std::copy(ba.begin(), ba.end(), var.data.begin());
//        auto j = get_value(var.data);
//        var.representation = get_representation(j);
//        return var;
//
//    }
//
//    inline json to_json(const variant_p& data){
//        return pre::json::to_json(data);
//    }
//
//    inline json to_raw(const variant_p& data){
//        auto ba = json::to_cbor(pre::json::to_json(data));
//        return ba;
//    }
//
//    inline variant_p from_raw(const json& data){
//        if (data.is_array()){
//            try {
//                auto var = data.get<BJson>();
//                auto object = json::from_cbor(var);
//                auto result = pre::json::from_json<variant_p>(object);
//                return result;
//            } catch (const std::exception &) {
//                //
//            }
//        }
//        return {};
//    }
//
//    inline size_t get_subtype(const BJson& data){
//        auto v = json::from_bson(data);
//        if(v.find("data") != v.end()){
//            if(v["data"].is_binary())
//                return v["data"].get_binary().subtype();
//        }
//        return 0;
//    }
//
//    inline void set_subtype(BJson& data, variant_subtype type){
//        auto v = json::from_bson(data);
//        json j;
//        j["data"] = json::binary(v.get_binary(), type);
//        data = json::to_bson(j);
//    }
//
//    inline QPair<QString, QVector<variant_p>> verify_array(const json& data){
//
//        if(!data.is_array())
//            return qMakePair("<массив>", QVector<variant_p>());
//
//        auto result = QVector<variant_p>();
//        QStringList representation;
//
//        for (auto itr = data.begin(); itr != data.end(); ++itr) {
//            const auto& object = *itr;
//            auto value = to_variant(object);
//            if(value.isValid()){
//                if(value.typeId() == QMetaType::QByteArray){
//                    result.push_back(from_variant(value));
//                }else{
//                    auto raw = variant_p();
//                    raw.representation = get_representation(data);
//                    raw.data = to_data(value);
//                    result.push_back(raw);
//                }
//            }else
//                result.push_back(variant_p());
//
//            const auto& ob = --result.end();
//            representation.append(ob->representation.c_str());
//        }
//
//        return qMakePair(representation.join(", "), result);
//    }
//
//    inline void variant_p_set_array(QPair<QString, QVector<variant_p>>& arr, variant_p& raw){
//        auto result = json::array();
//        foreach(auto itr , arr.second){
////            ByteArray bytes;
////            alpaca::serialize(itr, bytes);
//            result.push_back(to_raw(itr));
//        }
//        auto ba = json::to_cbor(result);// string_to_byte_array(result.dump());
//        raw.data = to_data(ba, subtypeArray);// ByteArray(ba.size());
//        //std::copy(ba.begin(), ba.end(), raw.data.begin());
//        raw.representation = arr.first.toStdString();
//    }

}


#endif //CONTROLSPROG_VARIANT_P_HPP
