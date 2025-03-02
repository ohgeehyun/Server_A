#pragma once
#include "DataContent.h"

class DataManager
{
public:
    DataManager();
    ~DataManager()= default;
    static DataManager& GetInstance() {
        static DataManager instance;
        return instance;
    };

    void Init();
    const HashMap<int32, Protocol::STATINFO>& GetStatDict() const { return _statDict; }
    const HashMap<int32, Skill>& GetSkillDict() const { return _skillDict; }
    const HashMap<string, ServerConfigData>& GetServerConfigDict() const { return _severConfigDict; }
    const string GetJWTSercretKey() const { return _jwtSecretKey; };
    template <typename Loader>
    unique_ptr<Loader> LoadStat();
    template <typename Loader>
    unique_ptr<Loader> LoadSkill();
    template <typename Loader>
    unique_ptr<Loader> LoadServerConfig();

    void LoadJWT_SecretKey();

private:

    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager&) = delete; 
    
private:
    HashMap<int32, Protocol::STATINFO> _statDict;
    HashMap<int32, Skill> _skillDict;
    HashMap<string,ServerConfigData>_severConfigDict;
    string _jwtSecretKey;
  
};

