#ifndef BANKSTATEMENTSPLUGUN_H
#define BANKSTATEMENTSPLUGUN_H

#include "../global/global.hpp"
#include "../plugins/include/iplugin.hpp"
#include <QObject>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QtPdf/QPdfDocument>
#include <QtPdf/QPdfSelection>

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::plugins), plugin_param,
    (std::string, key)
    (arcirk::ByteArray, value)
    (int, is_group)
    (std::string, ref)
    (std::string, parent)
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::plugins), task_param,
    (std::string, host) //хост 1С
    (std::string, token) // base64(user:password)
    (std::string, ib) //имя ИБ на хосте
    (bool, remove) // удалять не удалять файлы из папки "Загрузки"
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::plugins), pay_details,
    (std::string, parent)
    (std::string, pay)
    (std::string, parent_name)
    (std::string, pdf_dir)//каталог файлов pdf
    (std::string, txt_dir) //каталог выписок 1С
    )

#define PLUGIN_FILE_NAME "BankStatements.json"


namespace arcirk::plugins {

    typedef QMap<QString, pay_details > PayArray;

    struct param_t
    {
        param_t() {}
        param_t(const QByteArray data) {
            fromRaw(data);
        }

        QList<QByteArray> files;
        QList<QString> names;
        QList<QDateTime> creates;
        QList<QString> suffix;
        QList<QString> loc_paths;
        QList<QString> pay;

        bool verifyText(const QString& file, QByteArray& data, const PayArray& pays, QString& p){
            QFile f(file);
            bool isFind = false;
            if(f.open(QIODevice::ReadOnly)){
                data = f.readAll();
                f.close();
                if(data.indexOf("1CClientBankExchange") != -1){
                    for (auto itr = pays.begin(); itr != pays.end(); ++itr) {
                        if(data.indexOf(itr.key().toStdString()) != -1){
                            isFind = true;
                            p = itr.key();
                            break;
                        }
                    }
                    return isFind;
                }else
                     data = {};
            }
            return false;
        }
        bool verifyPdf(const QString& file, QByteArray& data, const PayArray& pays, QString& p){
            auto pdf = QPdfDocument();
            auto res = pdf.load(file);
            if(res == QPdfDocument::Error::None){
                auto ba = pdf.getAllText(0);
                auto text = ba.text();
                pdf.close();
                bool isFind = false;
                for (auto itr = pays.begin(); itr != pays.end(); ++itr) {
                    if(text.indexOf(itr.key()) != -1){
                        isFind = true;
                        p = itr.key();
                        break;
                    }
                }
                if(isFind){
                    QFile f(file);
                    if(f.open(QIODevice::ReadOnly)){
                        data = f.readAll();
                        f.close();
                        return true;
                    }
                }
            }
            return false;
        }

        void read(const QString& file, const PayArray& pays){
            QFileInfo fs(file);
            QByteArray data;
            QString m_pay;
            if(fs.suffix() == "txt"){
                if(!verifyText(file, data, pays, m_pay)){
                    return;
                }
            }else if(fs.suffix() == "pdf"){
                if(!verifyPdf(file, data, pays, m_pay)){
                    return;
                }
            }
            files.push_back(data);
            auto name = fs.fileName().left(fs.fileName().length() - fs.suffix().length() - 1);
            names.push_back(name);
            suffix.push_back(fs.suffix());
            creates.push_back(fs.lastModified());
            loc_paths.push_back(fs.fileName());
            pay.push_back(m_pay);
        }

        QByteArray toRaw() const{
            QByteArray ba;
            QDataStream stream {&ba, QIODevice::WriteOnly};
            stream << files;
            stream << names;
            stream << creates;
            stream << suffix;
            stream << loc_paths;
            stream << pay;
            return ba;
        }

        void fromRaw(const QByteArray& data){
            if(data.size() == 0)
                return;
            QDataStream stream {data};
            stream >> files;
            stream >> names;
            stream >> creates;
            stream >> suffix;
            stream >> loc_paths;
            stream >> pay;
        }

        QString generateName(int index){
            Q_ASSERT(index < files.size());
            Q_ASSERT(index>=0);
            QString result;
            if(names[index].indexOf("Выписка за") != -1){
                result = creates[index].toString("dd.MM.yy.hh.mm.ss");
            }
            return result;
        }
    };

    class BankStatementsPlugun : public QObject, public IPlugin
    {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "ru.arcirk.plugins.PluginInterface/1.0")
        Q_INTERFACES( arcirk::plugins::IPlugin )
    public:
        ~BankStatementsPlugun();

        void setParam(const QString& param) override;
        bool isValid() override;
        bool accept() override;
        bool prepare() override;
        QString param() const override;
        bool editParam(QWidget* parent) override;
        QString lastError() const override;

    private:
        bool is_valid;
        json m_param;
        QString m_last_error;

        static json default_param() ;
        static json read_param() ;
        void write_param();
        static json read_value(const ByteArray& value) ;

        static QByteArray readData(PayArray& pays) ;
        static json http_get(const QString& command, const json& param, const task_param& t_param);

        static bool pay_list(PayArray& pays, const task_param& t_param);
    };

}


#endif // BANKSTATEMENTSPLUGUN_H
