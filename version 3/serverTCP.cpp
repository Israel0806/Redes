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
#include <windows.h>

using namespace std;
map<int, string> socketList;

void socketRead(int);
void socketWrite(int, char *);

void cleanChar(char *var, int size)
{
  for (int i = 0; i < size; i++)
    var[i] = '\0';
}

int endOF = 1;

void setNickname(int socketClient)
{
  char msg[100];
  char nickname[100];
  read(socketClient, msg, 2); // read size of nickname
  msg[2] = '\0';
  int nicknameSize = atoi(msg);
  read(socketClient, nickname, nicknameSize); // read nickname
  socketList.insert({socketClient, nickname});
  write(socketClient, "S", 1);
}

void sendMessage(int socketClient)
{
  char msg[1000];
  char nickname[100];
  read(socketClient, msg, 2); // read size of nicknmae
  msg[2] = '\0';
  int nicknameSize = atoi(msg);
  read(socketClient, nickname, nicknameSize); // read nickname
  nickname[nicknameSize] = '\0';
  int socketDest = -1;
  for (auto &socket : socketList)
  {
    // cout << socket.second << " == " << nickname << endl;
    if (strcmp(socket.second.c_str(), nickname) == 0)
    {
      cout << "Match!" << endl;
      socketDest = socket.first;
      break;
    }
  }
  read(socketClient, msg, 3); // read size of message
  msg[3] = '\0';
  int msgSize = atoi(msg);
  read(socketClient, msg, msgSize); // read message
  msg[msgSize + 1] = '\0';
  write(socketDest, "M", 1);                             // write Code
  write(socketDest, to_string(nicknameSize).c_str(), 2); // write sizeofNickname
  write(socketDest, nickname, nicknameSize);             // write Nickname
  write(socketDest, to_string(msgSize).c_str(), 3);      // write Size of message
  write(socketDest, msg, msgSize);                       // write message
  cout << "Message sent to: " << nickname << endl;
}

void sendClientList(int socketClient)
{
  write(socketClient, "L", 1);                                      // code
  write(socketClient, to_string(socketList.size() - 1).c_str(), 2); // #Clients
  for (auto &socket : socketList)
  {
    if (socket.first != socketClient)
    {
      // cout << "Nickname sent: " << socket.second << "\n ";
      int sizeNickname = socket.second.length();
      // cout << "Size: " << sizeNickname << endl;
      if (sizeNickname < 10)
        write(socketClient, to_string(sizeNickname).c_str(), 2); // size of nickname
      else
        write(socketClient, '0' + to_string(sizeNickname).c_str(), 2); // size of nickname
      write(socketClient, socket.second.c_str(), sizeNickname);        // nickname
    }
  }
  cout << "List of clients send\n";
}

void sendMessageToAll(int socketClient)
{
  char msg[1000];
  read(socketClient, msg, 3); // read size of message
  msg[3] = '\0';
  int msgSize = atoi(msg);
  read(socketClient, msg, msgSize); // read message
  for (auto &socket : socketList)
  {
    if (socket.first != socketClient)
    {
      int nicknameSize = socketList[socketClient].length();
      write(socket.first, "A", 1);                                         // code
      write(socket.first, to_string(nicknameSize).c_str(), 2);             // size of nickname
      write(socket.first, socketList[socketClient].c_str(), nicknameSize); // nickname
      write(socket.first, to_string(msgSize).c_str(), 3);                  // size of message
      write(socket.first, msg, msgSize);                                   // message
    }
  }
  cout << "Message sent to all clients\n";
}

string getDigits(string fileSize)
{
  int digits = 0;
  while (isdigit(fileSize[digits]))
    digits++;
  return to_string(digits);
}

void sendFile(int socketClient)
{
  /*
  size-of-nickname (2B)
nickname (V)
size-of-file-name(2B)
file-name (V)
size-of-file (4B)
file (V)*/
  char msg[1000];
  char file[10000];
  char nickname[100];
  char filename[100];
  read(socketClient, msg, 2); // read size of nicknmae
  msg[2] = '\0';
  int nicknameSize = atoi(msg);
  read(socketClient, nickname, nicknameSize); // read nickname
  nickname[nicknameSize] = '\0';
  read(socketClient, msg, 2); // read size of filename
  msg[2] = '\0';
  int filenameSize = atoi(msg);
  read(socketClient, filename, filenameSize); // read filename
  filename[filenameSize] = '\0';

  char digits[100];
  char digitsSize[100];
  read(socketClient, digitsSize, 100);          // read number of digits
  read(socketClient, digits, stoi(digitsSize)); // read size of file
  int fileSize = atoi(digits);
  // cout << "Digits Size: " << digitsSize << endl;
  // cout << "File size: " << fileSize << endl;
  // cout << "Filename: " << filename << endl;
  int n = read(socketClient, file, atoi(digits)); // read file
  // cout << "File size2: " << fileSize << endl;
  // cout << "Bytes received: " << n << endl;
  // cout << "File lenght: " << strlen(file) << endl;
  // file[fileSize] = '\0';
  for (auto &socket : socketList)
  {
    if (strcmp(socket.second.c_str(), nickname) == 0)
    {
      write(socket.first, "F", 1);                             // write code
      write(socket.first, to_string(nicknameSize).c_str(), 2); // write size of nickname
      write(socket.first, nickname, nicknameSize);             // write nickname
      write(socket.first, to_string(filenameSize).c_str(), 2); // write size of filename
      write(socket.first, filename, filenameSize);             // write filename
      write(socket.first, digitsSize, 100);
      write(socket.first, digits, stoi(digitsSize)); // write size of filename
      write(socket.first, file, fileSize);           // write filename
      cout << "File sent!\n";
      break;
    }
  }
}

void socketRead(int socketClient)
{
  char msg[1000];
  char nickname[100];
  // char command;
  do
  {
    read(socketClient, msg, 1);
    if (msg[0] == 'N')
      setNickname(socketClient);
    else if (msg[0] == 'M')
    {
      cout << "WTF\n";
      sendMessage(socketClient);
    }
    else if (msg[0] == 'Q')
    {
      socketList.erase(socketClient);
      write(socketClient, "Q", 1);
      break;
    } // quit
    else if (msg[0] == 'F')
      sendFile(socketClient);
    else if (msg[0] == 'L')
      sendClientList(socketClient);
    else if (msg[0] == 'A')
      sendMessageToAll(socketClient);
  } while (endOF != 0);
  shutdown(socketClient, SHUT_RDWR); // shutdown socket
  close(socketClient);               // close socket
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
    thread(socketRead, socketClient).detach();
  } while (endOF != 0);
  printf("Shutting down server...\n");
  close(SocketServer);
  return 0;
}
