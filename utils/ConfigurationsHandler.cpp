#include "ConfigurationsHandler.hpp"

ConfigurationsHandler::ConfigurationsHandler(const QString& configFile) {
    _fileName = configFile.toStdString().c_str();
}

void ConfigurationsHandler::init() {
    _config = std::unique_ptr<libconfig::Config>(new libconfig::Config());
    try {
        _config->readFile(_fileName);
    }
    catch (const libconfig::FileIOException& ex) {
        qWarning() << "Configuration file I/O error!";
        qInfo() << "creating new configuration file...";
        _config->writeFile(_fileName);
    }

    libconfig::Setting& root = _config->getRoot();
    // create the collection if it doesnt exists
    if (root.exists("collections") == 0) {
        root.add("collections", libconfig::Setting::TypeList);
    }
    // load the collections
    _collections = std::unique_ptr<libconfig::Setting>(&root["collections"]);
    int count = _collections->getLength();
    //qInfo() << "collection count" << count;
    for (const auto& collection : *_collections.get()) {
        std::string name;
        collection.lookupValue("name", name);
        std::string URL;
        collection.lookupValue("URL", URL);
        //qInfo() << "name:" << QString::fromStdString(name) << "URL:" << QString::fromStdString(URL);
    }
}

void ConfigurationsHandler::addNewCollection(const QString& name, const QString& URL) {
    qInfo() << "creating new collection index...";
    // add new entry
    libconfig::Setting& collection = _collections->add(libconfig::Setting::TypeGroup);
    collection.add("name", libconfig::Setting::TypeString) = name.toStdString();
    collection.add("URL", libconfig::Setting::TypeString) = URL.toStdString();

    qInfo() << "saving the configurations...";
    // write out the updated configuration
    _config->writeFile(_fileName);
}

