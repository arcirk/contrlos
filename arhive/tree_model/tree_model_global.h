#ifndef TREE_MODEL_GLOBAL_H
#define TREE_MODEL_GLOBAL_H

#include <QtCore/qglobal.h>
#include "../global/global.hpp"
#include <QDebug>
#include <QUuid>
#include <QVariant>
#include <QStringList>

#if defined(TREE_MODEL_LIBRARY)
#  define TREE_MODEL_EXPORT Q_DECL_EXPORT
#else
#  define TREE_MODEL_EXPORT Q_DECL_IMPORT
#endif

namespace arcirk::tree_model{

    inline QUuid to_qt_uuid(const std::string& ref){
        return QUuid::fromString(ref.c_str());
    }

    inline QString uuid_to_string(const QUuid& uuid){
        return uuid.toString(QUuid::WithoutBraces);
    }

    inline QByteArray generate_uuid(){return QUuid::createUuid().toByteArray(QUuid::WithoutBraces);};

    inline QStringList to_string_list(const json& array){
        QStringList result;
        for (auto itr = array.begin(); itr != array.end(); ++itr) {
            std::string r = itr->get<std::string>();
            result.append(r.c_str());
        }
        return result;
    }

    inline json to_json(const QVariant &value)
    {
        json val{};
        if(value.typeId() == QMetaType::QString){
            val = value.toString().toStdString();
        }else if(value.typeId() == QMetaType::Int){
            val = value.toInt();
        }else if(value.typeId() == QMetaType::Double){
            val = value.toDouble();
        }else if(value.typeId() == QMetaType::Float){
            val = value.toFloat();
        }else if(value.typeId() == QMetaType::Bool){
            val = value.toBool();
        }else if(value.typeId() == QMetaType::QStringList){
            val = json::array();
                    foreach (auto itr, value.toStringList()) {
                    val += itr.toStdString();
                }
        }else
            val = "";

        return val;
    }

    inline QVariant to_variant(const json &value)
    {
        if(value.is_string())
            return QString::fromStdString(value.get<std::string>());
        else if(value.is_number_float())
            return value.get<double>();
        else if(value.is_number_integer())
            return value.get<int>();
        else if(value.is_boolean())
            return value.get<bool>();
        else if(value.is_array())
            return to_string_list(value);
        else
            return QVariant();
    }
}

#endif // TREE_MODEL_GLOBAL_H
