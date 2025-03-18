import Database  from './connection_pool.js';
import bcrypt from 'bcrypt';

async function RegisterUser(user_id,password,nickname)
{
    //user_id가 이미 존재하는지 확인
    const checkResult = await CheckUserId(user_id);
    console.log(checkResult.message);
    if (!checkResult.status)
        return { status: false, message: checkResult.message };    

    try {
        const hashedPassword = await bcrypt.hash(password,10);

        const query = `
            INSERT INTO regular_users (user_id, password,nickname) 
            VALUES (?, ?,?)
        `;

        // 실행할 데이터
        const values = [user_id, hashedPassword,nickname];

        // 쿼리 실행
        const [result] = await Database.DBpool.execute(query, values);

        // 삽입된 결과 확인 (예: result.insertId는 삽입된 유저의 ID를 반환)
        if (result.affectedRows > 0) 
            return { status: true, message: '유저가 성공적으로 등록되었습니다.' };

    } catch (err) {
        console.error('DB 오류:', err);
        return { status: 'error', message: '유저 등록 중 오류가 발생했습니다.' };
    }
}

async function GetUserInfo(user_Id)
{
    try {
        const query = `
            SELECT user_id, password, nickname, email, birth, gender
            FROM regular_users 
            WHERE user_id = ?
        `;

        // 실행할 데이터 (values 배열로 전달)
        const values = [user_Id];

        // 쿼리 실행
        const [rows, fields] = await Database.DBpool.execute(query, values);

        if (rows.length > 0) {
            return { status: true, data: rows[0] };
        } else {
            return { status: false, message: '해당 id로 사용자를 찾을 수 없습니다.' };
        }
    } catch (err) {
        console.log('err message : ', err);
        return { status: 'error', message: 'DB 조회 중 오류가 발생했습니다.' };
    }
}

//ID 중복검사
async function CheckUserId(user_Id)
{
    try {
      // user_id가 이미 존재하는지 확인

    const values = [user_Id];
    const query = 'SELECT * FROM regular_users WHERE user_id = ?';
    const [rows] = await Database.DBpool.execute(query, values);
      
    // 결과가 있으면 아이디가 이미 존재한다는 의미
    if (rows.length > 0) {
            return { status: false, message: '이미 사용 중인 아이디입니다.' };
        } else {
            return { status: true, message: '사용 가능한 아이디입니다.' };
        }
    } catch (err) {
    console.error('DB 오류:', err);
    return { status: 'error', message: '아이디 확인 중 오류가 발생했습니다.' };
    }
      
}

async function LoginUser(user_Id, password) {

    const query = 'SELECT user_id,password,nickname FROM regular_users WHERE user_id = ?'

    const values = [user_Id];
    const [rows] = await Database.DBpool.execute(query,values);

    if(rows.length === 0)
    {
        return {status: false, message:'아이디가 존재 하지 않습니다.'};
    }

    const hashedPassword = rows[0].password;
    const isPasswordValid = await bcrypt.compare(password, hashedPassword);

    if(isPasswordValid){
        return {status:true,result:rows[0] ,message:'로그인 성공'};
    }else{
        return {status:false,message:'비밀번호가 틀렸습니다.'};
    }
}


export default {
    RegisterUser,
    GetUserInfo,
    CheckUserId,
    LoginUser
};