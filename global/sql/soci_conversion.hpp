//
// Created by admin on 28.05.2024.
//

#ifndef CONTROLSPROG_SOCI_CONVERSION_HPP
#define CONTROLSPROG_SOCI_CONVERSION_HPP
#include <iostream>
#include <global.hpp>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>
#include <soci/boost-fusion.h>

// для работы со своими типами, в SOCI имеются конвертеры
namespace soci {

    template<typename T>
    struct type_conversion<T> {
        typedef values base_type;

        static void from_base(values const& v, indicator ind, T& p) {
            if (ind == i_null) return;
            try {
                p.id = v.get<int>("id", 0);
                p.birthday = v.get<std::tm>("birthday", {});
                p.firstname = v.get<std::string>("firstname", {});
                p.lastname = v.get<std::string>("lastname", {});

                std::string arr_str = v.get<std::string>("friends", {});
                extract_integers(arr_str, p.friends);
            } catch (std::exception const & e) { std::cerr << e.what() << std::endl; }
        }

        static void to_base(const T& p, values& v, indicator& ind) {
            try {
                auto object = pre::json::to_json(p);
                for (auto itr = object.items().begin(); itr != object.items().end(); ++itr) {
                    if(itr.value().is_string()){
                        json val = itr.value();
                        v.set(itr.key(), val.get<std::string>());
                    }else if(itr.value().is_boolean()){
                        json val = itr.value();
                        v.set(itr.key(), val.get<bool>());
                    }else if(itr.value().is_number_integer()){
                        json val = itr.value();
                        v.set(itr.key(), val.get<int>());
                    }else if(itr.value().is_number_float()){
                        json val = itr.value();
                        v.set(itr.key(), val.get<double>());
                    }else if(itr.value().is_number_unsigned()){
                        json val = itr.value();
                        v.set(itr.key(), val.get<unsigned int>());
                    }else if(itr.value().is_array()){
                        json val = itr.value();
                        auto ba = val.get<arcirk::BJson>();
                        auto b = blob()
                        v.set(itr.key(), val.get<unsigned int>());
                    }
                }
//                v.set("id", p.id);
//                v.set("birthday", p.birthday);
//                v.set("firstname", p.firstname);
//                v.set("lastname", p.lastname);
//
//                std::string arr_str;
//                split_integers(arr_str, p.friends);
//                v.set("friends", arr_str);

                ind = i_ok;
                return;
            } catch (std::exception const & e) { std::cerr << e.what() << std::endl; }
            ind = i_null;
        }

    };

}
#endif //CONTROLSPROG_SOCI_CONVERSION_HPP
