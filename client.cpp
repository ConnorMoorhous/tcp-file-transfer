#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

#define BUFFER_SIZE 1024

using namespace std;

int sendFile(int socket, string fileName) {
  char buffer[BUFFER_SIZE] = {0};
  FILE *file;

  if ((file = fopen(fileName.c_str(), "rb")) == NULL)
  {
    cerr << "ERROR: Failed to open file" << endl;
    return 17;
  }

  //size_t rret, wret;
  int bytes_read;

  while (!feof(file)) {
    memset(buffer, '\0', sizeof(buffer));

    if ((bytes_read = fread(&buffer, 1, BUFFER_SIZE, file)) > 0)
      send(socket, buffer, bytes_read, 0);
    else
      break;
  }
  
  if (fclose(file) == EOF) {
    cerr << "ERROR: Failed to close file" << endl;
    return 18;
  }

  return 0;
}

int main(int argc, char* argv[])
{
  // process host/ip argument
  string localhost = "127.0.0.1";
  string ip = string(argv[1]);
  cout << "ip address: " << ip << "\n";
  if (ip.compare("localhost") == 0) {
    ip.replace(0,9, localhost);
    cout << "new ip: " << ip << "\n";
  }

  // process port argument
  istringstream portArg(argv[2]);
  int port;
  portArg >> port;
  cout << "port: " << port << "\n";
  if (port < 1024) {
    cerr << "ERROR: Invalid port number" << endl;
    return 15;
  }

  // process file argument
  string fileName = string(argv[3]);
  cout << "file: " << fileName << "\n";

  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // struct sockaddr_in addr;
  // addr.sin_family = AF_INET;
  // addr.sin_port = htons(40001);     // short, network byte order
  // addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  // memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
  // if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
  //   perror("bind");
  //   return 1;
  // }

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);     // short, network byte order
  serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
  memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

  // connect to the server
  if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
    perror("connect");
    return 2;
  }

  // select timeout?

  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
    perror("getsockname");
    return 3;
  }

  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  cout << "Set up a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << endl;


  // send/receive data to/from connection
  int e;
  if ((e = sendFile(sockfd, fileName)) != 0) {
    return e;
  }

  close(sockfd);

  return 0;
}