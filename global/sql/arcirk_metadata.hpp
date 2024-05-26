//
// Created by admin on 24.05.2024.
//

#ifndef CONTROLSPROG_ARCIRK_METADATA_HPP
#define CONTROLSPROG_ARCIRK_METADATA_HPP
#include "../global.hpp"
#include <map>
#include <utility>

namespace arcirk{
    enum DatabaseType{
        dbTypeSQLite = 0,
        dbTypeODBC
    };
};

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), user_info,
    (std::string, first)
    (std::string, second)
    (arcirk::BJson , ref)
    (std::string, hash)
    (std::string, role)
    (std::string, performance)
    (arcirk::BJson, parent)
    (std::string, cache)
    (bool, is_group)
    (bool, deletion_mark)
    (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), organizations,
    (std::string, first)
    (std::string, second)
    (arcirk::BJson, ref)
    (std::string, cache)
    (arcirk::BJson, parent)
    (bool, is_group)
    (bool, deletion_mark)
    (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), subdivisions,
    (std::string, first)
    (std::string, second)
    (arcirk::BJson, ref)
    (std::string, cache)
    (arcirk::BJson, parent)
    (bool, is_group)
    (bool, deletion_mark)
    (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), warehouses,
    (std::string, first)
    (std::string, second)
    (arcirk::BJson, ref)
    (std::string, cache)
    (arcirk::BJson, parent)
    (bool, is_group)
    (bool, deletion_mark)
    (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), price_types,
    (std::string, first)
    (std::string, second)
    (arcirk::BJson, ref)
    (std::string, cache)
    (arcirk::BJson, parent)
    (bool, is_group)
    (bool, deletion_mark)
    (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), workplaces,
    (std::string, first)
    (std::string, second)
    (arcirk::BJson, ref)
    (std::string, cache)
    (std::string, server)
    (arcirk::BJson, parent)
    (bool, is_group)
    (bool, deletion_mark)
    (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), devices,
    (std::string, first)
    (std::string, second)
    (arcirk::BJson, ref)
    (std::string, cache)
    (arcirk::BJson, deviceType)
    (std::string, address)
    (arcirk::BJson, workplace)
    (arcirk::BJson, price_type)
    (arcirk::BJson, warehouse)
    (arcirk::BJson, subdivision)
    (arcirk::BJson, organization)
    (arcirk::BJson, parent)
    (bool, is_group)
    (bool, deletion_mark)
    (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), devices_view,
    (std::string, first)
    (std::string, second)
    (arcirk::BJson, device_type)
    (arcirk::BJson, workplace)
    (arcirk::BJson, price)
    (arcirk::BJson, warehouse)
    (arcirk::BJson, subdivision)
    (arcirk::BJson, organization)
    (std::string, address)
);

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), documents,
    (std::string, first)
    (std::string, second)
    (arcirk::BJson, ref)
    (std::string, cache)
    (std::string, number)
    (int, date)
    (std::string, xml_type)
    (int, version)
    (arcirk::BJson, device_id)
    (arcirk::BJson, workplace)
    (arcirk::BJson, parent)
    (bool, is_group)
    (bool, deletion_mark)
);

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), document_table,
    (std::string, first)
    (std::string, second)
    (arcirk::BJson, ref)
    (std::string, cache)
    (double, price)
    (double, quantity)
    (std::string, barcode)
    (std::string, vendor_code)
    (std::string, product)
    (arcirk::BJson, parent)
    (bool, is_group)
    (bool, deletion_mark)
    (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), document_marked_table,
    (std::string, first)
    (std::string, second)
    (arcirk::BJson, ref)
    (std::string, qr_code)
    (arcirk::BJson, document_ref)
    (arcirk::BJson, parent)
    (int, quantity)
    (bool, is_group)
    (bool, deletion_mark)
    (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), nomenclature,
    (std::string, first) // Наименование
    (std::string, second)
    (arcirk::BJson, ref)
    (std::string, cache) // Все остальные реквизиты
    (arcirk::BJson, parent)
    (std::string, vendor_code)
    (std::string, trademark)
    (std::string, unit)
    (int, is_marked)
    (bool, is_group)
    (bool, deletion_mark)
    (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), barcodes,
    (std::string, first)
    (std::string, second)
    (arcirk::BJson, ref)
    (std::string, barcode)
    (arcirk::BJson, parent)
    (std::string, vendor_code)
    (std::string, first_name)
    (bool, is_group)
    (bool, deletion_mark)
    (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), sqlite_functions_info,
    (std::string, name)
    (std::string, fun)
    (std::string, desc)
    (arcirk::BJson, ref)
    (arcirk::BJson, parent)
    (bool, is_group)
    (bool, deletion_mark)
    (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), registered_users_view,
    (arcirk::BJson, ref)
    (std::string, first)
    (std::string, uuid)
    (arcirk::BJson, parent)
    (bool, is_group)
    (int, unread)
);

namespace arcirk::database{

//    static inline std::map<tables, int> get_release_tables_versions(){
//        std::map<tables, int> result;
//        result.emplace(tables::tbDatabaseConfig, 0);
//        result.emplace(tables::tbNomenclature, 0);
//        result.emplace(tables::tbDocuments,0);
//        result.emplace(tables::tbDevices,0);
//        result.emplace(tables::tbMessages, 0);
//        result.emplace(tables::tbUsers, 0);
//        result.emplace(tables::tbDevicesType, 0);
//        result.emplace(tables::tbDocumentsTables, 0);
//        result.emplace(tables::tbOrganizations, 0);
//        result.emplace(tables::tbPriceTypes, 0);
//        result.emplace(tables::tbSubdivisions, 0);
//        result.emplace(tables::tbWarehouses, 0);
//        result.emplace(tables::tbWorkplaces, 0);
//        result.emplace(tables::tbBarcodes, 0);
//        result.emplace(tables::tbDocumentsMarkedTables, 0);
//        result.emplace(tables::tbCertificates, 0);
//        result.emplace(tables::tbCertUsers, 0);
//        result.emplace(tables::tbContainers, 0);
//        result.emplace(tables::tbAvailableCertificates, 0);
//        result.emplace(tables::tbSQliteFunctionsInfo, 10;
//        return result;
//    }
//    static inline void verify_database(soci::session& sql, DatabaseType type, const nlohmann::json& version){
//
//    }

}
#endif //CONTROLSPROG_ARCIRK_METADATA_HPP
