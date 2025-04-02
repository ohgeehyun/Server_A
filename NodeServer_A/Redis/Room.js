import Redis from './connection_pool.js'
import { promisify } from 'util';

// GET 명령어를 비동기로 실행할 수 있도록 promisify 사용
const getAsync = promisify(Redis.redis.get).bind(Redis.redis);
// SCAN 명령을 비동기로 실행할 수 있도록 promisify 사용
const scanAsync = promisify(Redis.redis.scan).bind(Redis.redis);
// HGETAll 명령을 비동기로 실행할 수 있도록 promisify 사용
const hgetallAsync = promisify(Redis.redis.hgetall).bind(Redis.redis);

//룸 아이디를 이용한 방 정보 조회
async function getRoomInfo(roomId) {
    try {
      // Redis에서 해당 방 정보를 가져오기 (JSON 문자열)
      const jsonStr = await getAsync(`room:${roomId}`);
      
      if (jsonStr) {
        // JSON 문자열을 JavaScript 객체로 변환
        const roomInfo = JSON.parse(jsonStr);
        console.log('Room Info:', roomInfo);
        //return roomInfo;
      } else {
        console.log('해당 roomId에 대한 정보가 존재하지 않습니다.');
        return null;
      }
    } catch (err) {
      console.error('Redis에서 방 정보 가져오기 실패:', err);
    }
  }

  //방 이름을 이용한 방 검색
  async function getRoomList(room_Name) {
    try {
      let cursor = '0'; // SCAN 시작 커서
      const allRooms = [];
      // SCAN 반복 실행
      do {
        const [newCursor, keys] = await scanAsync(cursor, 'MATCH', 'room:*');
        cursor = newCursor; // 새로운 커서 값 설정
  
        console.log('Returned cursor:', cursor);
        console.log('Keys returned:', keys);

        // 각 키에 대해 데이터를 가져옴
        for (const key of keys) 
        {
          console.log(`Fetching key: ${key}`); 
          const jsonStr = await Redis.redis.get(key);
        
          if (jsonStr) 
          {
            const roomInfo = JSON.parse(jsonStr); // JSON 문자열을 객체로 변환

            //SCARD 명령어로  해당 방의 유저 수 search
            const userCountKey =`room_user:${roomInfo.id}`
            const userCount = await Redis.redis.scard(userCountKey); //유저 수 조회

            roomInfo.userCount = userCount;
            
            if(room_Name === undefined||room_Name===null || room_Name==="")
                allRooms.push(roomInfo)
            else if (roomInfo.name === room_Name)
                allRooms.push(roomInfo);
          }
        }
      } while (cursor !== '0'); // 커서가 0이면 종료
      return { status: true, code:200,data: allRooms};
    } 
    catch (err) 
    {
      return { status: false,code:500,message: 'Redis roomlist 조회 중 오류가 발생했습니다.' };
    }
  }

  //방의 비밀번호 체크 true false 반환
  async function getRoomPwd(roomId,client_pwd) {
    try
    {
      const jsonStr = await getAsync(`room:${roomId}`);

      if(jsonStr)
      {
        const roomInfo = JSON.parse(jsonStr);
        //비밀번호 비교
        if(roomInfo.password === client_pwd)
          return {status:true,code:200,message:'패스워드 일치'};
        else
          return {status:false,code:401,message:'패스워드 불일치'};
      }
      else
        return {status:false,code:404,message:'해당 방이 존재하지 않거나 정보를 찾을 수 없습니다.'};
    }
    catch(err)
    {
      return { status: false,code:500, message: 'DB 조회 중 오류가 발생했습니다.' };
    }
  }

  //Room ScoreBoard data load
  async function getScoreBoardData(roomId) {
    try
    {
      // Redis에서 해당 방(roomId) 내 모든 유저의 데이터를 가져옴
      const keysAsync = promisify(Redis.redis.keys).bind(Redis.redis);
      const playerKeys = await keysAsync(`room_score:${roomId}:*`);

      if (playerKeys.length === 0) {
        return { status: false, code:404, message: '해당 방에 유저가없음...' };
      }

      const scoreBoard = [];

      for (const key of playerKeys) {
        const playerData = await hgetallAsync(key);
        const playerId = key.split(':').pop(); // key에서 playerId 추출
  
        scoreBoard.push({
          nickname: playerData.nickname || 'Unknown',
          kill: playerData.kill || 0,
          death: playerData.death || 0,
          damege: playerData.TotalDamege || 0
        });
      }
      console.log(scoreBoard)
      return { status: true,code:200,data: scoreBoard};
    }
    catch(err)
    {
      console.log(err);
      return { status: false,code:500, message: 'ScoreBoard 조회 중 오류가 발생했습니다.' };
    }
  }

export default {
    getRoomInfo,
    getRoomList,
    getRoomPwd,
    getScoreBoardData
};