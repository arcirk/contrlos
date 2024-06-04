#ifndef DIRECTORY_STRUCTURE_CHECK_HPP
#define DIRECTORY_STRUCTURE_CHECK_HPP

#include <global.hpp>
#include <fs.hpp>
#include <QStandardPaths>
#include <QDir>

#define ARCIRK_SERVER_NAME_ "winsrv"

namespace arcirk::verify_application {

    inline QString working_directory(){
        auto m = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
        return m.at(1);
    }


    template<class T>
    inline bool verify_directorias(T& conf, const QString& file_name_conf, const QString& version){

        using namespace arcirk::filesystem;

        FSPath dir(working_directory());
        dir /= version;
        auto result = dir.mkpath();
        if(!result)
            return false;
        conf.ServerWorkingDirectory = working_directory().toStdString();

        auto file = dir.to_file(file_name_conf);
        if(file.exists()){
            if(file.open(QIODevice::ReadOnly)){
                auto data = file.readAll();
                conf = secure_serialization<T>(data.toStdString(), __FUNCTION__);
                file.close();
            }
        }

        FSPath v_dir;
        v_dir << dir.path() /= "html";
        result = v_dir.mkpath();
        if(!result)
            return false;

        conf.ServerHttpRoot = v_dir.path().toStdString();

        QString current_address = "127.0.0.1:8080";
        if(!conf.ServerHost.empty())
            current_address = QString::fromStdString(conf.ServerHost) + ":" + QString::number(conf.ServerPort);

        auto index_html = v_dir.to_file("index.html");
        if(!index_html.exists()){
            if(conf.ServerHost.empty()){
                auto loc_addresses = local_host_addresses();
                if(loc_addresses.size() > 0){
                    current_address = loc_addresses[0] + ":8080";
                    conf.ServerHost = loc_addresses[0].toStdString();
                    conf.ServerPort = 8080;
                }
            }
            auto file_tmp = QFile(":/http/index.html");
            if(file_tmp.open(QIODevice::ReadOnly)){
                auto data = QString(file_tmp.readAll()).arg(ARCIRK_SERVER_NAME_);
                file_tmp.close();
                if(index_html.open(QIODevice::WriteOnly)){
                    index_html.write(data.toUtf8());
                    index_html.close();
                }else
                    return false;
            }else
                return false;
        }
        auto branding = v_dir.to_file("branding.cssl");
        if(!branding.exists()){
            auto file_tmp = QFile(":/http/branding.css");
            if(file_tmp.open(QIODevice::ReadOnly)){
                auto data = file_tmp.readAll();
                file_tmp.close();
                if(branding.open(QIODevice::WriteOnly)){
                    branding.write(data);
                    branding.close();
                }else
                    return false;
            }else
                return false;
        }
        auto bg_plain = v_dir.to_file("branding.cssl");
        if(!branding.exists()){
            auto file_tmp = QFile(":/http/bg-plain.jpg");
            if(file_tmp.open(QIODevice::ReadOnly)){
                auto data = file_tmp.readAll();
                file_tmp.close();
                if(bg_plain.open(QIODevice::WriteOnly)){
                    bg_plain.write(data);
                    bg_plain.close();
                }else
                    return false;
            }else
                return false;
        }

        QList<QString> subdirs = {"data", "ssl", "html/api/info", "html/api/files", "1c/bsl", "bin"};

        foreach (const auto& itr, subdirs) {
            v_dir << dir.path() /= itr;
            result = v_dir.mkpath();
            if(!result)
                return false;
        }

        return true;
    }
}

#endif // DIRECTORY_STRUCTURE_CHECK_HPP
