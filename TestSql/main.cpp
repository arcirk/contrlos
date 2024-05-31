#include <iostream>
#include <global.hpp>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>
#include <soci/boost-fusion.h>
#include <soci/odbc/soci-odbc.h>
#include <sql/query_builder.hpp>
#include <boost/filesystem.hpp>
#include "include/sql_query.hpp"

#include <boost/regex.hpp>

using namespace arcirk;
using namespace soci;
using namespace boost::filesystem;
using namespace arcirk::database;

std::shared_ptr<session> sql;

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk), test_database,
    (std::string, command)
    (std::string, param)
    (BJson, ref)
    (BJson, parent)
    (BJson, client_ref)
)

void get_param_from_position(variant_map& values, const std::string& query_text){

}

int
main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");

    sql = std::make_shared<session>();
    path data = "arcirk_test.sqlite";
    std::string connection_string = str_sample("db=%1% timeout=2 shared_cache=true", data.string());
    sql->open(soci::sqlite3, connection_string);

    if(sql->is_connected()){
        std::cout << "database connection success" << std::endl;
    }else{
        std::cout << "error database connection" << std::endl;
    }

//    auto object = test_database();
//    auto fields = builder::from_object_structure(pre::json::to_json(object));
//
//    *sql << builder::query_builder().create_table("test_database", fields).prepare();

    auto query = sql_query(sql);
    auto builder = builder::query_builder();
    builder.set_parameters_by_field_name(true);

    auto r_row = test_database();
    r_row.ref = generate_uuid();
    r_row.parent = generate_uuid();
    r_row.client_ref = generate_uuid();
    r_row.command = "test";
    r_row.param = "param";
    auto object = pre::json::to_json(r_row);
    builder.use(object);
    auto map_object = object_to_map(object);
    auto sql_text = builder.insert("test_database", false).prepare();

    static const boost::regex r(R"(:\w+)");
    boost::smatch x_results;
    boost::regex_match(sql_text,  x_results, r);


    for (const auto & match : ) {

    }

    std::cout << sql_text << std::endl;

//    soci::blob b_ref(*sql);
//    b_ref.write(0, reinterpret_cast<const char*>( r_row.ref.data() ), r_row.ref.size());
//    soci::blob b_parent(*sql);
//    b_parent.write(0, reinterpret_cast<const char*>( r_row.parent.data() ), r_row.parent.size());
//    soci::blob b_client_ref(*sql);
//    b_client_ref.write(0, reinterpret_cast<const char*>( r_row.client_ref.data() ), r_row.client_ref.size());



//    try {
//        auto st = (sql->prepare << sql_text);//, use(b_ref, "ref"), use(b_parent, "parent"), use(b_client_ref, "client_ref"), use(r_row.command, "command"), use(r_row.param, "param");
//        auto p = pre::json::to_json(r_row);
//        std::vector<soci::values> vals;
//        for (auto itr = p.items().begin(); itr != p.items().end(); ++itr) {
//            if(itr.value().is_array()){
//                BJson b = itr.value().get<BJson>();
//                soci::blob b_ref(*sql);
//                b_ref.write(0, reinterpret_cast<const char*>( r_row.ref.data() ), r_row.ref.size());
//                //st << soci::use(itr.key(), b_ref);
//                auto v = soci::values();
//                v.set(itr.key(), b_ref);
//            }else if(itr.value().is_string()){
//                st << itr.key() , soci::use(itr.value().get<std::string>());
//            }
//        }
//        soci::statement s = st;
//        s.execute();
//    } catch (const std::exception &e) {
//        std::cerr << e.what() << std::endl;
//    }


//    std::vector<std::tuple<std::string, BJson>> blobs{};
//    std::string  query_text = builder::query_builder().insert("test_database", blobs).prepare();
//    //query.set_text()
//
//
//    query.set_text("select * from test_database");
//
//    auto result = query.execute();
//
//    std::cout << "query result " << result << std::endl;
//
//    soci::session sql_ = soci::session();
//    sql_ << "select", soci::use(std::vector<std::string>());
}