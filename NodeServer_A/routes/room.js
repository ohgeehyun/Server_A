import express from 'express';
import Redis from '../Redis/connection_pool.js';
import Redis_Room from '../Redis/Room.js';
import jwt from '../jsonwebtoken/jwt.js';

const router = express.Router();
router.use(express.json());
//router.use(jwt.authenticateToken);

router.use(async (req, res, next) => {
  if(await Redis.isRedisConnected()) //DB 연결 체크
    next();
  else
    res.status(500).send('middle ware err : redis disconnected');
});

//전체 방 조회 
router.get('/list',async (req, res) => {
    const { room_Name } = req.query;

     //room_Name이 없으면 전체 방 조회
     const result = await Redis_Room.getRoomList(room_Name);
     console.log(result)
     if(result.status)
        return res.status(200).json(result);
     else
     {
        return res.status(500).json(result);
     }
});

//비밀번호 체크
router.post('/GetRoomPwd',async (req, res) => {
  const { Room_id,Room_pwd } = req.body;

  if(!Room_id && !Room_pwd)
    return res.status(400).send({status: false, code:400,message: '요청에 Room id 및 password 가 없습니다.'});
  
  const result = await Redis_Room.getRoomPwd(Room_id,Room_pwd);

  if(result.status)
    res.status(200).json(result);
  else
  {
    switch(result.code)
    {
      case 401:
        return res.status(401).json(result);  //패스워드 불일치
      case 404:
        return res.status(404).json(result);  //해당 방이 존재하지 않음.
      case 500:
        return res.status(500).json(result); // 조회 중 서버 오류 발생
    }
  }
});

//방 scoreBoardData 조회
router.post('/GetScoreBoard',async (req, res) => {
  const { Room_id } = req.body;

  if(!Room_id)
    return res.status(400).send({status: false, code:400,message: '요청에 Room_id가 없습니다.'});

  const result = await Redis_Room.getScoreBoardData(Room_id);

  if(result.status)
    return res.status(200).json(result);
  else
    return res.status(500).json(result);
});



export default router; // ES6 default export