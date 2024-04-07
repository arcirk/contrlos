#ifndef CONTROLS_GLOBAL_H
#define CONTROLS_GLOBAL_H

#include <QtCore/qglobal.h>
#include "../global/global.hpp"
#include <QUuid>
#include <QVariant>
//#include <QCborValue>
//#include <QStringList>

#if defined(CONTROLS_LIBRARY)
#define CONTROLS_EXPORT Q_DECL_EXPORT
#else
#define CONTROLS_EXPORT Q_DECL_IMPORT
#endif

namespace arcirk::widgets{

    enum table_private_roles{
        TABLE_DATA = (Qt::UserRole + 1),
        TABLE_ITEM_READ_ONLY = (Qt::UserRole + 2),
        TABLE_DATA_VALUE = (Qt::UserRole + 3),
        TABLE_ITEM_ROLE = (Qt::UserRole + 4),
        TABLE_ITEM_ICON = (Qt::UserRole + 5),
        TABLE_ITEM_SUBTYPE = (Qt::UserRole + 6),
        TABLE_ITEM_SELECT_TYPE = (Qt::UserRole + 7)
    };

    enum table_command{
        selectDirectory = 0,
        selectFile,
        selectReadFile,
        selectWriteFile
    };

//    inline QUuid to_qt_uuid(const std::string& ref){
//        return QUuid::fromString(ref.c_str());
//    }
//
//    inline QString uuid_to_string(const QUuid& uuid){
//        return uuid.toString(QUuid::WithoutBraces);
//    }
//
//    inline QByteArray generate_uuid(){return QUuid::createUuid().toByteArray(QUuid::WithoutBraces);};

//    inline QStringList to_string_list(const json& array){
//        QStringList result;
//        for (auto itr = array.begin(); itr != array.end(); ++itr) {
//            if(itr->is_string()){
//                std::string r = itr->get<std::string>();
//                result.append(r.c_str());
//            }else
//                result.append("<null>");
//        }
//        return result;
//    }

//    inline json to_json(const variant_p &value){
//        ByteArray bytes;
//        alpaca::serialize(value, bytes);
//        json result = bytes;
//        return result;
//    }
//    inline json to_json(const variant_p &value){
////        BJson bytes;
////        alpaca::serialize(value, bytes);
////        return result;
//        return pre::json::to_json(value);
//    }

//    inline json to_json(const QVariant &value)
//    {
//        json val{};
//        if(value.typeId() == QMetaType::QString){
//            val = value.toString().toStdString();
//        }else if(value.typeId() == QMetaType::Int){
//            val = value.toInt();
//        }else if(value.typeId() == QMetaType::Double){
//            val = value.toDouble();
//        }else if(value.typeId() == QMetaType::Float){
//            val = value.toFloat();
//        }else if(value.typeId() == QMetaType::Bool){
//            val = value.toBool();
//        }else if(value.typeId() == QMetaType::QStringList){
//            val = json::array();
//            foreach (auto itr, value.toStringList()) {
//                val += itr.toStdString();
//            }
//        }else if(value.typeId() == QMetaType::QByteArray){
//            ByteArray bytes(value.toByteArray().size());
//            auto var = value.toByteArray();
//            std::copy(var.begin(), var.end(), bytes.begin());
//            val = var;
//        }else if(value.typeId() == QMetaType::QCborValue) {
////            BJson bytes(value.toByteArray().size());
////            auto var = value.toByteArray();
////            std::copy(var.begin(), var.end(), bytes.begin());
////            val = var;
//        }else
//            val = "";
//
//        return val;
//    }

//    inline QVariant to_variant(const json &value)
//    {
//        if(value.is_string())
//            return QString::fromStdString(value.get<std::string>());
//        else if(value.is_number_float())
//            return value.get<double>();
//        else if(value.is_number_integer())
//            return value.get<int>();
//        else if(value.is_boolean())
//            return value.get<bool>();
//        else if(value.is_array()){
//            try {
//                auto var = value.get<ByteArray>();
//                return QByteArray(reinterpret_cast<const char*>(var.data()), qsizetype(var.size()));
//            }catch(const std::exception &){
//                return to_string_list(value);
//            }
//        }else if(value.is_binary()) {
//            auto var = json::to_cbor(value);
//            return QCborValue(reinterpret_cast<const char*>(var.data())).toVariant();
//            //return QByteArray(reinterpret_cast<const char*>(var.data()), qsizetype(var.size()));
//        }else
//            return {};
//    }

//    inline variant_p to_variant_p(const QVariant& value){
//        if(value.typeId() != QMetaType::QByteArray)
//            return variant_p_();
//        if (sizeof(value.toByteArray().data())  != sizeof(variant_p*))
//            return variant_p_();
//        auto var = value.toByteArray();
//        ByteArray ba(var.size());
//        std::copy(var.begin(), var.end(), ba.begin());
//        std::error_code ec;
//        auto m_raw = alpaca::deserialize<variant_p>(ba, ec);
//        if (!ec) {
//            return m_raw;
//        }else
//            return variant_p_();
//    }

//    inline variant_p to_variant_p(const json& value){
//        if(!value.is_array())
//            return variant_p_();
//
//        auto var = to_variant(value);
//
//        if(var.typeId() != QMetaType::QByteArray)
//            return variant_p_();
//
//        return to_variant_p(var);
//
//    }

//    inline QVariant to_variant(const variant_p& value){
//        ByteArray bytes;
//        alpaca::serialize(value, bytes);
//        return QByteArray(reinterpret_cast<const char*>(bytes.data()), qsizetype(bytes.size()));
//    }

    inline QString variant_to_number_string(const QVariant& value){
        if(value.typeId() == QMetaType::Int){
            return QString::number(value.toInt());
        }else if(value.typeId() == QMetaType::Double){
            return QString::number(value.toDouble());
        }else if(value.typeId() == QMetaType::Float) {
            return QString::number(value.toFloat());
        }else
            return QString::number(0);
    }
//
//    inline void variant_p_set_data(const QVariant& value, variant_p& raw, tree_editor_inner_role text_role = widgetInnerRoleINVALID){
//        auto result = to_json(value);
//        auto data = string_to_byte_array(result.dump());
//        raw.data = ByteArray(data.size());
//        std::copy(data.begin(), data.end(), raw.data.begin());
//        if(value.typeId() == QMetaType::QString){
//            if(text_role == widgetInnerRoleINVALID){
//                if (value.toString().indexOf("\n") == -1)
//                    raw.role = widgetText;
//                else
//                    raw.role = widgetMultiText;
//            }else
//                raw.role = text_role;
//            raw.representation = value.toString().toStdString();
//        }else if(value.typeId() == QMetaType::QStringList){
//            raw.representation = value.toStringList().join(",").toStdString();
//            raw.role = widgetArray;
//        }else if(value.typeId() == QMetaType::Bool){
//            raw.representation = value.toBool() ? "Истина" : "Ложь";
//            raw.role = widgetBoolean;
//        }else if(value.typeId() == QMetaType::Int ||
//                value.typeId() == QMetaType::Double ||
//                value.typeId() == QMetaType::Float){
//            raw.representation = variant_to_number_string(value).toStdString();
//            raw.role = widgetNumber;
//        }else if(value.typeId() == QMetaType::QByteArray){
//            raw.representation = "<массив>";
//        }
//        raw.type = vJsonDump;
//    }
//
//    inline variant_p variant_p_default(tree_editor_inner_role role){
//        if(role == widgetNullType || role == widgetInnerRoleINVALID){
//            return variant_p_();
//        }else{
//            auto raw = variant_p_();
//            raw.role = role;
//            raw.type = vJsonDump;
//            json val{};
//            if(role == widgetFilePath ||
//                role == widgetDirectoryPath ||
//                role == widgetText ||
//                role == widgetMultiText){
//                val = "";
//
//            }else if(role == widgetNumber){
//                val = 0;
//            }else if(role == widgetBoolean){
//                val = false;
//            }else if(role == widgetArray ||
//                     role == widgetDataReference ||
//                     role == widgetByteArray){
//                val = json::array();
//                raw.representation = "<массив>";
//            }
//            auto ba = string_to_byte_array(val.dump());
//            raw.data = ByteArray(ba.size());
//            std::copy(ba.begin(), ba.end(), raw.data.begin());
//            return raw;
//        }
//
//    }

    inline std::string to_number_string(const json& value) {
        if(value.is_number_float()){
            return QString::number(value.get<double>()).toStdString();
        }else if(value.is_number_integer()){
            return QString::number(value.get<int>()).toStdString();
        }else
            return "0";
    }
//
//    inline void variant_p_set_data(const json& value, variant_p& raw){
//        auto data = string_to_byte_array(value.dump());
//        raw.data = ByteArray(data.size());
//        std::copy(data.begin(), data.end(), raw.data.begin());
//        if(value.is_string()){
// //           if(text_role == widgetInnerRoleINVALID) {
//                if (index_of(value.get<std::string>(), "\n") == -1)
//                    raw.role = widgetText;
//                else
//                    raw.role = widgetMultiText;
////            }else
////                raw.role = text_role;
//            raw.representation = value.get<std::string>();
//        }else if(value.is_array()){
//            raw.representation = "<массив>";
//            raw.role = widgetArray;
//        }else if(value.is_boolean()){
//            raw.representation = value.get<bool>() ? "Истина" : "Ложь";
//            raw.role = widgetBoolean;
//        }else if(value.is_number()){
//            raw.representation = to_number_string(value);
//            raw.role = widgetNumber;
//        }
//        raw.type = vJsonDump;
//    }
//
//    template<typename T>
//    inline T parse_value(const ByteArray& value){
//        auto str = byte_array_to_string(value);
//        if(!json::accept(str))
//            return T();
//        auto value_ = json::parse(str);
//        if(typeid(T) == typeid(json))
//            return value_;
//        try {
//            return value_.get<T>();
//        } catch (const std::exception &) {
//            return T();
//        }
//    }
//
//    inline variant_p from_json(const json& value){
//        auto val = to_variant(value);
//        if(!val.isValid())
//            return variant_p_();
//        if(val.typeId() == QMetaType::QByteArray)
//            return to_variant_p(value);
//        else{
//            auto raw = variant_p_();
//            variant_p_set_data(value, raw);
//            return raw;
//        }
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
//                    result.push_back(to_variant_p(value));
//                }else{
//                    auto raw = variant_p_();
//                    variant_p_set_data(value, raw);
//                    result.push_back(raw);
//                }
//            }else
//                result.push_back(variant_p_default(widgetNullType));
//
//            const auto& ob = --result.end();
//            representation.append(ob->representation.c_str());
//        }
//
//        return qMakePair(representation.join(", "), result);
//    };
//
//    inline void variant_p_set_array(QPair<QString, QVector<variant_p>>& arr, variant_p& raw){
//        auto result = json::array();
//        foreach(auto itr , arr.second){
//            ByteArray bytes;
//            alpaca::serialize(itr, bytes);
//            result.push_back(bytes);
//        }
//        auto ba = string_to_byte_array(result.dump());
//        raw.data = ByteArray(ba.size());
//        std::copy(ba.begin(), ba.end(), raw.data.begin());
//        raw.representation = arr.first.toStdString();
//    }

}

#endif // CONTROLS_GLOBAL_H
