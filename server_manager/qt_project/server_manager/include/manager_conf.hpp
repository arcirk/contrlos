//
// Created by admin on 07.06.2024.
//

#ifndef CONTROLSPROG_MANAGER_CONF_HPP
#define CONTROLSPROG_MANAGER_CONF_HPP

#include <QtCore>
#include <global.hpp>
#include <QStandardPaths>
#include <fs.hpp>

namespace arcirk::client{

    inline bool verify_directories(){
        using namespace arcirk::filesystem;

        auto path = FSPath();
        path.init_app_data_dir();

        path /= "data";
        if(!path.dir_exists())
            path.mkpath();

        return true;
    }

}

#endif //CONTROLSPROG_MANAGER_CONF_HPP
