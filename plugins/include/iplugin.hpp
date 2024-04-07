#ifndef IPLUGIN_HPP
#define IPLUGIN_HPP
#include <QString>
#include <QVariant>
#include <QtPlugin>
#include <QWidget>

namespace arcirk::plugins {
    class IPlugin {
    public:
        virtual ~IPlugin() = default;

        virtual void setParam(const QString& param) = 0;
        virtual bool editParam(QWidget* parent) = 0;
        virtual bool isValid() = 0;
        virtual bool accept() = 0;
        virtual bool prepare() = 0;
        virtual QString param() const = 0;
        virtual QString lastError() const = 0;

    };
}

Q_DECLARE_INTERFACE( arcirk::plugins::IPlugin, "ru.arcirk.plugins.PluginInterface/1.0" )
#endif // IPLUGIN_HPP