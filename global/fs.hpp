//
// Created by admin on 29.01.2024.
//

#ifndef CONTROLSPROG_FS_HPP
#define CONTROLSPROG_FS_HPP

#include <QFile>
#include <QDir>
#include <QStandardPaths>

namespace arcirk::filesystem{

    class FSPath{

    public:
        explicit FSPath(){};
        ~FSPath(){};

        void operator /=(const QString& v){
            m_path.append(QDir::separator());
            m_path.append(v);
        }

        QFile to_file(){
            return QFile(m_path);
        }

        QDir to_dir(){
            return QDir(m_path);
        }

        FSPath& init_app_data_dir(){
            m_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            auto d = QDir(m_path);
            if(!d.exists())
                d.mkpath(m_path);
            return *this;
        }


        QString path(){
            return m_path;
        }

        bool mkpath(){
            auto d = QDir(m_path);
            if(!d.exists())
                return d.mkpath(m_path);
            else
                return true;
        }

        bool file_exists(){
            auto f = QFile(m_path);
            return f.exists();
        }

        bool dir_exists(){
            auto f = QDir(m_path);
            return f.exists();
        }

    private:
        QString m_path;
    };

}

#endif //CONTROLSPROG_FS_HPP
