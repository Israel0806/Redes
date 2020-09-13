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

using namespace std;

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
    printf("Server: [%s]\n", msg); // print server msg
  } while (endOF != 0);
  shutdown(p_socket, SHUT_RDWR); // shutdown socket
  close(p_socket);               // close socket
}

void socketWrite(int p_socket)
{
  char msg[1000];
  do
  {
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
  int SocketClient = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  // structure
  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(50001);                              // port number
  Res = inet_pton(AF_INET, "192.168.43.37", &stSockAddr.sin_addr); // IP

  /// connect(socket, structure, sizeof(structure))
  connect(SocketClient, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));
  endOF = 1;
  thread(socketRead, SocketClient).detach();
  thread(socketWrite, SocketClient).detach();

  do
  {

  } while (endOF != 0);
  printf("disconnecting...\n");

  // shutdown(SocketClient, SHUT_RDWR); // shutdown socket
  // close(SocketClient);               // close socket
  return 0;
}
