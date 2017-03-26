#ifndef CONFIGURATIONSHANDLER_HPP
#define CONFIGURATIONSHANDLER_HPP

#include <memory>
#include <string>

#include <QDebug>

#include <libconfig.h++>


class ConfigurationsHandler {
public:
    ConfigurationsHandler() = default;
    ConfigurationsHandler(const QString& configFile);
    ~ConfigurationsHandler() = default;

    void init();
    void setConfigFile(const QString& fileName) { _fileName = fileName.toStdString().c_str(); }
    void addNewCollection(const QString& name, const QString& URL);

private:
    const char* _fileName;
    std::unique_ptr<libconfig::Config> _config;
    std::unique_ptr<libconfig::Setting> _collections;
};

#endif // CONFIGURATIONSHANDLER_HPP
