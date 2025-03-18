import Redis from './connection_pool.js'
import DataBase_Util from '../Db/Utils.js'
import { promisify } from 'util';

// GET 명령어를 비동기로 실행할 수 있도록 promisify 사용
const getAsync = promisify(Redis.redis.get).bind(Redis.redis);
// SCAN 명령을 비동기로 실행할 수 있도록 promisify 사용
const scanAsync = promisify(Redis.redis.scan).bind(Redis.redis);
// XREAD 명령어를 비동기로 실행할 수 있도록 promisify 사용
const xreadAsync = promisify(Redis.redis.xread).bind(Redis.redis);

const delAsync = promisify(Redis.redis.del).bind(Redis.redis);

//룸 아이디를 이용한 채팅 저장
async function saveRoomChat(roomId) {
    try {
        //테이블 체크
        const TableName = `room_chat:${roomId}_log`
        if((await DataBase_Util.CheckTable(TableName)).status == false)
            if((await DataBase_Util.CreateTable(TableName)).status == false)
                return { status: false, message: 'DB 조회 및 생성 테이블 에러' };
        
        const streamName = `room_chat:${roomId}`;  // 예시: room_chat:1

        // XREAD로 스트림 데이터 읽기 (BLOCK은 무한 대기)
        const result = await xreadAsync('STREAMS', streamName, '0'); // '0'은 처음부터 읽기
        console.log(`Room ${roomId}의 채팅 스트림 데이터:`, result);
    
        // 스트림 데이터 처리
        result.forEach((stream) => {
          const [streamName, messages] = stream;
          messages.forEach((data) => {
            const [messageId, fields] = data;

            let user_id, nickname, message, timestamp;

            for (let i = 0; i < fields.length; i += 2) {
                const field = fields[i];
                const value = fields[i + 1];
                if (field === 'user_id') {
                    user_id = value;
                } else if (field === 'nickname') {
                    nickname = value;
                } else if (field === 'message') {
                    message = value;
                } else if (field === 'timestamp') {
                    timestamp = value;
                }
            }
            const columns = ['message_id', 'user_id', 'nickname', 'message', 'timestamp'];
            const values = [messageId, user_id, nickname, message, timestamp];
            console.log(`MessageId: ${messageId},User ID: ${user_id}, Nickname: ${nickname}, Message: ${message}, Timestamp: ${timestamp}`);
            DataBase_Util.InsertTable(TableName, columns, values);
          });
        });
        deleteRoomChat(roomId);
        return {status:true,message:"save room chat complete."}
    } catch (err) {
      return {status:false,message:'saveRoomChat 함수 실패'}
    }
  }

async function deleteRoomChat(roomId) {
    try {
        const streamName = `room_chat:${roomId}`; // 예: room_chat:1

        // DEL 명령어로 전체 삭제
        await delAsync(streamName);

        console.log(`Room ${roomId}의 채팅 스트림 데이터 전체 삭제 완료`);

        return { status: true, message: `Room ${roomId}의 채팅 데이터 전체 삭제 완료` };
    } catch (err) {
        return { status: false, message: 'deleteRoomChat 함수 실패', error: err };
    }
}



export default {
    saveRoomChat,
    deleteRoomChat
};