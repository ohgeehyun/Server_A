#include "pch.h"
#include "DataManager.h"
#include <fstream>
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


