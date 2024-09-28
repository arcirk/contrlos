#ifndef ITEM_DATA_H
#define ITEM_DATA_H

#ifdef IS_USE_QT_LIB

#ifdef _CONTROLS_EXPORT_DLL
#include "../../controls/controls_global.h"
#else
#define CONTROLS_EXPORT
#include <global.hpp>
#endif

#include <vector>
#include <iostream>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <nlohmann/json.hpp>
#include <pre/json/from_json.hpp>
#include <pre/json/to_json.hpp>

#include <QVariant>
#include <QUuid>
#include <QString>

using json = nlohmann::json;

typedef std::vector<std::uint8_t> BJson;

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk), variant_p,
    (std::string, representation)
    (int, editor_role)
    (std::string, table)
    (BJson, data)
)

namespace arcirk {

    enum variant_subtype{
        subtypeNull = 0,
        subtypeDump,
        subtypeArray,
        subtypeByte,
        subtypeRef,
        subtypeObject
    };

    inline BJson to_byte(const json& value){
        return json::to_cbor(value.dump());
    }

    inline json to_binary(const QUuid& value){
        auto ba = value.toRfc4122();
        auto ba_ = BJson(ba.size());
        std::copy(ba.begin(), ba.end(), ba_.begin());
        return json::binary(ba_, subtypeRef);
    }

    inline json to_binary(const BJson& value, variant_subtype subtype = subtypeDump){
        return json::binary(value, subtype);
    }

    template<class T>
    inline json to_binary(const T& value, variant_subtype subtype = subtypeDump){
        BJson ba  = json::to_cbor(json(value).dump());
        return json::binary(ba, subtype);
    }

    inline BJson to_nil_uuid(){
       // auto ref = QUuid::fromString(NIL_STRING_UUID);
        return to_byte(to_binary(QUuid()));
    }

    inline BJson generate_uuid(){
        return to_byte(to_binary(QUuid::createUuid()));
    }



    template<class T>
    inline T from_binary(const json& value){
        if(!value.is_binary())
            return T();

        auto subtype = (variant_subtype)value.get_binary().subtype();
        json data = json::from_cbor(value.get_binary());
        if(subtype == subtypeDump){
            if(data.is_string()){
                auto str = data.get<std::string>();
                if(json::accept(str)){
                    auto var = json::parse(str);
                    if(typeid(T) == typeid(QString)){
                        if(var.is_string())
                            return QString(var.get<std::string>().c_str());
                        else
                            return T();
                    }else{
                        try {
                            T result = value.get<T>();
                            return result;
                        } catch (...) {
                            return T();
                        }
                    }
                }else
                    return T();
            }else
                return T();
        }else if(subtype == subtypeRef){
            try {
                if(typeid(T) == typeid(QUuid)){
                    return QUuid::fromRfc4122(data.get<BJson>());
                }else if(typeid(T) == typeid(QString)){
                    auto var = QUuid::fromRfc4122(data.get<BJson>());
                    return var.toString(QUuid::WithoutBraces);
                }else if(typeid(T) == typeid(std::string)){
                    auto var = QUuid::fromRfc4122(data.get<BJson>());
                    return var.toString(QUuid::WithoutBraces).toStdString();
                }else{
                    T result = data.get<T>();
                    return result;
                }
            } catch (...) {
                return T();
            }
        }else{
            try {
                T result = data.get<T>();
                return result;
            }catch (...) {
                return T();
            }

        }

    }

    inline variant_subtype subtype(const json& value){
        if(value.is_binary()){
            return (variant_subtype)value.get_binary().subtype();
        }else
            return subtypeNull;
    }

    inline json binary_to_json_value(const json& value){
        if(value.is_binary()){
            auto subtype = (variant_subtype)value.get_binary().subtype();
            if(subtype != subtypeRef){
                json data = json::from_cbor(value.get_binary());
                if(subtype == subtypeDump){
                    if(data.is_string()){
                        auto str = data.get<std::string>();
                        if(json::accept(str)){
                            return json::parse(str);
                        }else
                            return {};
                    }else
                        return {};
                }else if(subtype == subtypeArray){
                    if(data.is_string()){
                        auto str = data.get<std::string>();
                        if(json::accept(str)){
                            return json::parse(str);
                        }else
                            return {};
                    }else
                        return data;
                }else
                    return data;
            }else
                return value.get_binary();

        }else
            return {};
    }

    inline std::string array_to_string(const json& value){
        if(!value.is_array())
            return {};
        else{
            std::vector<std::string> result;
            for (auto itr = value.begin(); itr != value.end(); ++itr) {
                const auto& value_ = *itr;
                if(value_.type() == json::value_t::string){
                    result.push_back(value_.get<std::string>());
                }else if(value_.type() == json::value_type::value_t::boolean){
                    result.emplace_back(value_.get<bool>() ? "Истина" : "Ложь");
                }else if(value_.type() == json::value_type::value_t::number_float){
                    result.push_back(std::to_string(value_.get<double>()));
                }else if(value_.type() == json::value_type::value_t::number_integer){
                    result.push_back(std::to_string(value_.get<int>()));
                }else if(value_.type() == json::value_type::value_t::number_unsigned){
                    result.push_back(std::to_string(value_.get<int>()));
                }else
                    result.emplace_back(value_.type_name());
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
        }else if(value.type() == json::value_type::value_t::number_unsigned){
            return std::to_string(value.get<int>());
        }else if(value.type() == json::value_type::value_t::array){
            return array_to_string(value);
        }else if(value.type() == json::value_type::value_t::object){
            return "<object>";
        }return {};

    }

    inline QByteArray byte_to_qbyte(const BJson& value){
        const int size = (int)value.size();
        return {reinterpret_cast<const char*>(value.data()), size};
    }

    inline QVariant to_variant(const json& value){
            if(value.is_string())
                return value.get<std::string>().c_str();
            else if(value.is_boolean())
                return value.get<bool>();
            else if(value.is_number_float())
                return value.get<double>();
            else if(value.is_number_integer())
                return value.get<int>();
            else if(value.is_array()){
                auto val = value.get<BJson>();
                return byte_to_qbyte(val);

                //auto b = json::from_cbor(val);
                //return QByteArray(reinterpret_cast<const char*>(val.data()), (qsizetype)val.size());
            // else if(val.is_binary())
            //     return QByteArray(reinterpret_cast<const char*>(value.data()), (qsizetype)value.size());
            // else if(val.is_object())
            //     return QByteArray(reinterpret_cast<const char*>(value.data()), (qsizetype)value.size());
            }else
                return {};
    }

    inline BJson qbyte_to_byte(const QByteArray& value){
        auto ba = BJson(value.size());
        std::copy(value.begin(), value.end(), ba.begin());
        return ba;
    }

    inline json from_variant(const QVariant& value){
        if(value.userType() == QMetaType::QString){
            return value.toString().toStdString();
        }else if(value.userType() == QMetaType::Int){
            return value.toInt();
        }else if(value.userType() == QMetaType::Double){
            return value.toDouble();
        }else if(value.userType() == QMetaType::QStringList){
            return to_byte(to_binary(value.toStringList().join(",").toStdString(), subtypeArray));
        }else if(value.userType() == QMetaType::QByteArray){
            return qbyte_to_byte(value.toByteArray());
        }else if(value.userType() == QMetaType::Bool){
            return value.toBool();
        }else{
            return {};
        }
    }

    inline json from_variant(const QVariant& value, nlohmann::json::value_t t){
        if(t == json::value_t::null) return from_variant(value);
        else if(t == json::value_t::boolean){
            if(value.userType() == QMetaType::Double ||
                    value.userType() == QMetaType::Int ||
                    value.userType() == QMetaType::LongLong ||
                    value.userType() == QMetaType::ULongLong ||
                    value.userType() == QMetaType::Long ||
                    value.userType() == QMetaType::Float){
                return value.toInt() > 0;
            }else if(value.userType() == QMetaType::Bool){
                return value.toBool();
            }else
                return false;
        }
        else if(t == json::value_t::number_integer) return value.toInt();
        else if(t == json::value_t::number_unsigned) return value.toUInt();
        else if(t == json::value_t::number_float) return value.toFloat();
        else if(t == json::value_t::object) return qbyte_to_byte(value.toByteArray());
        else if(t == json::value_t::array) return qbyte_to_byte(value.toByteArray());
        else if(t == json::value_t::string) return value.toString().toStdString();
        else if(t == json::value_t::binary) return qbyte_to_byte(value.toByteArray());
        else return {};
    }

    namespace widgets {

        struct CONTROLS_EXPORT binary_data{
            BJson data{};
            variant_subtype subtype{};

            [[nodiscard]] json to_json() const{
                return json::binary(data, subtype);
            }


            [[nodiscard]] BJson to_byte() const{
                return json::to_cbor(json::binary(data, subtype).dump());
            }

            [[nodiscard]] json to_json_value() const{
                if(subtype != subtypeNull){
                    if(subtype != subtypeRef && subtype != subtypeByte)
                        return binary_to_json_value(json::binary(data, subtype));
                    else
                        return to_byte();
                }else
                    return {};
            }

            bool from_byte(const json& value){
                try {
                    auto v_ba = json::from_cbor(value.get<BJson>());
                    auto str = v_ba.get<std::string>();
                    if(json::accept(str)){
                        auto bi_object = json::parse(v_ba.get<std::string>());
                        data = bi_object["bytes"].get<BJson>();
                        auto subtype_ = bi_object["subtype"].get<int>();
                        subtype = (variant_subtype)subtype_;
                        return true;
                    }
                } catch (const std::exception&) {
                    //
                }
                return false;
            }

            void from_json(const json& value){
                if(value.is_null()){
                    data = {};
                    subtype = subtypeNull;
                }else{
                    if(!from_byte(value)){
                        if(value.is_binary()){
                            subtype = (variant_subtype)value.get_binary().subtype();
                            data = value.get_binary();
                        }else{
                            data = json::to_cbor(value.dump());
                            //std::cout << value.dump(4) << std::endl;
                            if(value.is_array()){
                                subtype = subtypeArray;
                            }else subtype = subtypeDump;
                        }
                    }
                }
            }
        };

        class CONTROLS_EXPORT item_data
        {
        public:
            explicit item_data();
            explicit item_data(const json& value);
            ~item_data()= default;

            [[nodiscard]] std::string representation() const;
            [[nodiscard]] int role() const;
            void set_role(const int& value);
            [[nodiscard]] std::string table() const;
            [[nodiscard]] binary_data * data() const;
            [[nodiscard]] json json_value() const;

            [[nodiscard]] QVariant value() const;
            void set_value(const json& value);

            void from_json(const json& value);
            [[nodiscard]] json to_json() const;
            [[nodiscard]] BJson to_byte() const;

        private:
            std::shared_ptr<binary_data> m_data;
            int m_role;
            std::string m_table;

        };

    }

    inline QUuid to_qt_uuid(const json& value){
        if(value.empty())
            return {};

        auto ref = widgets::item_data(value);
        auto uuid = QUuid::fromRfc4122(ref.data()->data);
        return uuid;
    }
}

typedef std::pair<std::string, std::shared_ptr<arcirk::widgets::item_data>> value_pair;
typedef std::vector<value_pair> variant_map;

inline value_pair to_value_pair(const std::string& key, const json& value){
    return std::make_pair(key, std::make_shared<arcirk::widgets::item_data>(value));
}

inline variant_map object_to_map(const json& object){
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
            auto val = exists->second->json_value();
            //std::cout << key << " " << arcirk::type_string(result[key].type()) << " " << arcirk::type_string(val.type()) << std::endl;
            if(result[key].type() == val.type())
                result[key] = val;
            else{
                if(result[key].is_number() && val.is_number())
                    result[key] = val;
            }
        }
    }
    try {
        return pre::json::from_json<T>(result);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        throw std::exception("Ошибка сериализации структуры");
    }

}

#endif
#endif // ITEM_DATA_H
