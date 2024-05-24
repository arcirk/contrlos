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
    (arcirk::database), database_config,
    (std::string, first)
    (std::string, second)
    (arcirk::BJson , ref)
    (int, version)
);

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
        (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, cache)
                (std::string, number)
                (int, date)
                (std::string, xml_type)
                (int, version)
                (std::string, device_id)
                (std::string, workplace)
                (std::string, parent)
                (int, is_group)
                (int, deletion_mark)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), document_table,
        (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, cache)
                (double, price)
                (double, quantity)
                (std::string, barcode)
                (std::string, vendor_code)
                (std::string, product)
                (std::string, parent)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), document_marked_table,
        (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, qr_code)
                (std::string, document_ref)
                (std::string, parent)
                (int, quantity)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), nomenclature,
        (int, _id)
                (std::string, first) // Наименование
                (std::string, second)
                (std::string, ref)
                (std::string, cache) // Все остальные реквизиты
                (std::string, parent)
                (std::string, vendor_code)
                (std::string, trademark)
                (std::string, unit)
                (int, is_marked)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), barcodes,
        (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, barcode)
                (std::string, parent)
                (std::string, vendor_code)
                (std::string, first_name)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), available_certificates,
        (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, user_uuid)
                (std::string, cert_uuid)
                (std::string, parent)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), certificates,
        (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, cache)
                (arcirk::ByteArray, data)
                (std::string, private_key)
                (std::string, subject)
                (std::string, issuer)
                (std::string, not_valid_before)
                (std::string, not_valid_after)
                (std::string, parent_user)
                (std::string, serial)
                (std::string, suffix)
                (std::string, sha1)
                (std::string, parent)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), cert_users,
        (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, cache)
                (std::string, uuid)
                (std::string, sid)
                (std::string, system_user)
                (std::string, host)
                (std::string, parent)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), table_info_sqlite,
        (int, cid)
                (std::string, name)
                (std::string, type)
                (int, notnull)
                (std::string, dflt_value)
                (int, bk)
);
BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), sqlite_functions_info,
        (std::string, name)
                (std::string, fun)
                (std::string, desc)
                (std::string, ref)
                (std::string, parent)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), containers,
        (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, cache)
                (arcirk::ByteArray, data)
                (std::string, subject)
                (std::string, issuer)
                (std::string, not_valid_before)
                (std::string, not_valid_after)
                (std::string, parent_user)
                (std::string, sha1)
                (std::string, parent)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);
BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), registered_users_view,
        (std::string, ref)
                (std::string, first)
                (std::string, uuid)
                (std::string, parent)
                (int, is_group)
                (int, unread)
);

#endif //CONTROLSPROG_ARCIRK_METADATA_HPP
