#include<thread>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/time.h>
#include<iostream>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<time.h>

#define PORT_NUMBER 8100
#define LISTENQ 10
#define MAXLINE 1024

void clientHandler(int);
int readNumBytes(int, char*, int);
int writeNumBytes(int, char*, int);


int main(int argc, char **argv){
    int listenfd, connfd;
    socklen_t len;
    struct sockaddr_in serveraddr, clientaddr;

    if ((listenfd = socket(AF_INET,SOCK_STREAM ,0)) < 0){
        std::cout << "socket failed ";
        exit(0);
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr .sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(PORT_NUMBER);

    if(bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0){
        std::cout << "Bind failed. ";
        exit(0);
    }

    if(listen(listenfd, LISTENQ) < 0){
        std::cout << "Listen failed. ";
        exit(0);
    }

    for(;;){
        len = sizeof(clientaddr);
        if((connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &len)) < 0){
            std::cout << "Accept failed. ";
            exit(0);
        }

        std::thread(clientHandler, connfd).detach();

    }

}


void clientHandler(int connection){
    char greeting[] = "Send a word.";
    short len = (short)strlen(greeting);
    short netLen = htons(strlen(greeting));
    if(writeNumBytes(connection, (char*)&netLen, sizeof(netLen)) < 0){
        std::cout << "Server write error on length of greeting ";
        exit(0);
    }
    if(writeNumBytes(connection, greeting, len) < 0){
        std::cout << "Server write error on greeting ";
        exit(0);
    }
    short lenClientMsg = 0;
    if(readNumBytes(connection, (char*) &lenClientMsg, sizeof(lenClientMsg)) < 0){
        std::cout << "Server read error on lenClientMsg ";
        exit(0);
    }


    lenClientMsg = ntohs(lenClientMsg);


    //std::cout << "length client message: " << lenClientMsg << std::endl;


    char clientMsg[lenClientMsg + 1];
    if(readNumBytes(connection, clientMsg, lenClientMsg) < 0){
        std::cout << "Server read error on clientMsg ";
        exit(0);
    }

    //std::cout << "client message: " << clientMsg << std::endl;

    clientMsg[lenClientMsg] = '\n';
    for (int i = 0; i < strlen(clientMsg); ++i){
        clientMsg[i] = (char)toupper(clientMsg[i]);
    }
    short netLenClientMsg = htons(lenClientMsg);
    if(writeNumBytes(connection, (char*)&netLenClientMsg, sizeof(netLenClientMsg)) < 0){
        std::cout << "Server write error on length of message back ";
        exit(0);
    }

    if(writeNumBytes(connection, clientMsg, lenClientMsg) < 0){
        std::cout << "Server write error on message back ";
        exit(0);
    }
    close(connection);
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
