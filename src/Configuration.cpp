#include <iostream>
#include <sstream>
#include "../include/Configuration.h"

Configuration::Configuration(const std::string& config_path) {
    try {
        cfg.readFile(config_path.c_str());
    } catch(const libconfig::FileIOException &fioex) {
        throw std::runtime_error("I/O error while reading config file.");
    } catch(const libconfig::ParseException &pex) {
        std::ostringstream error;
        error << "Parse error at " << pex.getFile() << ":" << pex.getLine()
              << " - " << pex.getError() << std::endl;
        throw std::runtime_error(error.str());
    }
}

std::string Configuration::get_string_entry(const std::string& key) {
    std::string val;
    try {
         val = (std::string) cfg.lookup(key);
    } catch(const libconfig::SettingNotFoundException &nfex) {

    }
    return val;
}

int Configuration::get_int_entry(const std::string& key) {
    int val;
    try {
        val = cfg.lookup(key);
    } catch(const libconfig::SettingNotFoundException &nfex) {
        val = -1;
    }
    return val;
}

void Configuration::get_storage_classes() {
    const libconfig::Setting& root = cfg.getRoot();
    const libconfig::Setting& storage_classes = root["storage_classes"];
    std::cout << storage_classes.getLength() << std::endl;
}