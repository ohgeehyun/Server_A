#include "pch.h"
#include "DataContent.h"
#include "Protocol.pb.h"
#include <google/protobuf/util/json_util.h>

HashMap<int, Protocol::STATINFO> StatData::MakeDict()
{
    HashMap<int, Protocol::STATINFO> dict;

    for (auto& stat : stats)
    {
        stat.set_hp(stat.maxhp());
        dict[stat.level()] = stat;
    }
    return dict;
}

void StatData::Deserialize(const nlohmann::json& j) {
    for (const auto& statJson : j["stats"]) {
        
        Protocol::STATINFO stat;

        std::string jsonString = statJson.dump();

        auto status = google::protobuf::util::JsonStringToMessage(jsonString, &stat);
        if (!status.ok()) {
            throw std::runtime_error("Failed to parse JSON to Protobuf: " + status.message().as_string());
        }
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

void ServerConfigData::Deserialize(const nlohmann::json& j)
{
    mysqlData.host = j["Mysql"]["host"].get<std::string>();
    mysqlData.port = j["Mysql"]["port"].get<std::string>();
    mysqlData.user = j["Mysql"]["user"].get<std::string>();
    mysqlData.pwd = j["Mysql"]["pwd"].get<std::string>();
    mysqlData.dbname = j["Mysql"]["dbname"].get<std::string>();

    redisData.host = j["Redis"]["host"].get<std::string>();
    redisData.port = j["Redis"]["port"].get<int>();
    redisData.auth = j["Redis"]["auth"].get<std::string>();
}

HashMap<string, ServerConfigData> ServerConfigData::MakeDict()
{
    HashMap<string, ServerConfigData> dict;

    dict["database"] = *this ;

    return dict;
}
