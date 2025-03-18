import mysql from 'mysql2/promise';
import dotenv from 'dotenv';

dotenv.config();

const DBpool = mysql.createPool({
    host: process.env.DB_HOST,
    port: process.env.DB_PORT,
    user: process.env.DB_USER,
    password: process.env.DB_PASSWORD,
    database: process.env.DB_NAME
  });

async function testDBConnection () {
    try {
      const connection = await DBpool.getConnection();
      console.log('DB 연결 성공');
      connection.release();  // 연결을 사용한 후 반환
    } catch (err) {
      console.error('DB 연결 실패:', err);
      process.exit(1); // 서버 종료
    }
  }

  async function DBConnection () {
    try {
      const connection = await DBpool.getConnection();
      console.log('DB 연결 성공');
    } catch (err) {
      console.error('DB 연결 실패:', err);
      process.exit(1); // 서버 종료
    }
  }

  async function isConnection()
  {
    try{
        await DBpool.execute('SELECT 1');
        return true;
    }
    catch (err){
        console.error('DB 연결 실패',err);
        return false;
    }
  }


export default {
    DBpool,
    testDBConnection,
    DBConnection,
    isConnection
};