import fs from 'fs';
import crypto from 'crypto';

const generateSecretKey = () =>{
    //256비트(32바이트) 랜덤 비밀키 생성
    const secretKey = crypto.randomBytes(32).toString('hex');
    return secretKey;
}

const updateEnvFileSecretKey = (secretKey) =>{
    const envFilePath = './.env';

    if(!fs.existsSync(envFilePath)){
        //기존.env파일이없는경우 
        fs.writeFileSync(envFilePath, `JWT_SECRET_KEY=${secretKey}\n`);
        console.log('env파일 생성완료')
    }else{
        //.env파일이 있는경우
        const envFileContent = fs.readFileSync(envFilePath,'utf8');
        const newEnvFileContent = envFileContent.replace(
            /JWT_SECRET_KEY=[^\n]*/g,
            `JWT_SECRET_KEY=${secretKey}`
        );

        //파일 덮어쓰기
        fs.writeFileSync(envFilePath,newEnvFileContent);
        console.log('시크릿키 갱신 완료.');
    }
}


// 서버 시작 시 비밀키 생성 및 .env 파일 업데이트 env파일을 읽기전에 바꿀 것
const secretKey = generateSecretKey();
updateEnvFileSecretKey(secretKey);