#include "pch.h"
#include "ConfigManager.h"
#include <fstream>
#include "nlohmann/json.hpp"
#include "DataContent.h"

void ConfigManager::LoadConfig()
{
    
    std::ifstream ifs("../Binary/Debug/config.json");

    if (!ifs.is_open()) {
        throw std::runtime_error("Could not open config.json");
    }
   
    //파일 내용을 문자열에 담기
    nlohmann::json j;
    ifs >> j;


    // JSON 데이터를 ServerConfig 객체에 역직렬화
    ServerConfig config;
    config.statPath = j.at("statPath").get<std::string>();
    config.skillPath = j.at("skillPath").get<std::string>();
    config.configPath = j.at("configPath").get<std::string>();
    config.keyPath = j.at("keyPath").get<std::string>();
    SetServerConfig(config);

    std::cout << "Config loaded successfully: " << config.statPath << std::endl;
    std::cout << "Config loaded successfully: " << config.skillPath << std::endl;
    std::cout << "Config loaded successfully: " << config.configPath << std::endl;
    std::cout << "Config loaded successfully: " << config.keyPath << std::endl;
}
