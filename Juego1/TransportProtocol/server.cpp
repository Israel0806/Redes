#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <iostream>
#include <thread>
#include <map>
#include <cstring>
#include <vector>
#include <time.h>

#define PORT 50001
#define MAXLINE 1024

using namespace std;

int endOF = 1;
int n;

map<char, sockaddr_in> clientList;
map<char, int> points;

//map of users' coordinates
map<char, vector<pair<int, int>>> playerPos;
// stores nickname, lastMove
sockaddr_in servaddr, cliaddr;
unsigned int len;
unsigned int clientSocket;

void menu();
void UDPsetup();
void QFunction();
void socketRead();

int waitForMessage(char *buffer, sockaddr_in &clientTMP);

int getInt(char *buff, int a, int b)
{
    string value;
    for (int i = a; i <= b; i++)
    {
        value.push_back(buff[i]);
    }
    int val = stoi(value);
    return val;
}


// 1 Move is ok   300 (3B)
// 2 Move Is not accepted 100
// 3 You hit a wall  600(3B)
// 4 You hit a worm 650 (3B)
// 5 You hit the head of a worm  660 (3B)
int checkMov(char mov, char nickname)
{
    auto pos = playerPos.find(nickname);
    auto lastPos = playerPrevPos.find(nickname);

    pair<int, int> currentPos = pos->second[0]; // head coords
    pair<int, int> newPos = getNewCoords(mov, currentPos, lastPos->second);
    int player = (int)mapa[newPos.first][newPos.second];
    // cout << newPos.first << "==" << pos->second[1].first << " " << newPos.second << "==" << pos->second[1].second << endl;
    // cout << newPos.first << "==" << pos->second[0].first << " " << newPos.second << "==" << pos->second[0].second << endl;
    if (newPos.first == pos->second[1].first && pos->second[1].second == newPos.second)
        return 2; // not ok
    else if (newPos.first == 0 || newPos.first == 99 || newPos.second == 0 || newPos.second == 99)
        return 3; // hit a wall
    else
    {
        // moveSnake(nickname, mov);
        if (mapa[newPos.first][newPos.second] == WORMBODY)
            return 4; // hit a worn
        else if ((player >= 65 && player <= 90) || (player >= 97 && player <= 122))
            return 5; // hit a head
        else
            return 1; // ok
    }
}

/// compares if two sockaddr_in are equal
bool cmpSockaddr(sockaddr_in addr1, sockaddr_in addr2)
{
    pair<int, int> add1 = pair<int, int>((int)addr1.sin_addr.s_addr, (int)addr1.sin_port);
    pair<int, int> add2 = pair<int, int>((int)addr2.sin_addr.s_addr, (int)addr2.sin_port);
    if (add1.first == add2.first && add1.second == add2.second)
        return true;
    return false;
}

void validateNickname(sockaddr_in &clientTMP, char *buffer)
{
    string returning;
    char nick = buffer[1];

    //if it is found

    if (clientList.empty() || clientList.find(buffer[1]) == clientList.end())
    { //Return    O200

        //insert new nick
        clientList.insert({buffer[1], clientTMP});

        //sending that the letter was successfully created
        returning = "O200";
        sendToClient(returning, clientTMP, returning.size());
        cout << returning << endl;

        //send to the new user the coords of all the users (except himself)
        //N P 007 009......
        for (auto &item : playerPos)
        {
            if (item.first != nick)
            {
                returning = "N";                             // (NJ0503 * 7) * clientList.size()
                returning += item.first;                     // NS0207091015203050
                for (int i = 0; i < item.second.size(); i++) //getting coords of each user in string
                {
                    returning += twoBytesreturn(item.second[i].first);  //2B axis i
                    returning += twoBytesreturn(item.second[i].second); //2B axis j
                }
                sendToClient(returning, clientTMP, returning.size());
            }
        }
    }
    else
    {
        cout << "ERROR";
        returning = "E400";
        sendToClient(returning, clientTMP, returning.size());
    }
}

void settingCoords(sockaddr_in &clientTMP, char *buffer)
{
    cout << buffer << endl;
    int cont = 1, coordI, coordJ;
    char nick = buffer[cont];
    cont++;
    coordI = getInt(buffer, cont, cont + 1); //getting coord x
    cont += 2;
    coordJ = getInt(buffer, cont, cont + 1); //getting coord y
    cont += 2;
    //look for emptyblock and place it
    vector<pair<int, int>> coords; //this saves new coords
    char lastMove;

    coords = newCoordinates(coordI, coordJ, lastMove); //receive a valid

    //insert coords and last movement
    playerPos.insert({nick, coords});       //all the 7 positions of the worm
    playerPrevPos.insert({nick, lastMove}); //LRUF last movement
    points.insert({nick, 99});              //Insert points

    //send to all users the new coords (including the new user)
    string returning = "N"; //N J   005 007     008 010 ....
    returning += nick;
    for (int i = 0; i < coords.size(); i++) //getting coords in string
    {
        returning += twoBytesreturn(coords[i].first);  //2Bytes
        returning += twoBytesreturn(coords[i].second); //2Bytes
    }
    for (auto &item : clientList)
    {
        sendToClient(returning, item.second, returning.size()); //sending all
    }
}

// Driver code
int main()
{
    UDPsetup();
    memset(mapa, ' ', 100 * 100);
    for (int i = 0; i < MAXMAPSIZEI; ++i)
        for (int j = 0; j < MAXMAPSIZEJ; ++j)
            if (i == 0 || j == 0 || i == MAXMAPSIZEI - 1 || j == MAXMAPSIZEJ - 1)
                mapa[i][j] = WALL;
    thread(socketRead).detach();

    while (endOF)
    {
    }
    return 0;
}

// exit function
void QFunction(sockaddr_in &clientTMP)
{
    for (auto &item : clientList)
    {
        if (cmpSockaddr(item.second, clientTMP))
        {
            clientList.erase(item.first);
            playerPrevPos.erase(item.first);
            playerPos.erase(item.first);
            // sendToClient('Q', clientTMP, 1);
        }
    }
    // close(clientSocket);
}
/// reads the command from client
void menu(char *buffer, sockaddr_in &clientTMP)
{
    switch (buffer[0])
    {
    case 'N': // client sends nickname
        cout << buffer[1] << endl;
        validateNickname(clientTMP, buffer);
        break;
    case 'M': // client movement
        movePlayerSnake(buffer, clientTMP);
        break;
    case 'S': // Setting the coords
        settingCoords(clientTMP, buffer);
        break;
    case 'Q': //Quit
        QFunction(clientTMP);
        break;
    }
}
// returns a 3 bye number filled with 0 to the left
string twoBytesreturn(int tam)
{
    string returning, conv;
    if (tam < 10)
    {
        returning.push_back('0');
    }
    conv = to_string(tam);
    returning += conv;
    return returning;
}
string threeBytesreturn(int tam)
{
    string returning, conv;
    if (tam < 10)
    {
        returning.push_back('0');
        returning.push_back('0');
    }
    else
    {
        if (tam < 100)
        {
            returning.push_back('0');
        }
    }
    conv = to_string(tam);
    returning += conv;
    return returning;
}
/// wait for a mesage from a user and sends the command to the menu
void socketRead()
{
    char code[MAXLINE];
    do
    {
        //memset( code, '\0', MAXLINE );
        sockaddr_in clientTMP;

        waitForMessage(code, clientTMP);
        //NJ
        cout << code << endl;
        menu(code, clientTMP);
        memset(code, '\0', MAXLINE);
    } while (endOF != 0);
    close(clientSocket); // close socket
}
/// default config for UDP server
void UDPsetup()
{
    // Creating socket file descriptor
    if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
    // Bind the socket with the server address
    if (bind(clientSocket, (const struct sockaddr *)&servaddr,
             sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    len = sizeof(cliaddr); //len is value/resuslt
}

/// receives the ipPort of the client to send message
void sendToClient(string buffer, pair<int, int> ipPort, int size)
{
    sendToClient(buffer.c_str(), ipPort, size);
}
void sendToClient(char buffer, pair<int, int> ipPort, int size)
{
    sendToClient(&buffer, ipPort, size);
}
void sendToClient(char *buffer, pair<int, int> ipPort, int size)
{
    struct sockaddr_in tmp;
    tmp = cliaddr;
    tmp.sin_addr.s_addr = ipPort.first;
    tmp.sin_port = ipPort.second;

    sendto(clientSocket, buffer, size,
           MSG_CONFIRM, (const struct sockaddr *)&tmp,
           sizeof(tmp));
}
/// send a message to passed client socket
void sendToClient(string buffer, sockaddr_in cli, int size)
{
    sendto(clientSocket, buffer.c_str(), size,
           MSG_CONFIRM, (const struct sockaddr *)&cli,
           sizeof(cli));
}
void sendToClient(char *buffer, sockaddr_in cli, int size)
{
    sendto(clientSocket, buffer, size,
           MSG_CONFIRM, (const struct sockaddr *)&cli,
           sizeof(cli));
}
void sendToClient(char buffer, sockaddr_in cli, int size)
{
    sendto(clientSocket, (char *)&buffer, size,
           MSG_CONFIRM, (const struct sockaddr *)&cli,
           sizeof(cli));
}
/// wait for mesages
int waitForMessage(char *buffer, sockaddr_in &clientTMP)
{
    memset(buffer, '\0', MAXLINE);
    int n = recvfrom(clientSocket, (char *)buffer, MAXLINE,
                     MSG_WAITALL, (struct sockaddr *)&cliaddr,
                     &len);
    buffer[n] = '\0';
    clientTMP = cliaddr;
    //cout<<buffer[0]<<" "<<buffer[1]<<endl;
    cout << "Message received: " << buffer << endl;

    return n;
}
