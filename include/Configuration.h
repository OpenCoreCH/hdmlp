#ifndef HDMLP_CONFIGURATION_H
#define HDMLP_CONFIGURATION_H


#include <string>
#include <libconfig.h++>

class Configuration {
public:
    explicit Configuration(const std::string& config_path);
    std::string get_string_entry(const std::string& key);
    int get_int_entry(const std::string& key);
    void get_storage_classes();

private:
    libconfig::Config cfg;

};


#endif //HDMLP_CONFIGURATION_H
