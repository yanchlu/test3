#include <WinSock2.h>
#include <stdio.h>    
#include <stdlib.h>    
#include <string.h> 
#include <time.h>
#include <vector>
#pragma comment(lib,"ws2_32.lib")
#define SERV_PORT 3000    
#define MAX_DATA 51200  //50kb    
#define FILE_LENGTH 921654  
#define BUFFER_SIZE 51200  
#include <iostream>
using namespace std; 

int get_seq(char* receBuf); 
int get_ack(char* receBuf); 
void sendPacketforHandShake(SOCKET sock, SOCKADDR_IN addrR);
int ack = 0;
int seq = 0;

char* AddHead(int seq, int ack, int mode, char* buffer) {
	int l = strlen(buffer);
    char* headedPackage = new char[l + 9];
	headedPackage[0] = seq / (16*16*16) + 1;
	headedPackage[1] = seq / (16*16) % 16 + 1; 
	headedPackage[2] = seq / 16 % 16 + 1; 
	headedPackage[3] = seq % 16 + 1; 
	headedPackage[4] = ack / (16*16*16) + 1;
	headedPackage[5] = ack / (16*16) % 16 + 1; 
	headedPackage[6] = ack / 16 % 16 + 1; 
	headedPackage[7] = ack % 16 + 1;  
	headedPackage[8] = mode;
	for(int i = 0;i < l;i++){
		headedPackage[i + 9] = buffer[i];
	}
    //TODO: 加上头部分
    return headedPackage;
}
char* ThrowHead(char* buffer) {
	int l = strlen(buffer);
	cout << l << endl;
    char* headedPackage = new char[l - 9];
	for(int i = 0;i < l;i++){
		headedPackage[i] = buffer[i + 9];
	}
    //TODO: 加上头部分
    return headedPackage;
}
struct Client{
	string ip;
	int id;
	int port;
	int ack;
	int seq;
};
vector<Client> Vistors;

int main(){    
	//Initialize
	WORD ver;
	WSADATA WSAData;
	ver=MAKEWORD(1,1);
	if(WSAStartup(ver,&WSAData)){
		printf("fail to initialize");
		getchar();
		return 0;
	}
 	if ( LOBYTE( WSAData.wVersion ) != 1 ||HIBYTE( WSAData.wVersion ) != 1 ) {
		printf("adsf3");
		getchar();
		WSACleanup(); 
		return 0;
	}
	SOCKET sock=socket(AF_INET,SOCK_DGRAM,0);
	SOCKADDR_IN addrR;
	addrR.sin_addr.s_addr=htonl(INADDR_ANY);
	addrR.sin_family=AF_INET;
	addrR.sin_port=htons(9988);
	if(SOCKET_ERROR==bind(sock,(SOCKADDR*)&addrR,sizeof(SOCKADDR))){
		printf("adsf4");
		getchar();
		WSACleanup(); 
		return 0;
	}
	//Listening
	while(1) { 
		SOCKADDR_IN addrS;
		int len=sizeof(SOCKADDR);
		char receBuf[BUFFER_SIZE];
		int k = recvfrom(sock,receBuf,BUFFER_SIZE,0,(SOCKADDR*)&addrS,&len);
		if(k>0){  //get packet   
			int flags[8]={0};
			for(int i=7;i >= 0;i--){
				flags[7 - i]=(receBuf[8] & (0x01 << i))?1:0;  
			}
			string c_addr = inet_ntoa(addrS.sin_addr);
			SOCKADDR_IN addrC;
			const char *p = c_addr.c_str();
			addrC.sin_addr.s_addr = inet_addr(p);
			addrC.sin_family = AF_INET;
			addrC.sin_port = htons(8899);
			if(flags[0] == 1){		//recieved the first handshake	
				cout << "get1" << endl;
				//send the second hand
				int c_seq = get_seq(receBuf);
				ack = c_seq + 1;
				
				sendPacketforHandShake(sock, addrC);			
			}
			else if(flags[1] == 1){  //recieved the third shake
				cout << "get2" << endl;
				int c_seq = get_seq(receBuf);
				int c_ack = get_ack(receBuf);
				if(c_ack != seq + 1){
					cout << "ack error";
				}
			}
			else if(flags[1] == 1 && flags[3]==0){  //recieved the third shake
				cout << "get2" << endl;
				int c_seq = get_seq(receBuf);
				int c_ack = get_ack(receBuf);
				if(c_ack != seq + 1){
					cout << "ack error";
				}
			}
			else if(flags[1] == 1 && flags[3] == 1){
				if(flags[4]==1){
					cout << "client upload file" << endl;
					int c_seq = get_seq(receBuf);
					int c_ack = get_ack(receBuf);
					if(c_ack != seq + 1){
						cout << "ack error";
					}
					ack = c_seq + 1;
					seq = seq + 1;
					char* buffer = AddHead(seq,ack,64,"");
					sendto(sock,buffer,9,0, (SOCKADDR*)&addrC,len);
					//SendACK 
				}
				else{
					cout << "client download file" << endl;
				}
			}
      	}   
		else {
			fprintf(stderr, "recvfrom error!\n");
    		exit(1);
		}
	}		
    closesocket(sock);
    WSACleanup(); 
	getchar();
    return 0;    
}    

int get_seq(char* receBuf){
	int s = 0;;
	for(int i = 0;i < 4;i++){
		int result[8]={0};
		for(int j=0;j<8;j++){
			result[7 - j]=(receBuf[i] & (0x01 << j))?1:0;  
		}
		for(int j=0;j<8;j++){
			s *= 2;
			s += result[j];			
		}
		s -= 1;
	}
	return s;
}
int get_ack(char* receBuf){
	int a = 0;;
	for(int i = 4;i < 8;i++){
		int result[8]={0};
		for(int j=0;j<8;j++){
			result[7 - j]=(receBuf[i] & (0x01 << j))?1:0;  
		}
		for(int j=0;j<8;j++){
			a *= 2;
			a += result[j];			
		}
		a -= 1;
	}
	return a;
}

void sendPacketforHandShake(SOCKET sock, SOCKADDR_IN addrC){
	char* headBuf = AddHead(seq,ack,192,"");;
	//send packet
	int len=sizeof(SOCKADDR);
	sendto(sock,headBuf,9,0, (SOCKADDR*)&addrC,len);
}

