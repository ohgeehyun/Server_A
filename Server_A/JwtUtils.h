#pragma once
#include <nlohmann/json.hpp>
class JwtUtils
{
public:
    JwtUtils();
    ~JwtUtils();

    bool JwtVerify(string token, const char* log);
    bool GetVerifyStat() { return _verifyStat; }
    nlohmann::json GetPayload_Json() { return payload_json; }

    //토큰 검증만을 위한 함수 
    static bool GJwtVerify(const string& token, const char* log);


private:
    nlohmann::json payload_json;
    bool _verifyStat = false;
};

