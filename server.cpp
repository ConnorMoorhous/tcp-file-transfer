#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

#define BUFFER_SIZE 1024

using namespace std;

void receiveFile(int socket, string dir, int fileNum) {
  bool isEnd = false;
  char buffer[BUFFER_SIZE] = {0};
  size_t dataSize;
  string fileName = dir + "/" + to_string(fileNum) + ".file";

  FILE *file = fopen(fileName.c_str(), "wb");

  while (!isEnd) {
    memset(buffer, '\0', sizeof(buffer));
    if ((dataSize = recv(socket, buffer, BUFFER_SIZE, 0)) <= 0) {
        break;
    }
    fwrite(&buffer, 1, dataSize, file);
  }
  fclose(file);
  //cout << "File received\n";
} 

int main(int argc, char* argv[])
{
  // process port argument
  istringstream portArg(argv[1]);
  int port;
  portArg >> port;
  cout << "port: " << port << "\n";
  if (port < 1024) {
    cerr << "ERROR: Invalid port number" << endl;
    return 15;
  }

  // process directory argument
  string dir = string(argv[2]);
  dir.erase(dir.begin());
  cout << "directory: " << dir << "\n";

  if (mkdir(dir.c_str(), 0777) == -1) {
      // cerr << "ERROR: Failed to create directory" << endl;
    }

  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // allow others to reuse the address
  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    return 1;
  }

  // bind address to socket
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);     // short, network byte order
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

  if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind");
    return 2;
  }

  // set socket to listen status
  if (listen(sockfd, 1) == -1) {
    perror("listen");
    return 3;
  }

  // accept a new connection
  struct sockaddr_in clientAddr;
  socklen_t clientAddrSize = sizeof(clientAddr);
  int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

  if (clientSockfd == -1) {
    perror("accept");
    return 4;
  }

  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  cout << "Accept a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << endl;

  // read/write data from/into the connection
  
  //create file first, once connection is accepted?
  receiveFile(clientSockfd, dir, 1);

  close(clientSockfd);
  
  return 0;
}