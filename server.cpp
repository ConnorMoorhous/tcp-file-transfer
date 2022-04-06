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

#include <fstream>
#include <signal.h>
#include <sys/select.h> 

#define BUFFER_SIZE 1024
#define TIMEOUT 10
#define MAX_CONNECTIONS 10
#define UNLIMITED_CONNECTIONS false

using namespace std;

void receiveFile(int socket, string filePath) {

  bool isEnd = false;
  char buffer[BUFFER_SIZE] = {0};
  size_t dataSize;

  // open file for writing
  ofstream file;
  file.open(filePath, ios::out | ios::trunc);

  int selVal;
  struct timeval timeout;
  fd_set readfds;

  timeout.tv_sec = TIMEOUT;
  timeout.tv_usec = 0;

  while (!isEnd) {

    FD_ZERO(&readfds);
    FD_SET(socket, &readfds);

    selVal = select(socket + 1, &readfds, NULL, NULL, &timeout);
    if (selVal == -1) {
      cerr << "ERROR: select() failed" << endl; 
      return;
    }
    if (selVal == 0) {
      // Timeout
      string timeError = "ERROR";
      cerr << "ERROR: timeout while receiving data from client" << endl;
      file.close();
      file.open(filePath, ios::out | ios::trunc);
      file.write(timeError.c_str(), timeError.size());
      file.close();
      return;
    }
    // receive data
    memset(buffer, '\0', sizeof(buffer));
    if ((dataSize = recv(socket, buffer, BUFFER_SIZE, 0)) <= 0) {
      file.close();
      return;
    }
    // write data to file
    file.write(buffer, dataSize);
  }
} 

void signalHandler(int sigNum) {
  if (sigNum == SIGTERM || sigNum == SIGQUIT) {
    exit(0);
  }
  return;
}

int main(int argc, char* argv[])
{
  signal(SIGTERM, signalHandler); 
  signal(SIGQUIT, signalHandler); 

  // process port argument
  istringstream portArg(argv[1]);
  int port;
  portArg >> port;
  cout << "port: " << port << "\n";
  if (port < 1024) {
    cerr << "ERROR: invalid port number" << endl;
    return 15;
  }

  // process directory argument
  string path = string(argv[2]);

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

  int connectionID = 0;

  while (connectionID++ < MAX_CONNECTIONS || UNLIMITED_CONNECTIONS) {
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
    cout << "Accepted a connection from: " << ipstr << ":" <<
      ntohs(clientAddr.sin_port) << endl;

    // read/write data from the connection
    string filePath = path + to_string(connectionID) + ".file";
    receiveFile(clientSockfd, filePath);

    // abort connection
    close(clientSockfd);
  }
  
  // close socket
  close(sockfd);

  return 0;
}