#pragma once

class JsonUtils
{
public:
    template<typename ...Args>
    static nlohmann::json createJson(Args&&...args);
  
private:
    template<typename T>
    static void add_to_json(nlohmann::json& json_obj, T&& arg);
    template<typename T,typename... Args>
    static void add_to_json(nlohmann::json& json_obj, T&& arg, Args&&... args);

};

template<typename ...Args>
inline nlohmann::json JsonUtils::createJson(Args && ...args)
{
    nlohmann::json json_obj;
    add_to_json(json_obj,std::forward<Args>(args)...);
    return json_obj;
}

template<typename T>
inline void JsonUtils::add_to_json(nlohmann::json& json_obj, T&& arg)
{
    json_obj[std::forward<T>(arg).first] = std::forward<T>(arg).second;
}

template<typename T, typename ...Args>
inline void JsonUtils::add_to_json(nlohmann::json& json_obj, T&& arg, Args && ...args)
{
    json_obj[std::forward<T>(arg).first] = std::forward<T>(arg).second;
    add_to_json(json_obj, std::forward<Args>(args)...);
}

