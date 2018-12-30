#include "head.hpp"
using namespace std;

#define MAXBUFFERSIZE 51200
#define BUFFERFULL sendBuffer.size() >= maxWindowSize
#define BUFFEREMPTY sendBuffer.size() == 0

map<int, char*> sendBuffer;
map<int, char*> waitBuffer;
int maxWindowSize = 50;


bool SendAndPutInBuffer(char* buffer, int seq, SOCKET socket, SOCKADDR_IN addSocket) {
    if (BUFFERFULL) {
        printf(">Full buffer, failed to read into the buffer.\n");
        return 0;
    }
    sendBuffer.insert(pair<int, char*>(seq, buffer));
    char* finalPackage = AddHead(seq, buffer, 0);
    sendto(socket, finalPackage, strlen(finalPackage), 0, (SOCKADDR*)&addSocket, sizeof(SOCKADDR));
    return 1;
}

char* AddHead(int seq, char* buffer, int mode) {
    char* headedPackage = new char[strlen(buffer) + 9];
    //TODO: 加上头部分
    return headedPackage;
}

void ListenToACK(SOCKET socketListen, SOCKADDR_IN addSocketListen, SOCKET socketSend, SOCKADDR_IN addSocketSend) {
    char* bufferTemp = new char[MAXBUFFERSIZE];
    int len = sizeof(SOCKADDR);
    while (1) {
        int receiveByte = recvfrom(socketListen, bufferTemp, MAXBUFFERSIZE, 0, (SOCKADDR*)&addSocketListen, &(len));
        if (receiveByte == 0) continue;
        Head headReceive = getHead(bufferTemp);
        auto iter = waitBuffer.begin();
        while (iter->first < headReceive.ack) {
            iter = waitBuffer.erase(iter);
        }
        if (iter->first == headReceive.ack) {
            char* finalPackage = AddHead(iter->first, iter->second, 0);
            sendto(socketSend, finalPackage, strlen(finalPackage), 0, (SOCKADDR*)&addSocketSend, sizeof(SOCKADDR));
        }
    }
}

void SendFile(const char* path, SOCKET socket, SOCKADDR_IN addSocket) {
    FILE* target = fopen(path, "rb");
    if (target == NULL) {
        printf(">The file %s doesn't exist.\n", path);
        fclose(target);
        return;
    }

    FILE* tempFilePtr = target;
    fseek(tempFilePtr, 0, SEEK_END);
    int byteTotal = ftell(tempFilePtr);
    printf(">The size of %s is %d bytes.\n", path, byteTotal);

    char* tempBuffer = new char[MAXBUFFERSIZE];
    int byteSent = 0, packageCount = 0;

    printf(">Start reading file to the buffer.\n");
    int seq = 10;
    while(1) {
        int byteRead = fread(tempBuffer, sizeof(unsigned char), MAXBUFFERSIZE, target);
        if (byteRead == 0) {
            printf(">Have read all the content to be sent.\n");
            break;
        }
        while (1) {
            if (SendAndPutInBuffer(tempBuffer, seq, socket, addSocket) == 1) break;
        }
        seq++;
        byteSent += byteRead;
        packageCount++;
        printf("Have send %d packages (%d bytes) to buffer.\n", packageCount, byteSent);
    }
}