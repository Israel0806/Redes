/* Client code in C */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <thread>
#include <iostream>

using namespace std;

int endOF = 1;
void cleanChar(char *var, int size)
{
  for (int i = 0; i < size; i++)
    var[i] = '\0';
}

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
    printf("%s\n", msg); // print server msg
    cleanChar(msg, 1000);
  } while (endOF != 0);
  shutdown(p_socket, SHUT_RDWR); // shutdown socket
  close(p_socket);               // close socket
}

void socketWrite(int p_socket)
{
  char msg[1000];
  do
  {
    memset(msg, ' ', 1000);
    printf("Message: ");
    // scanf("%s", msg);
    fgets(msg, 256, stdin);

    if (strcmp(msg, "chao\n") == 0)
    {
      endOF = 0;
    }
    write(p_socket, msg, strlen(msg) - 1); // write msg to server

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
  stSockAddr.sin_port = htons(50001);                              // port number
  Res = inet_pton(AF_INET, "192.168.43.37", &stSockAddr.sin_addr); // IP

  /// connect(socket, structure, sizeof(structure))
  connect(socketClient, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));
  endOF = 1;

  char msg[50];
  printf("Please enter your nickname: ");
  memset(msg, '\0', 50);
  cout.flush();
  fgets(msg, 50, stdin);
  write(socketClient, msg, strlen(msg) - 1); // write msg to server
  bool found = false;
  while (!found)
  {
    read(socketClient, msg, 50);
    // cout << "Response: " << msg << endl;
    if (strcmp("code:nickname_set", msg) == 0)
      found = true;
    else
    {
      cout << "Nickname already in use, enter a new nickname: ";
      memset(msg, '\0', 50);
      cout.flush();
      fgets(msg, 50, stdin);
      write(socketClient, msg, strlen(msg) - 1); // write msg to server
    }
  }

  cout << "Nickname set!" << endl;

  printf("Enter nickname to connect: ");
  memset(msg, '\0', 50);
  cout.flush();
  fgets(msg, 50, stdin);
  write(socketClient, msg, strlen(msg) - 1); // write msg to server
  found = false;
  while (!found)
  {
    read(socketClient, msg, 50);

    cout << "Response2: " << msg << endl;
    if (strcmp("code:nickname_found", msg) == 0)
      found = true;
    else
    {
      cout << "Nickname not found, enter a new nickname: ";
      memset(msg, '\0', 50);
      fgets(msg, 50, stdin);
      cout.flush();
      write(socketClient, msg, strlen(msg) - 1);
    }
  }

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
