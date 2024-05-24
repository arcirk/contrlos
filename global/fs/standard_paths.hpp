//
// Created by admin on 22.05.2024.
//

#ifndef CONTROLSPROG_STANDARD_PATHS_HPP
#define CONTROLSPROG_STANDARD_PATHS_HPP

#include "../global.hpp"
#include <boost/filesystem.hpp>

using namespace arcirk::strings;

namespace arcirk::standard_paths{

    inline std::string home(){
        using namespace boost::filesystem;
#ifdef _WINDOWS
        path p(to_utf(getenv("APPDATA")));
        return p.string();
#else
        return home();
#endif
    }

    inline std::string home_roaming_dir(){
        using namespace boost::filesystem;
#ifdef _WINDOWS
        path p(to_utf(getenv("APPDATA")));
        return p.string();
#else
        return home();
#endif
    }

    inline std::string home_local_dir(){
        using namespace boost::filesystem;
#ifdef _WINDOWS
        path p(to_utf(getenv("LOCALAPPDATA")));
        return p.string();
#else
        return home();
#endif
    }
    inline std::string temporary_dir(){
        return boost::filesystem::temp_directory_path().string();
    }

    inline std::string program_data_dir(){
        using namespace boost::filesystem;
#ifdef _WINDOWS
        std::string pt = to_utf(getenv("PROGRAMDATA"));
        //path p();
        return pt;//p.string();
#else
        return home();
#endif
    }

    inline bool verify_directory(const std::string& dir_path) {
        using namespace boost::filesystem;
        path p(from_utf(dir_path));

        if (!exists(p)) {
            try {
                return boost::filesystem::create_directories(p);
            }catch (std::exception& e){
                std::cerr << e.what() << std::endl;
                return false;
            }
        }
        return true;

    }

    inline bool verify_directory(const boost::filesystem::path& dir_path){
        using namespace boost::filesystem;
        //const std::string w_p = from_utf(dir_path.string());
        //path p(w_p);
        if (!boost::filesystem::exists(dir_path)) {
            try {
                return boost::filesystem::create_directories(dir_path);
            }catch (std::exception& e){
                std::cerr << e.what() << std::endl;
                return false;
            }

        }
        return true;
    }

    inline std::string this_application_conf_dir(const std::string& app_name, bool mkdir_is_not_exists = true){
        boost::filesystem::path app_conf(home_roaming_dir());
        app_conf /= app_name;
        if (mkdir_is_not_exists)
            verify_directory(app_conf);
        return app_conf.string();
    }

    inline std::string this_server_conf_dir(const std::string& app_name, bool mkdir_is_not_exists = true){
        const auto p = to_utf(getenv("PROGRAMDATA"));// program_data_dir();
        boost::filesystem::path app_conf(p);
        app_conf /= app_name;
        if (mkdir_is_not_exists){
            auto result = verify_directory(app_conf);
            if(!result)
                throw std::exception("Error verify directory!");
        }

        return app_conf.string();
    }

    inline boost::filesystem::path program_data(){
        using namespace boost::filesystem;

        std::string arcirk_dir = "arcirk";
#ifndef _WINDOWS
        arcirk_dir = "." + arcirk_dir;
#endif

        path app_conf(arcirk::standard_paths::this_server_conf_dir(arcirk_dir));

        return app_conf;
    }

}
#endif //CONTROLSPROG_STANDARD_PATHS_HPP
