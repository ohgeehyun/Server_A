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
router.post('/GetRoomList',async (req, res) => {
    const { room_Name } = req.body;
     const result = await Redis_Room.getRoomList(room_Name);
     console.log(result)
    return res.status(200).json(result);
});

//비밀번호 체크
router.post('/GetRoomPwd',async (req, res) => {
  const { Room_id,Room_pwd } = req.body;
  const result = await Redis_Room.getRoomPwd(Room_id,Room_pwd);
  return res.status(200).json(result);
});

//방 scoreBoardData 조회
router.post('/GetScoreBoard',async (req, res) => {
  const { Room_id } = req.body;
  const result = await Redis_Room.getScoreBoardData(Room_id);
  return res.status(200).json(result);
});



export default router; // ES6 default export