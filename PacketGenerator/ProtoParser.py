import re

class ProtoParser():
    def __init__(self,start_id,recv_prefix,send_prefix):
        self.recv_pkt = [] # 수신 패킷 목록
        self.send_pkt = [] # 송신 패킷 목록
        self.total_pkt = [] # 모든 패킷 목록
        self.start_id = start_id
        self.id = start_id
        self.recv_prefix = recv_prefix
        self.send_prefix = send_prefix
        self.msgid_enum = {}  # MsgId 열거형 저장

    def parse_proto(self,path):
        f = open(path,'r',encoding = 'utf-8')
        lines = f.readlines() # lines에 읽은 파일의 정보를 저장

        enum_found = False
        current_enum = None

        for line in lines:
             line = line.strip()

                 # MsgId 열거형 탐지
             if line.startswith("enum MsgId"):
                enum_found = True
                current_enum = self.msgid_enum
                continue

             if enum_found:
                if line.startswith("}"):
                    enum_found = False
                    continue

                # 열거형 값 파싱 (형식: Name = Value;)
                match = re.match(r"([a-zA-Z0-9_]+)\s*=\s*(\d+);", line)
                if match:
                    key, value = match.groups()
                    current_enum[f"PKT_{key.upper()}"] = int(value)


        for line in lines:
            print(f"First few lines of proto file: {line}")  # 읽은 내용 확인
            if line.startswith('message') == False:
                continue


            pkt_name = line.split()[1].upper()
            print(f"Parsing packet: {pkt_name}")
            if pkt_name.startswith(self.recv_prefix):
                print(f"Parsing packet: {pkt_name}")
                print(f"Parsing packet: {self.id}")
                self.recv_pkt.append(Packet(pkt_name,self.id))
            elif pkt_name.startswith(self.send_prefix):
                print(f"Parsing packet: {pkt_name}")
                print(f"Parsing packet: {self.id}")
                self.send_pkt.append(Packet(pkt_name,self.id))
            else:
                continue

            self.total_pkt.append(Packet(pkt_name,self.id))
            self.id += 1

        f.close()



class Packet:
    def __init__(self, name ,id):
        self.name = name
        self.id = id