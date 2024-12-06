
테스트 영상 링크
https://youtu.be/ouMp5Gto1_s

- 프로젝트 소개 -
c++ iocp 기반의 ServerCore 부분 과 실질적으로 unity와 통신하는 GameServer 부분 으로 나뉘어진 서버
패킷자동화 관련 부분은 Google ProtoBuffer 와 python3.9 부분을 사용하여 패킷 자동화 부분 적용

주요 Directory 설명 

Common : 공용 파일들을 저장하여 사용 Google Buffer를 사용한 패킷 자동화 및 클라이언트 와 같이 사용하게 될 GridMap에 대한 정보가 담겨있다.

Libraries : 외부 라이브러리(GoogleProtoBufFer) 및 ServerCore 라이브러리를 정적라이브러리로 담아 둔 폴더.

PacketGenerator : Protobuf에 따라 사용할 패킷이 늘어남에 따라 packethandler에서 패킷만드는 작업을 자동화 해주는 python 파일 폴더

ServerCore : iocp socket통신을 하기 위한 뼈대 라이브러리들을 작성한 폴더 

Server_A : 실질적으로 클라이언트와 통신을할 서버의 파일 폴더


