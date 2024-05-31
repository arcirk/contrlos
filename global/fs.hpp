//
// Created by admin on 29.01.2024.
//

#ifndef CONTROLSPROG_FS_HPP
#define CONTROLSPROG_FS_HPP

#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QHostAddress>
#include <QNetworkInterface>

namespace arcirk::filesystem{

    class FSPath{

    public:
        explicit FSPath(){};
        explicit FSPath(const QString& value){m_path = value;};
        ~FSPath(){};

        FSPath& operator /=(const QString& v){
            m_path.append(QDir::separator());
            m_path.append(v);
            return *this;
        }

        FSPath& operator =(const QString& v){
            m_path = v;
            return *this;
        }

        QFile to_file(){
            return QFile(m_path);
        }

        QFile to_file(const QString& fileName){
            return QFile(m_path + QDir::separator() + fileName);
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

        FSPath& operator <<(const QDir& v){
            m_path = v.absolutePath();
            return *this;
        }

        FSPath& operator <<(const QFile& v){
            m_path = v.fileName();
            return *this;
        }

        FSPath& operator <<(const QString& v){
            m_path = v;
            return *this;
        }

    private:
        QString m_path;
    };

    static inline QList<QString> local_host_addresses(){
        QList<QString> m_local_addresses;
        const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
        for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost)
                m_local_addresses.append(address.toString());
        }
        return m_local_addresses;
    }
}

#endif //CONTROLSPROG_FS_HPP
