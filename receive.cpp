#include "head.hpp"
using namespace std;

#define MAXBUFFERSIZE 51209//51200+9B
#define BUFFERFULL receiveBufferEnd == receiveBufferStart
#define BUFFEREMPTY (receiveBufferEnd + 1) % 50 == receiveBufferStart



char receiveBuffer[50][MAXBUFFERSIZE];
int receiveBufferStart = 0, receiveBufferEnd = 49;

void Add(int& arg) {
    arg = (arg + 1) % 50;
}

void SendACK(SOCKET socket, SOCKADDR_IN addSocket, int seq,int ack) {
	char * buff = AddHead(seq, ack, 64, "");
	//send packet
	int len=sizeof(SOCKADDR);
	sendto(sock,buff,9,0, (SOCKADDR*)&addSocket,len);
}



void WritePackage(char* content, char* path) {
    FILE* target = fopen(path, "ab");
    fwrite(content, strlen(content), sizeof(char), target);
}

void ProcessPackage(SOCKET socket, SOCKADDR_IN addSocket, int index, map<int, char*> window, char* path) {
    Head headTemp = getHead(receiveBuffer[index]);
    window.insert(pair<int, char*>(headTemp.seq, receiveBuffer[index]));
    auto iter = window.begin();
    //TODO:修改！
    while (iter != window.end() && iter->first + 1 == (++iter)->first) {
        iter--;
        WritePackage(iter->second, path);
        iter = window.erase(iter);
    }
    int numToAck = window.begin()->first;
    SendACK(socket, addSocket, numToAck + 1);
}

void ListenAndReceive(SOCKET socket, SOCKADDR_IN addSocket, char* path, int packageCount) {
    int len = sizeof(SOCKADDR);
    char tempBuffer[MAXBUFFERSIZE];
    memset(tempBuffer, 0, MAXBUFFERSIZE * sizeof(char));
    map<int, char*> window;
    while(1) {
        if (BUFFERFULL) continue;
        int receiveByte = recvfrom(socket, receiveBuffer[receiveBufferStart], MAXBUFFERSIZE, 0, (SOCKADDR*)&addSocket, &(len));
        ProcessPackage(socket, addSocket, receiveBufferStart, window, path);
        Add(receiveBufferStart);
        if (window.size() == 1 && window.end()->first == packageCount) break;
    }
    WritePackage(window.begin()->second, path);
}
