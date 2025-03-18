import jwt from 'jsonwebtoken';

// JWT 생성 함수
function generateToken(user_id,nickname) {
    const secretKey = process.env.JWT_SECRET_KEY;
    console.log(secretKey);
    const expiration = '12h';
    const algorithm = 'HS256';

    //user 객체를 payload로 사용하여 Jwt생성
    const token = jwt.sign({user_id,nickname},secretKey,{expiresIn: expiration,algorithm:algorithm});
    return token;
}

//jwt 검증 
function verifyToken(token)
{
    const secretKey = process.env.JWT_SECRET_KEY;

    try{   
        //토큰 검증후 playload 반환
        const decoded = jwt.verify(token,secretKey);
        return decoded;
    }catch(err){
        throw new Error('Invalid token');
    }
}

//jwt 인증 미들웨어
function authenticateToken(req,res,next)
{
    //헤더에 jwt토큰을 포함시켜 요청을 보냄 일반적으론 Bearer방식을 사용
    const token = req.headers['authorization'] && req.headers['authorization'].split(' ')[1];

    if(!token){
        return res.status(200).json({status:500, message:'토큰이 없습니다.'});
    }

    try{
        const decoded = verifyToken(token);
        req.user = decoded;
        next();
    }catch(err){
        return res.status(200).json({status:501, message:'유효하지 않은 토큰 입니다.'})
    }
}

export default {
    generateToken,
    verifyToken,
    authenticateToken
}
