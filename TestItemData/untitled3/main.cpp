#include <QCoreApplication>
#include <global.hpp>
#include <variant/item_data.h>
#include <QUuid>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

//    auto data = json::object({
//                              {"ref", arcirk::to_byte(arcirk::to_binary(QUuid()))}
//    });

    //json data = arcirk::to_byte(arcirk::to_binary(QUuid()));
    //json data = arcirk::to_byte(arcirk::to_binary(""));
    auto item1 = arcirk::widgets::item_data(json("jj"));
    auto data = item1.to_byte();
    auto item = arcirk::widgets::item_data();
    item.from_json(data);


    qDebug() << item.role();
    qDebug() << item.data()->subtype;
    qDebug() << item.representation().c_str();

    return a.exec();
}
