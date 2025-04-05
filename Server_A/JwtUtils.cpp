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
            // TODO : 토큰 시간 만료 시간 일떄 클라이언트에게 jwt 만료 신호보내주자 클라이언트는 Node server에 jwt재발급을 요청할 것
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
