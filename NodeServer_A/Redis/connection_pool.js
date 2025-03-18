import Redis from 'ioredis';
import dotenv from 'dotenv';

dotenv.config();

const redis = new Redis({
    host: process.env.REDIS_HOST,
    port: process.env.REDIS_PORT,
    password: process.env.REDIS_PASSWORD,  // 비밀번호가 있는 경우
    db: process.env.REDIS_DB,              // 사용할 DB 번호 (기본값 0)
  });

// Redis 연결 테스트 함수
async function testRedisConnection() {
    try {
      const reply = await redis.ping();  // PING 명령어로 연결 테스트
      console.log('Redis 연결 테스트 성공:', reply);  // 'PONG'을 받으면 연결 성공
    } catch (err) {
      console.error('Redis 연결 테스트 실패:', err);
    }
  }
  
  // Redis 연결 확인 함수+                                                                                            
  async function isRedisConnected() {
    try {
      const reply = await redis.ping();  // PING 명령어로 연결 상태 확인
      console.log('Redis 연결 ping 확인 완료');
      return reply === 'PONG';           // 'PONG' 응답을 받으면 연결이 정상적임

    } catch (err) {
      console.error('Redis 연결 실패:', err);
      return false;
    }
  }
  
  export default {
    redis,
    testRedisConnection,
    isRedisConnected,
  };