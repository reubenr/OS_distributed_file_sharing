#include<thread>
#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<unistd.h>


#define SERVER_PORT_NUM 8100
#define CLIENT_PORT_NUM 8104
#define SERVER_IP "127.0.0.1"

void serverComm(int);
int readNumBytes(int, char*, int);
int writeNumBytes(int, char*, int);


int main(int argc, char **argv){
    int serverSocketFD;
    struct sockaddr_in servaddr, clientaddr;

    if ((serverSocketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        std::cout << "Socket error " << std::endl;
        exit(0);
    }

    memset(&clientaddr, 0, sizeof(clientaddr));
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientaddr.sin_port = htons(CLIENT_PORT_NUM);
    if(bind(serverSocketFD, (struct sockaddr *) &clientaddr, sizeof(clientaddr)) < 0){
        std::cout << "Client bind error " << std::endl;
        exit(0);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    servaddr.sin_port = htons(SERVER_PORT_NUM);
    if(connect(serverSocketFD, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
        std::cout << "Connect error " << std::endl;
        exit(0);
    }

    std::thread server(serverComm, serverSocketFD);
    server.join();
    return 0;
}

void serverComm(int serverSocketFD) {
    for(;;){
        short len = 0;
        if (readNumBytes(serverSocketFD, (char *) &len, sizeof(len)) < 0) {
            std::cout << "Client read error on len of greeting" << std::endl;
            exit(0);
        }

        len = ntohs(len);

        char serverGreeting[len + 1];

        if (readNumBytes(serverSocketFD, serverGreeting, len) < 0) {
            std::cout << "Client read error greeting" << std::endl;
            exit(0);
        }

        std::cout << serverGreeting << std::endl;

        serverGreeting[len] = '\0';
        std::string inputWord = "";
        std::cin >> inputWord;
        std::cin.clear();
        std::cin.ignore();

        short inputLen = (short) inputWord.length();
        char inputWordCStr[inputLen + 1];
        for (int i = 0; i < inputLen; ++i) {
            inputWordCStr[i] = inputWord[i];
        }

        inputWordCStr[inputLen] = '\0';

        short netInputLen = htons(inputLen);

        if (writeNumBytes(serverSocketFD, (char *) &netInputLen, sizeof(netInputLen)) < 0) {
            std::cout << "Client write error word " << std::endl;
            exit(0);
        }

        if (writeNumBytes(serverSocketFD, inputWordCStr, inputLen) < 0) {
            std::cout << "Client write error word " << std::endl;
            exit(0);
        }

        short lenReturnMessage = 0;
        if (readNumBytes(serverSocketFD, (char *) &lenReturnMessage, sizeof(lenReturnMessage)) < 0) {
            std::cout << "Client read error on len of return" << std::endl;
            exit(0);
        }

        lenReturnMessage = ntohs(lenReturnMessage);

        char serverReturn[lenReturnMessage + 1];
        if (readNumBytes(serverSocketFD, serverReturn, lenReturnMessage) < 0) {
            std::cout << "Client read error return" << std::endl;
            exit(0);
        }

        serverReturn[lenReturnMessage] = '\0';
        std::cout << serverReturn << std::endl;
        if(inputWord == "exit"){
            break;
        }
    }
    close(serverSocketFD);
}


int readNumBytes(int socketFileDescriptor, char *buffer, int numBytes){
    int num_left,num_read;
    num_left = numBytes;
    while (num_left > 0) {
        num_read = read(socketFileDescriptor, buffer, num_left);
        if(num_read <0)  return(num_read);
        if (num_read == 0) break;
        num_left -= num_read;
        buffer += num_read;
    }
    return(numBytes - num_left);
}

int writeNumBytes(int socketFileDescriptor, char *buffer, int numBytes){
    int num_left, num_written;
    num_left = numBytes;
    while(num_left > 0){
        num_written = write(socketFileDescriptor, buffer, numBytes);
        if(num_written <= 0) return(num_written);
        num_left -= num_written;
        buffer += num_written;
    }
    return (numBytes - num_left);
}
