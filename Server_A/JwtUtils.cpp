#include "pch.h"
#include "JwtUtils.h"
#include "DataManager.h"
#include <jwt-cpp/jwt.h>
JwtUtils::JwtUtils()
{

}
JwtUtils::~JwtUtils()
{

}

bool JwtUtils::JwtVerify(string token, const char* log)
{
    string secretkey = DataManager::GetInstance().GetJWTSercretKey();

    try {
        auto decoded = jwt::decode(token);

        std::string payload = decoded.get_payload();
        payload_json = nlohmann::json::parse(payload);

        jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{ secretkey }) // 사용할 알고리즘과 비밀 키 지정
            .verify(decoded);

        _verifyStat = true;
        cout << log <<"sucesse jwt token verify! " << endl;
    }
    catch (const jwt::error::signature_verification_exception& e) {

        cout << log <<e.what() << endl;
        return false;
    }
    catch (const std::exception& e) {
        cout << log << e.what() << endl;
        return false;
    }
}

bool JwtUtils::JwtVerify(string& token, const char* log)
{
    try 
    {
        auto decoded = jwt::decode(token);
        jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{ DataManager::GetInstance().GetJWTSercretKey() })
            .verify(decoded);
        
        return true;
    }
    catch(const jwt::error::signature_verification_exception& e)
    {
        cout << log <<e.what() << endl;
        return false;
    }
    catch (const std::exception& e) {
        cout << log <<e.what() << endl;
        return false;
    }
}
