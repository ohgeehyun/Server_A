import Database  from './connection_pool.js';

//테이블 검사
async function CheckTable(TableName)
{
    try {
        const values = [TableName];
        const query = `
        SELECT COUNT(*)
        FROM INFORMATION_SCHEMA.TABLES
        WHERE TABLE_SCHEMA = 'GAME_A' AND TABLE_NAME = ?;`;
        const [rows] = await Database.DBpool.execute(query, values);

        //0이상이면 테이블이 존재
        if (rows[0]['COUNT(*)'] > 0) {
            return { status: true, message: `테이블 ${tableName}이 존재합니다.` };
        } else {
            return { status: false, message: `테이블 ${tableName}이 존재하지 않습니다.` };
        }

        } catch (err) {
            return { status: false, message: 'table 검사중 오류 발생.' };
        }      
}


//테이블 생성
async function CreateTable(TableName)
{
    try {
        const values = [TableName];
        const createQuery = `
          CREATE TABLE  IF NOT EXISTS \`${TableName}\` (
                    id INT AUTO_INCREMENT PRIMARY KEY,
                    message_id VARCHAR(255) NOT NULL,
                    user_id VARCHAR(255) NOT NULL,
                    nickname VARCHAR(255) NOT NULL,
                    message TEXT NOT NULL,
                    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                );`;
                
           // 테이블 생성 실행
           const result = await Database.DBpool.execute(createQuery);
           return { status: true, message: `테이블 ${TableName}이 생성되었습니다.` };

        } catch (err) {
            return { status: 'error', message: 'table 생성 중 오류 발생.' };
        }      
}

//테이블에 값 추가
async function InsertTable(TableName, columns, values) {
    try {
        // columns가 배열이고, values가 배열인지 확인
        if (!Array.isArray(columns) || !Array.isArray(values)) {
            return { status: 'error', message: 'columns 또는 values는 배열이어야 합니다.' };
        }

        // 컬럼 수와 value의 수는 같아야함
        if (columns.length !== values.length) {
            return { status: 'error', message: 'columns와 values의 길이가 일치해야 합니다.' };
        }

        const columnsStr = columns.join(', ');
        const placeholders = columns.map(() => '?').join(', ');

        const query = `
            INSERT INTO \`${TableName}\` (${columnsStr})
            VALUES (${placeholders});
        `;

        // 데이터베이스에 쿼리 실행
        const result = await Database.DBpool.execute(query, values); // 비동기 쿼리 실행

        console.log("insert value")
        console.log(result)
        // 성공적으로 데이터 삽입된 경우
        return { status: true, message: `테이블 ${TableName}에 데이터 삽입 성공` };

    } catch (err) {
        console.log(err)
        return { status: false, message: 'table Insert 중 오류 발생.' };
    }
}




export default {
    CheckTable,
    CreateTable,
    InsertTable
};