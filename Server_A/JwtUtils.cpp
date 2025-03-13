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

bool JwtUtils::GJwtVerify(const string& token, const char* log)
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
        std::error_code ec = e.code();

        if (ec == jwt::error::token_verification_error::token_expired) {
            //토큰 시간 만료 시간으로 인해 새로운 jwt 토큰 발급 후 요청 패킷 전송
            cout << log << " 토큰 시간 만료" << endl;
            return false;
        }

        //일반 검증 실패 에러 
        cout << log <<e.what() << endl;
        return false;
    }
    catch (const std::exception& e) {
        cout << log <<e.what() << endl;
        return false;
    }
}
