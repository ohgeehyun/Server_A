#pragma once
#include "nlohmann/json.hpp"
#include "Protocol.pb.h"
template <typename Key, typename Value>
class ILoader {
public:
    virtual HashMap<Key, Value> MakeDict() abstract;
    virtual ~ILoader() = default;
};

/*------------------------------------------
*                   stat
--------------------------------------------*/
class Stat
{
public:
    int32 level;
    int32 maxHp;
    int32 attack;
    int32 totalExp;

    void Deserialize(const nlohmann::json& j) {
        level = j.at("level").get<int32>();
        maxHp = j.at("maxHp").get<int32>();
        attack = j.at("attack").get<int32>();
        totalExp = j.at("totalExp").get<int32>();
    }
};

class StatData : public ILoader<int, Stat>
{

public:
    HashMap<int, Stat> MakeDict() override;
    void Deserialize(const nlohmann::json& j);

public:
    List<Stat> stats;
 
};

/*-----------------------------
           projectile
------------------------------*/
class ProjectileInfo
{
public:
    string name;
    float speed;
    int32 range;
    string prefab;

    void Deserialize(const nlohmann::json& j) {
        name = j.at("name").get<std::string>();
        speed = j.at("speed").get<float>();
        range = j.at("range").get<int32>();
        prefab = j.at("prefab").get<std::string>();
    }
};

/*-----------------------------
              skill
------------------------------*/
class Skill
{

public:


public:
    int32 id;
    string name;
    float cooldown;
    int32 damege; 
    Protocol::SkillType skillType;
    ProjectileInfo projectile;

    void Deserialize(const nlohmann::json& j) {
        id = j.at("id").get<int32>();
        name = j.at("name").get<string>();
        cooldown = j.at("cooldown").get<float>();
        damege = j.at("damege").get<int32>();
        skillType = ParseSkillType(j.at("skillType").get<string>());
        if (j.contains("projectile")) {
            projectile.Deserialize(j.at("projectile"));
        }
    }

private:
    Protocol::SkillType ParseSkillType(const std::string& str); // 문자열 -> 열거형 변환
};


class SkillData : public ILoader<int, Skill>
{

public:
    HashMap<int, Skill> MakeDict() override;
    void Deserialize(const nlohmann::json& j);

public:
    List<Skill> skills;

};