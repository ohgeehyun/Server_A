import createError from 'http-errors';
import express from 'express';
import path from 'path';
import cookieParser from 'cookie-parser';
import logger from 'morgan';
import './Utils/InitEnv.js'; 

import 'dotenv/config';  // .env 파일을 로드하기 위해 사용
import Database  from './Db/connection_pool.js';
import Redis from './Redis/connection_pool.js';
import Room from './Redis/Room.js';


import indexRouter from './routes/index.js';
import userRouter from './routes/user.js';
import roomRouter from './routes/room.js'
import chatRouter from './routes/chat.js'

const app = express();
app.use(express.json());

// view engine setup
app.set('views', path.join(process.cwd(), 'views'));  // process.cwd()로 현재 작업 디렉토리 경로를 사용
app.set('view engine', 'ejs');

app.use(logger('dev'));
app.use(express.json());
app.use(express.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(process.cwd(), 'public')));

app.use('/', indexRouter);
app.use('/user',userRouter);
app.use('/room',roomRouter);
app.use('/chat',chatRouter)

// catch 404 and forward to error handler
app.use((req, res, next) => {
  next(createError(404));
});

// error handler
app.use((err, req, res, next) => {
  // set locals, only providing error in development
  res.locals.message = err.message;
  res.locals.error = req.app.get('env') === 'development' ? err : {};

  // render the error page
  res.status(err.status || 500);
  res.render('error');
});

// 연결 테스트 (선택 사항) 필요없으면 삭제 또는 주석처리 서버 최초 실행시 DB 연결 시도
import mysql from 'mysql2/promise';
import dotenv from 'dotenv';

dotenv.config();

//DB연결
await Database.DBConnection(); 
//Redis테스트
await Redis.isRedisConnected();

export default app;