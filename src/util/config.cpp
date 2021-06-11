#include "config.h"

YamlConfig::YamlConfig(const std::string &config_path) : config(YAML::LoadFile(config_path)) {}
