// routes/users.js
import express from 'express';
import Database from '../Db/connection_pool.js';
import User_DB from '../Db/User.js';
import jwt from '../jsonwebtoken/jwt.js';



const router = express.Router();
router.use(express.json());


router.use(async (req, res, next) => {
  if(await Database.isConnection()) //DB 연결 체크
    next();
  else
    res.status(500).send('DB 연결 실패');
});

//유저 정보 검색
router.post('/GetUserInfo',async (req, res) => {
  const { user_Id } = req.body;

  if (!user_Id) 
    return res.status(200).send({status: false, message: '요청에 id가 없습니다.'});
  
  const result = await User_DB.GetUserInfo(user_Id);

  console.log(result);
  if(result.status)
    return res.status(200).json(result); //성공적으로 데이터  찾았으면 200과 result를 반환
  else
    return res.status(200).json(result); //실패는 아니다 성공이다 하지만 유저정보가없을뿐
});

//유저 등록
router.post('/RegisterUser',async (req,res)=> {
  const {user_Id,password,nickname}  = req.body;
  
  if(!user_Id && !password && ! nickname)
    return res.status(200).send({status: false, message: '요청에 id 및 password 가 없습니다.'});

  const result = await User_DB.RegisterUser(user_Id,password,nickname);
  console.log(result);
  if(result.status)
    return res.status(200).json(result); //성공적으로 데이터  찾았으면 200과 result를 반환
  else
    return res.status(200).json(result); //실패는 아니다 성공이다 하지만 중복된 아이디 검출
});

//로그인 요청
router.post('/LoginUser',async (req,res) => {
  const {user_Id,password} = req.body;

  if(!user_Id && !password)
    return res.status(200).send({status: false, message: '요청에 id 및 password 가 없습니다.'});

  const result = await User_DB.LoginUser(user_Id,password);
  console.log(result);

  if(result.status){
    const token = jwt.generateToken(user_Id,result.result.nickname);
    console.log(token);
    return res.status(200).json({ status: true,message:'200',token:token}); //성공적으로 데이터  찾았으면 200과 result를 반환
  }else{
    return res.status(200).json(result); //실패는 아니다 성공이다 하지만 중복된 아이디 검출
  }
});





export default router; 