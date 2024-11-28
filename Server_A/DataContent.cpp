#include "pch.h"
#include "DataContent.h"

HashMap<int, Stat> StatData::MakeDict()  
{
    HashMap<int, Stat> dict;

    for (const auto& stat : stats)
    {
        dict[stat.level] = stat;
    }
    return dict;
}

void StatData::Deserialize(const nlohmann::json& j) {
    for (const auto& statJson : j["stats"]) {
        Stat stat;
        stat.Deserialize(statJson);
        stats.push_back(stat);
    }
}

HashMap<int, Skill> SkillData::MakeDict()
{
    HashMap<int, Skill> dict;

    for (const auto& skill : skills)
    {
        dict[skill.id] = skill;
    }
    return dict;
}

void SkillData::Deserialize(const nlohmann::json& j)
{
    for (const auto& statJson : j["skills"]) {
        Skill skill;
        skill.Deserialize(statJson);
        skills.push_back(skill);
    }
}

Protocol::SkillType Skill::ParseSkillType(const string& str)
{
    Protocol::SkillType type;
    if (Protocol::SkillType_Parse(str, &type)) {
        return type; // 성공적으로 변환된 경우
    }
    throw std::runtime_error("Invalid SkillType string: " + str);
}
