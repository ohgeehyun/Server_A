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

//전체 방 조회 
router.post('/savaChat',async (req, res) => {
    const { roomId } = req.body;
     const result = await room_chat.saveRoomChat(roomId);
    return res.status(200).json(result);
});


export default router; // ES6 default export