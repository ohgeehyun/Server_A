// routes/users.js
import express from 'express';
import Database from '../Db/connection_pool.js';
import User_DB from '../Db/User.js';
import jwt from '../jsonwebtoken/jwt.js';
import { json } from 'stream/consumers';



const router = express.Router();
router.use(express.json());


router.use(async (req, res, next) => {
  if(await Database.isConnection()) //DB 연결 체크
    next();
  else
    res.status(500).send({status:false, message : 'Database not connected'});
});

//유저 정보 검색
router.get('/UserInfo',async (req, res) => {
  const { user_Id } = req.query;

  if (!user_Id) 
    return res.status(400).send({status: false, message: '요청에 id가 없습니다.'});
  
  const result = await User_DB.GetUserInfo(user_Id);

  console.log(result);
  if(result.status)
    return res.status(200).json(result); //성공적으로 데이터  찾았으면 200과 result를 반환
  else
  {
    switch(result.code)
    {
      case 404 :
        return res.status(404).json(result); //유저 정보 가 없는 경우.
      case 500 :
        return res.status(500).json(result);// DB관련 문제 접속이 끊겼거나 DB에서 조회 문제 발생
      default:
        return res.status(400).json(result); // 기본적인 에러 처리
    }
  }
});

//유저 등록
router.post('/register',async (req,res)=> {
  const {user_Id,password,nickname}  = req.body;
  
  if(!user_Id && !password && ! nickname)
    return res.status(400).send({status: false, message: '요청에 id 및 password 가 없습니다.'});

  const result = await User_DB.RegisterUser(user_Id,password,nickname);
  console.log(result);
  if(result.status)
    return res.status(201).json(result); //성공적으로 데이터 등록완료  200과 result를 반환
  else
  {
    switch(result.code)
    {
      case 409: 
        return res.status(409).json(result); //실패는 아니다 성공이다 하지만 중복된 아이디 검출   
      case 500:
        return res.status(500).json(result); //db 또는 서버 로직처리중 오류
    }
  }
});

//로그인 요청
router.post('/login',async (req,res) => {
  const {user_Id,password} = req.body;

  if(!user_Id && !password)
    return res.status(400).send({status: false, message: '요청에 id 및 password 가 없습니다.'});

  const result = await User_DB.LoginUser(user_Id,password);
  console.log(result);

  if(result.status){
    const token = jwt.generateToken(user_Id,result.result.nickname);
    console.log(token);
    return res.status(200).json({ status: true,message:'200',token:token}); //성공적으로 데이터  찾았으면 200과 result를 반환
  }else{
    switch(result.code)
    {
      case 401: 
        return res.status(401).json(result); //비밀번호 오류
      case 404:
        return res.status(404).json(result); //아이디 찾기 실패
      default:
        return res.status(500).json(result);

    }
  }
});

export default router; 