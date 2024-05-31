//
// Created by admin on 27.05.2024.
//

#ifndef CONTROLSPROG_SQL_QUERY_HPP
#define CONTROLSPROG_SQL_QUERY_HPP

#include <global.hpp>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>
#include <soci/boost-fusion.h>
#include <boost/variant.hpp>
#include <map>

using namespace arcirk::database;
using namespace soci;

namespace arcirk::database{

    typedef boost::variant<BJson, std::string, bool, int, double> variant_type;

    class sql_query{


    public:

        explicit sql_query(std::shared_ptr<session>& sess);
        ~sql_query();

        void set_text(const std::string& query_text);

        void set_value(const std::string& field, const variant_type& value);

        bool execute();

        bool next();

        void clear();

    private:
        std::shared_ptr<session>& sql;
        std::string m_text;
        std::map<std::string, variant_type> m_values;
        statement m_recordset;

    };
}


#endif //CONTROLSPROG_SQL_QUERY_HPP
