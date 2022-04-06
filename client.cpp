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

#include <fcntl.h>
#include <sys/select.h>

#define BUFFER_SIZE 1024
#define TIMEOUT 10

using namespace std;

void connectServer(int socket, int port, string ip) {

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);     // short, network byte order
  serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
  memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

  // set socket to nonblocking
  int flags = fcntl(socket, F_GETFL);
  flags = flags + O_NONBLOCK;
  fcntl(socket, F_SETFL, flags);

  // connect to server
  if (connect(socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
    
    if (errno != EINPROGRESS) {
      perror("connect");
      exit(3);
    }

    struct timeval timeout;
    fd_set writefds;
    fd_set extendfds;

    FD_ZERO(&writefds);
    FD_SET(socket, &writefds);
    FD_ZERO(&extendfds);
    FD_SET(socket, &extendfds);
    
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;

    int selVal = select(socket + 1, NULL, &writefds, &extendfds, &timeout);
    if (selVal == -1) {
      cerr << "ERROR: select() failed" << endl; 
      exit(4);
    }
    if (selVal == 0) {
      // timeout
      close(socket);
      cerr << "ERROR: timeout while trying to connect to server" << endl;
      exit(5);
    } 

    // revert socket back to blocking
    flags = flags - O_NONBLOCK;
    fcntl(socket, F_SETFL, flags);
  }

  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  if (getsockname(socket, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
    perror("getsockname");
    exit(6);
  }

  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  cout << "Set up a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << endl;

  return;
}

void sendFile(int socket, string fileName) {

  char buffer[BUFFER_SIZE] = {0};
  int bytesRead;
  FILE *file;

  // open file for reading
  if ((file = fopen(fileName.c_str(), "r")) == NULL)
  {
    cerr << "ERROR: failed to open " << fileName << endl;
    exit(7);
  }

  int selVal;
  struct timeval timeout;
  fd_set writefds;
    
  timeout.tv_sec = TIMEOUT;
  timeout.tv_usec = 0;
  
  // send file over socket
  while (!feof(file)) {

    FD_ZERO(&writefds);
    FD_SET(socket, &writefds);
  
    selVal = select(socket + 1, NULL, &writefds, NULL, &timeout);
    if (selVal == -1) {
      cerr << "ERROR: select() failed" << endl; 
      exit(8);
    }
    if (selVal == 0) {
      // timeout
      close(socket);
      cerr << "ERROR: timeout while trying to send data to server" << endl;
      exit(9);
    } 

    memset(buffer, '\0', sizeof(buffer));

    if ((bytesRead = fread(&buffer, 1, BUFFER_SIZE, file)) > 0) {
      send(socket, buffer, bytesRead, 0);
    }
    else {
      break;
    }
  }
  
  // close file
  if (fclose(file) == EOF) {
    cerr << "ERROR: failed to close " << fileName << endl;
    exit(10);
  }

  return;
}

bool validateIP(string ip) {

  size_t pos = 0;
  int n = 0;
  bool isDigit;
  string token;

  while ((pos = ip.find(".")) != string::npos && n < 4) {
    
    if (pos == 0) {
      return false;
    }

    token = ip.substr(0, pos);
    cout << "token" << n << ": " << token << "\n";
    isDigit = !token.empty() && (token.find_first_not_of("[0123456789]") == string::npos);
    
    if (!isDigit || stoi(token) < 0 || stoi(token) > 255) {
      return false;
    }

    ip.erase(0, pos + 1);
    n++;
  }

  if (n != 3) {
    return false;
  }

  token = ip.substr(0, pos);
  isDigit = !token.empty() && (token.find_first_not_of("[0123456789]") == string::npos);
  
  if (!isDigit || stoi(token) < 0 || stoi(token) > 255) {
      return false;
  }

  return true;
}

int main(int argc, char* argv[])
{
  // process hostname/ip argument
  string localhost = "127.0.0.1";
  string ip = string(argv[1]);
  cout << "ip address: " << ip << "\n";

  if (ip.compare("localhost") == 0) {
    ip.replace(0, localhost.length(), localhost);
  }
  else if (!validateIP(ip)) {
    cerr << "ERROR: invalid ip address" << endl;
    return 1;
  }

  // process port argument
  istringstream portArg(argv[2]);
  int port;
  portArg >> port;
  cout << "port: " << port << "\n";
  if (port < 1024) {
    cerr << "ERROR: invalid port number" << endl;
    return 2;
  }

  // process file argument
  string fileName = string(argv[3]);
  cout << "file: " << fileName << "\n";

  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // connect to the server
  connectServer(sockfd, port, ip);

  // send file to the server
  sendFile(sockfd, fileName);

  // abort connection
  close(sockfd);

  return 0;
}