#ifndef CONTROLS_GLOBAL_H
#define CONTROLS_GLOBAL_H

#include <QtCore/qglobal.h>
#include "../global/global.hpp"
#include <QUuid>
#include <QVariant>

#if defined(CONTROLS_LIBRARY)
#define CONTROLS_EXPORT Q_DECL_EXPORT
#else
#define CONTROLS_EXPORT Q_DECL_IMPORT
#endif

typedef QMap<QString, QString> AliasesMap;

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

    enum tree_rows_icons{
        ItemGroup = 0,
        DeletedItemGroup,
        ItemGroupPredefined,
        Item,
        DeletedItem,
        ItemPredefined
    };

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

    inline std::string to_number_string(const json& value) {
        if(value.is_number_float()){
            return QString::number(value.get<double>()).toStdString();
        }else if(value.is_number_integer()){
            return QString::number(value.get<int>()).toStdString();
        }else
            return "0";
    }

}

#endif // CONTROLS_GLOBAL_H
