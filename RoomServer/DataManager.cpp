#include "pch.h"
#include "DataManager.h"
#include <fstream>
#include <cstdlib>
#include "nlohmann/json.hpp"
#include "ConfigManager.h"
DataManager::DataManager()
{
    Init();
}
void DataManager::Init()
{
    auto statData = LoadStat<StatData>();
    _statDict = statData->MakeDict();
    auto skillData = LoadSkill<SkillData>();
    _skillDict = skillData->MakeDict();
    auto serverConfigData = LoadServerConfig<ServerConfigData>();
    _severConfigDict = serverConfigData->MakeDict();

    LoadJWT_SecretKey();
}

void DataManager::LoadJWT_SecretKey()
{
    std::ifstream envFile(ConfigManager::GetInstance()._config.keyPath);
    std::string line;

    if (envFile.is_open()) 
    {
        while (std::getline(envFile, line)) 
        {

            //주석 및 공백 라인무시
            if (line.empty() || line[0] == '#')
                continue;

            //키와 값 분리
            size_t eqPos = line.find('=');
            if (eqPos != std::string::npos)
            {
                std::string key = line.substr(0, eqPos);
                std::string value = line.substr(eqPos + 1);

                //찾은 secretKey를 저장
                if (key == "JWT_SECRET_KEY") 
                {
                    _jwtSecretKey=value;
                }
            }
        }
        envFile.close();
    }
    else
    {
        std::cerr << "Error: Unable to open .env file." << std::endl;
    }
}


template <typename Loader>
std::unique_ptr<Loader> DataManager::LoadStat() {
    std::ifstream file(ConfigManager::GetInstance()._config.statPath +".json");
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: ");
        cout << ConfigManager::GetInstance()._config.statPath + ".json" << endl;
    }

    nlohmann::json j;
    file >> j;

    auto loader = std::make_unique<Loader>();
    loader->Deserialize(j);
    return loader;
}

template<typename Loader>
unique_ptr<Loader> DataManager::LoadSkill()
{
    std::ifstream file(ConfigManager::GetInstance()._config.skillPath + ".json");
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: ");
        cout << ConfigManager::GetInstance()._config.skillPath + ".json" << endl;
    }

    nlohmann::json j;
    file >> j;

    auto loader = std::make_unique<Loader>();
    loader->Deserialize(j);
    return loader;
}

template<typename Loader>
unique_ptr<Loader> DataManager::LoadServerConfig()
{
    std::ifstream file(ConfigManager::GetInstance()._config.configPath + ".json");
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: ");
        cout << ConfigManager::GetInstance()._config.configPath + ".json" << endl;
    }

    nlohmann::json j;
    file >> j;

    auto loader = std::make_unique<Loader>();
    loader->Deserialize(j);
    return loader;
}



