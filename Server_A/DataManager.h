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
    const HashMap<int32, Stat>& GetStatDict() const { return _statDict; }
    const HashMap<int32, Skill>& GetSkillDict() const { return _skillDict; }
    template <typename Loader>
    unique_ptr<Loader> LoadStat();
    template <typename Loader>
    unique_ptr<Loader> LoadSkill();
private:

    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager&) = delete; 
    
private:
    HashMap<int32, Stat> _statDict;
    HashMap<int32, Skill> _skillDict;
};

