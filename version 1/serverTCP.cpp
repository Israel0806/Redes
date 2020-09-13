/* Server code in C */

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
    printf("Client: [%s]\n", msg); // print server msg
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
  int SocketServer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  char buffer[256];
  int n;

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(50001);
  stSockAddr.sin_addr.s_addr = INADDR_ANY;

  bind(SocketServer, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));

  // 10 is the size of the ...
  listen(SocketServer, 10);
  int SocketClient = 0;
  SocketClient = accept(SocketServer, NULL, NULL);
  endOF = 1;
  thread(socketRead, SocketClient).detach();
  thread(socketWrite, SocketClient).detach();
  do
  {
  } while (endOF != 0);
  printf("Shutting down server...\n");
  close(SocketServer);
  return 0;
}
