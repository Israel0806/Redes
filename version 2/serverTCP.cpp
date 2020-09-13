/* Server code in C */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <thread>
#include <map>
#include <vector>

using namespace std;
map<int, string> socketList;
map<int, int> activeConn;

void socketRead(int);
void socketWrite(int, char *);

void cleanChar(char *var, int size)
{
  for (int i = 0; i < size; i++)
    var[i] = '\0';
}

int endOF = 1;
void socketRead(int p_socket)
{
  char msg[1000];
  do
  {
    read(p_socket, msg, 1000); // read server msg
    if (strcmp(msg, "chao") == 0)
    {
      endOF = 0;
      write(p_socket, "chao", 4);
    }
    socketWrite(p_socket, msg);
    printf("Client: [%s]\n", msg); // print server msg
    cleanChar(msg, 1000);
  } while (endOF != 0);
  shutdown(p_socket, SHUT_RDWR); // shutdown socket
  close(p_socket);               // close socket
}

void socketWrite(int p_socket, char *msg)
{
  string message = socketList[p_socket] + ": ";
  for (int i = 0;; ++i)
  {
    if (msg[i] == '\0')
      break;
    message += msg[i];
  }

  write(activeConn[p_socket], message.c_str(), message.length()); // write msg to server
                                                                  // write(socket.first, message.c_str(), message.length()); // write msg to server
}
// write(socket.first, "asd", 3); // write msg to server

void newConn(int socketClient)
{
  char msg[50];
  bool unique = false;

  /// set new nickname
  while (!unique)
  {
    unique = true;
    /// read nickname
    read(socketClient, msg, 50);
    cout << "Client message: " << msg << endl;
    for (auto &socket : socketList)
    {
      if (strcmp(socket.second.c_str(), msg) == 0)
      {
        unique = false;
        break;
      }
    }
    if (unique == false)
    {
      memset(msg, '\0', 50);
      cout.flush();
      write(socketClient, "code:nickname_used", 18);
    }
  }
  cout.flush();
  write(socketClient, "code:nickname_set", 17);
  // cout << "Nickname to be set: [" << msg << "]\n";
  socketList.insert({socketClient, msg});

  /// read dest nickname
  memset(msg, '\0', 50);
  read(socketClient, msg, 50);
  cout << "Client response: " << msg << endl;
  unique = false;

  /// find nickname
  while (!unique)
  {
    for (auto &socket : socketList)
    {
      if (strcmp(socket.second.c_str(), msg) == 0)
      {
        unique = true;
        break;
      }
    }
    if (unique == false)
    {
      cout.flush();
      memset(msg, '\0', 50);
      write(socketClient, "code:nickname_not_found", 23);
    }
  }
  for (auto &socket : socketList)
    if (strcmp(socket.second.c_str(), msg) == 0)
      activeConn.insert({socketClient, socket.first});
  cout.flush();
  memset(msg, '\0', 50);
  write(socketClient, "code:nickname_found", 19);

  thread(socketRead, socketClient).detach();
}

int main(void)
{
  struct sockaddr_in stSockAddr;
  int SocketServer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  char buffer[256];
  int n;

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(50001);
  stSockAddr.sin_addr.s_addr = INADDR_ANY;

  bind(SocketServer, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));
  // 10 is the size of the ...
  listen(SocketServer, 10);
  do
  {
    endOF = 1;
    int socketClient = 0;
    socketClient = accept(SocketServer, NULL, NULL);
    cout << "New connection" << endl;
    thread(newConn, socketClient).detach();
  } while (endOF != 0);
  printf("Shutting down server...\n");
  close(SocketServer);
  return 0;
}
