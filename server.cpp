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

#include <sys/select.h> 
#include <sys/stat.h> // mkdir

#define BUFFER_SIZE 1024
#define TIMEOUT 10

using namespace std;

bool makeDirectory(string path) {

    if (mkdir(path.c_str(), 0775) == -1) {
        switch(errno){
            case (ENOENT): {
              // parent didn't exist, try to create it
              if (makeDirectory(path.substr(0, path.find_last_of('/')))) {
                // try to create again
                return (mkdir(path.c_str(), 0775) == 0);
              }
              else {
                return false;
              }
            }
            case (EEXIST): {
              // path already exists
              return true;
            }
            default: {
              return false;
            }
        }
    }
    else {
      return true;
    } 
}

void receiveFile(int socket, FILE* file, string fileName) {
  bool isEnd = false;
  char buffer[BUFFER_SIZE] = {0};
  size_t dataSize;

  int selVal;
  struct timeval timeout;
  fd_set readfds;

  FD_ZERO(&readfds);
  FD_SET(socket, &readfds);

  timeout.tv_sec = TIMEOUT;
  timeout.tv_usec = 0;

  while (!isEnd) {
    selVal = select(socket + 1, &readfds, NULL, NULL, &timeout);
    if (selVal == -1) {
      cerr << "ERROR: select() failed" << endl; 
      return;
    }
    if (selVal == 0) {
      // Timeout
      string timeError = "ERROR";
      cerr << "ERROR: timeout while receiving data from client" << endl;
      fclose(file);
      file = fopen(fileName.c_str(), "wb");
      fwrite(timeError.c_str(), sizeof(char), timeError.size(), file);
      fclose(file);
      return;
    }
    // receive data
    memset(buffer, '\0', sizeof(buffer));
    if ((dataSize = recv(socket, buffer, BUFFER_SIZE, 0)) <= 0) {
      cout << "all data received\n";
      fclose(file);
      break;
    }
    // write data to file
    fwrite(&buffer, 1, dataSize, file);
  }
  cout << "all data written to /" << fileName << "\n";
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
  string path = string(argv[2]);
  if (path.front() == '/') {
    path.erase(path.begin());
  }
  cout << "directory: " << path << "\n";

  //string dirCommand = "sudo mkdir -p " + path;
  //system(dirCommand.c_str());

  if (makeDirectory(path) == false) {
      cerr << "ERROR: Failed to create directory: " << path << endl;
      return 20;
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
  int connectionID = 1;

  if (path.back() != '/') {
    path += '/';
  }
  string fileName = path + to_string(connectionID) + ".file";

  FILE *file = fopen(fileName.c_str(), "wb");
  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  cout << "Accepted a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << endl;

  // read/write data from/into the connection
  
  receiveFile(clientSockfd, file, fileName);

  close(clientSockfd);
  
  return 0;
}