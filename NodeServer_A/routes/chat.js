import express from 'express';
import Redis from '../Redis/connection_pool.js';
import room_chat from '../Redis/room_chat.js';

const router = express.Router();
router.use(express.json());

router.use(async (req, res, next) => {
  if(await Redis.isRedisConnected()) //DB 연결 체크
    next();
  else
    res.status(500).send('middle ware err : redis disconnected');
});

//채팅 저장
router.post('/sava',async (req, res) => {
    const { roomId } = req.body;

    if(!roomId)
      return res.status(400).send({status: false, code:400,message: '요청에 Room id 가 없습니다.'});
  
    const result = await room_chat.saveRoomChat(roomId);
    
    if(result)
      return res.status(201).json(result);
    else
    {
      return res.status(500).json(result);
    }
});


export default router; // ES6 default export