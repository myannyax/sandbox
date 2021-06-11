#pragma once

#include <iostream>
#include <yaml-cpp/yaml.h>

class YamlConfig {
private:
    YAML::Node config;
public:
    explicit YamlConfig(const std::string& config_path);

    template <class T>
    std::optional<T> get(const std::string& option_name);

    template <class T>
    T get(const std::string& option_name, const T& default_value);
};

template<class T>
std::optional<T> YamlConfig::get(const std::string &option_name) {
    if (config[option_name]) {
        return config[option_name].as<T>();
    }
    return {};
}

template<class T>
T YamlConfig::get(const std::string &option_name, const T &default_value) {
    if (config[option_name]) {
        return config[option_name].as<T>();
    }
    return default_value;
}
