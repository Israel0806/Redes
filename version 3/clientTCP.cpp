/* Client code in C */

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
#include <fstream>
#include <cctype>

using namespace std;

int endOF = 1;
bool clientSet = false;
void cleanChar(char *var, int size)
{
  for (int i = 0; i < size; i++)
    var[i] = '\0';
}

void recieveMessage(int socket)
{
  char msg[1000];
  char nickname[100];
  read(socket, msg, 2); // size of nickname
  msg[2] = '\0';
  int nicknameSize = atoi(msg);
  read(socket, nickname, nicknameSize); // nickname
  read(socket, msg, 3);                 // size of message
  nickname[nicknameSize] = '\0';
  msg[3] = '\0';
  int messageSize = atoi(msg);
  read(socket, msg, messageSize); // size of message
  cout << nickname << ": " << msg << endl;
}

string getDigits(string);
void recieveFile(int socket)
{
  char msg[1000];
  char nickname[100];
  char file[10000];
  char filename[100];
  read(socket, msg, 2); // read size of nickname
  msg[2] = '\0';
  int nicknameSize = atoi(msg);
  read(socket, nickname, nicknameSize); // read nickname
  nickname[nicknameSize] = '\0';
  read(socket, msg, 2); // read size of filename
  msg[2] = '\0';
  int filenameSize = atoi(msg);
  read(socket, filename, filenameSize); // read filename

  char digits[100];
  char digitsSize[100];

  read(socket, digitsSize, 100);          // read number of digits
  read(socket, digits, stoi(digitsSize)); // read size of file

  // cout << "File size: " << digits << endl;
  // cout << "Filename: " << filename << endl;
  int fileSize = atoi(digits);
  int n = read(socket, file, fileSize); // read file
  // cout << "Bytes recieved: " << n << endl;
  ofstream oFile(filename);
  if (oFile)
  {
    oFile << file;
    oFile.close();
  }
  cout << "File recieved!\n";
}

void recieveClientList(int socket)
{
  char nickname[100];
  char msg[1000];
  read(socket, nickname, 2); // number of nicknames
  nickname[3] = '\0';
  int numberNicknames = atoi(nickname);
  cout << "List of nicknames (" << numberNicknames << "):\n";
  int nicknameSize = 0;
  for (int i = 0; i < numberNicknames; ++i)
  {
    read(socket, msg, 2); // size of nickname
    msg[2] = '\0';
    nicknameSize = atoi(msg);
    read(socket, msg, nicknameSize); // nickname
    cout << "- " << msg << endl;
  }
}
void recieveMessageToAll(int socket)
{
  char nickname[100];
  char msg[1000];
  read(socket, nickname, 2); // size of nickname
  nickname[2] = '\0';
  int nicknameSize = atoi(nickname);
  read(socket, nickname, nicknameSize); // nickname
  nickname[nicknameSize] = '\0';
  read(socket, msg, 3); // size of message
  msg[3] = '\0';
  int messageSize = atoi(msg);
  read(socket, msg, messageSize); // message
  cout << nickname << ": " << msg << endl;
}

void socketRead(int p_socket)
{
  char msg[1000];
  do
  {
    read(p_socket, msg, 1); // code
    if (msg[0] == 'S')
      cout << "Nickname set!\n";
    else if (msg[0] == 'M')
      recieveMessage(p_socket);
    else if (msg[0] == 'Q')
      endOF = 0;
    else if (msg[0] == 'F')
      recieveFile(p_socket);
    else if (msg[0] == 'L')
      recieveClientList(p_socket);
    else if (msg[0] == 'A')
      recieveMessageToAll(p_socket);
  } while (endOF != 0);
  shutdown(p_socket, SHUT_RDWR); // shutdown socket
  close(p_socket);               // close socket
}

void setNickname(int socket, string msg)
{
  write(socket, "N", 1);
  int nicknameSize = stoi(msg.substr(0, 2));
  write(socket, msg.substr(0, 2).c_str(), 2); // write nickname size
  msg = msg.substr(2);
  write(socket, msg.c_str(), nicknameSize); // write nickname
}

void sendMessage(int socket, string msg)
{
  write(socket, "M", 1);
  int nicknameSize = stoi(msg.substr(0, 2));
  write(socket, to_string(nicknameSize).c_str(), 2);                // write nickname size
  msg = msg.substr(2);                                              // ignore nickname size
  write(socket, msg.substr(0, nicknameSize).c_str(), nicknameSize); // write nickname
  msg = msg.substr(nicknameSize);
  int messageSize = stoi(msg.substr(0, 3));
  write(socket, to_string(messageSize).c_str(), 3); // write message size
  msg = msg.substr(3);                              // ignore message size
  write(socket, msg.c_str(), messageSize);          // write message
}

string getDigits(string fileSize)
{
  int digits = 0;
  while (isdigit(fileSize[digits]))
    digits++;
  return to_string(digits);
}

void sendFile(int socket, string msg)
{
  char digits[100];
  write(socket, "F", 1);
  int nicknameSize = stoi(msg.substr(0, 2));
  msg = msg.substr(2);
  write(socket, to_string(nicknameSize).c_str(), 2);                // write size of nickname
  write(socket, msg.substr(0, nicknameSize).c_str(), nicknameSize); // write nickname
  msg = msg.substr(nicknameSize);
  int filenameSize = stoi(msg.substr(0, 2));
  msg = msg.substr(2);
  write(socket, to_string(filenameSize).c_str(), 2); // write size of filename
  string filename = msg.substr(0, filenameSize);
  write(socket, filename.c_str(), filenameSize); // write filename

  ifstream oFile(filename);
  string contents((istreambuf_iterator<char>(oFile)),
                  istreambuf_iterator<char>());
  oFile.close();
  strcpy(digits, to_string(contents.length()).c_str());
  string digitsSize = getDigits(digits);   // to see how many bytes send
  digitsSize[digitsSize.length()] = '\0';  // try to make an end
  write(socket, digitsSize.c_str(), 100);  // write number of digits
  write(socket, digits, stoi(digitsSize)); // write size

  write(socket, contents.c_str(), contents.length()); // write file
  // cout << "Bytes sent: " << contents.length() << endl;
  cout << "File sent!\n";
}

void sendClientList(int socket, string msg)
{
  write(socket, "L", 1);
}

void sendMessageToAll(int socket, string msg)
{
  write(socket, "A", 1);
  int messageSize = stoi(msg.substr(0, 3));
  write(socket, to_string(messageSize).c_str(), 3); // write message size
  msg = msg.substr(3);                              // ignore message size
  write(socket, msg.c_str(), messageSize);          // write message
}

void socketWrite(int p_socket)
{
  char code;
  string msg;
  do
  {
    // printf("Message: ");
    getline(cin, msg);
    code = msg[0];
    msg = msg.substr(1);
    if (code == 'N')
      setNickname(p_socket, msg);
    else if (code == 'M')
      sendMessage(p_socket, msg);
    else if (code == 'Q')
    {
      write(p_socket, "Q", 1);
      endOF = 0;
      break;
    } // quit
    else if (code == 'F')
      sendFile(p_socket, msg);
    else if (code == 'L')
      sendClientList(p_socket, msg);
    else if (code == 'A')
      sendMessageToAll(p_socket, msg);
    code = '\0';
    msg[0] = '\0';
  } while (endOF != 0);
  shutdown(p_socket, SHUT_RDWR); // shutdown socket
  close(p_socket);               // close socket
}

int main(void)
{
  struct sockaddr_in stSockAddr;
  int Res;
  // File descriptor ( network comm ,socket(TCP/UDP),0)
  int socketClient = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  // structure
  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(50001);                          // port number
  Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr); // IP

  /// connect(socket, structure, sizeof(structure))
  connect(socketClient, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));
  endOF = 1;

  thread(socketRead, socketClient).detach();
  thread(socketWrite, socketClient).detach();

  do
  {

  } while (endOF != 0);
  printf("disconnecting...\n");

  // shutdown(socketClient, SHUT_RDWR); // shutdown socket
  // close(socketClient);               // close socket
  return 0;
}
