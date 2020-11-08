#include <iostream>
#include <vector>
#include <map>
#include <string>
using namespace std;

#define PORT 50001
#define MAXLINE 1024

/// client info
int endOF = 1;
int clientSocket;
unsigned int len;
bool initialized = false;

/// server info
int points = 99;
vector<char> clientList;
struct sockaddr_in servaddr;
map<char, char> playerPrevPos;
map<char, vector<pair<int, int>>> playerPos;

// send & receive
int waitForMessage(char *buffer);
void sendToServer(string buffer);
void sendToServer(char buffer, int size);
void sendToServer(char *buffer, int size);

// utilities
string twoBytesreturn(int tam);
string threeBytesreturn(int tam);
void cleanChar(char *var, int size);

int getInt(char *buff, int a, int b)
{
    string value;
    for (int i = a; i <= b; ++i)
        if (buff[i])
            value.push_back(buff[i]);

    int val = stoi(value);
    return val;
}

string getString(char *buff, int a, int b)
{
    string value;
    for (int i = a; i <= b; ++i)
        if (buff[i])
            value.push_back(buff[i]);

    return value;
}

void menuWrite(char code)
{
    string temp;
    switch (code)
    {
    case 'W':
    case 'w':
        lastMove = 'U';
        temp = "M" + nickname + "U";
        sendToServer(temp);
        break;
    case 'A':
    case 'a':
        lastMove = 'L';
        temp = "M" + nickname + "L";
        sendToServer(temp);
        break;
    case 'D':
    case 'd':
        lastMove = 'R';
        temp = "M" + nickname + "R";
        sendToServer(temp);
        break;
    case 'S':
    case 's':
        lastMove = 'D';
        temp = "M" + nickname + "D";
        sendToServer(temp);
        break;
    case 'f':
    case 'F':
        temp = "F" + nickname + code;
        sendToServer(temp);
        break;
    case 'Q':
        endOF = 0;
        temp = "Q";
        sendToServer(temp);
    default:
        break;
    }
}

// reconstructs the map if updated
void reDrawMap()
{
    memset(mapa, ' ', 100 * 100);
    // draws walls
    for (int i = 0; i < MAXMAPSIZEI; ++i)
        for (int j = 0; j < MAXMAPSIZEJ; ++j)
            if (i == 0 || j == 0 || i == MAXMAPSIZEI - 1 || j == MAXMAPSIZEJ - 1)
                mapa[i][j] = WALL;

    // draws every player
    for (auto &client : playerPos)
    {
        bool start = true;
        for (auto &clientCoords : client.second)
            if (start)
            {
                start = false;
                mapa[clientCoords.first][clientCoords.second] = client.first;
            }
            else
            {
                int player = (int)mapa[clientCoords.first][clientCoords.second];
                if (!(player >= 65 && player <= 90 || player >= 97 && player <= 122))
                {
                    mapa[clientCoords.first][clientCoords.second] = WORMBODY;
                }
            }
    }
    cout << "\n\nYour score: " << points << endl;
    for (int i = 0; i < MAXMAPSIZEI; ++i)
    {
        for (int j = 0; j < MAXMAPSIZEJ; ++j)
            cout << mapa[i][j];
        cout << endl;
    }
}

// check if worm moved has hitted another worn or wall
bool checkColition(char nick, pair<int, int> playerNewCoords)
{
    // auto user = clientList.find(nick);
    // auto user = find(clientList.begin(), clientList.end(), nick);
    auto playerCoords = playerPos.find(nick);
    char map = mapa[playerNewCoords.first][playerNewCoords.second];

    if (((int)map >= 65 && (int)map <= 90) || ((int)map >= 97 && (int)map <= 122)) // hit a worm head
        return true;
    else if (map == WALL && map == WORMBODY)
        return true;
    return false;
}

// checks the last move
void moveSnake(char nick, char move = lastMove)
{
    auto playerCoords = playerPos.find(nick);
    vector<pair<int, int>> tmpCoords = playerCoords->second;
    switch (move)
    {
    case 'U':
        playerPrevPos.find(nick)->second = 'U';
        playerCoords->second[0].first--;
        for (int i = 1; i < 7; ++i)
            playerCoords->second[i] = tmpCoords[i - 1];
        break;
    case 'D':
        playerPrevPos.find(nick)->second = 'D';
        playerCoords->second[0].first++;
        for (int i = 1; i < 7; ++i)
            playerCoords->second[i] = tmpCoords[i - 1];
        break;
    case 'L':
        playerPrevPos.find(nick)->second = 'L';
        playerCoords->second[0].second--;
        for (int i = 1; i < 7; ++i)
            playerCoords->second[i] = tmpCoords[i - 1];
        break;
    case 'R':
        playerPrevPos.find(nick)->second = 'R';
        playerCoords->second[0].second++;
        for (int i = 1; i < 7; ++i)
            playerCoords->second[i] = tmpCoords[i - 1];
        break;
    case 'F':
        moveSnake(nick, playerPrevPos.find(nick)->second);
        break;
    }

    reDrawMap();
}

// set a new life for a send nickname
// void setNewLife(char nick, int newLife) {
//     auto user = points(nick);
//     user->second = newLife;
//     if(user->second >= 200) {
//         cout<<"Gano el jugador " << user->first<<endl;
//     }
// }

// 1 Move is ok   300 (3B)
// 2 Move Is not accepted 100
// 3 You hit a wall  600(3B) Points(2B or 3B)
// 4 You hit a worm 650 (3B) Points(2B or 3B)
// 5 You hit the head of a worm  660 (3B) Points(2B or 3B)
// example 'W' + threeBytesreturn(300) = 4 bytes
void getMoveResponse(char *buffer)
{
    reDrawMap();
    cout << "Getting code\n";
    cout << "Buffer: " << buffer << endl;
    int code = getInt(buffer, 1, 3);
    switch (code)
    {
    case 300: // move snake
        moveSnake(nickname[0]);
        break;
    case 100: // move not ok
        cout << "Invalid move\n";
        break;
    case 600: // hit a wall
    case 650: // hit a worm
    case 660: // hit a worm head
        points = getInt(buffer, 4, 6);
        if (points <= 0)
        {
            cout << "Perdiste\n";
            points = 0;
        }
        moveSnake(nickname[0]);
        break;
    }
}

//
void errorsHere(char *buffer)
{
    int cont = 1;
    int error = getInt(buffer, cont, cont + 2);
    string returning;
    cout << buffer << endl;
    switch (error)
    {
    case 400:
        //error name already used
        cout << "This letter is already used" << endl;
        cout << "Write other Nickname:    " << endl;
        cin >> nickname;
        returning = "N" + nickname;
        sendToServer(returning);
        break;
    }
}

void setInitialCoords(char *buffer)
{
    int cont = 1;
    int error = getInt(buffer, cont, cont + 2);
    string returning;
    switch (error)
    {
    case 200:
        //error name already used
        cout << "Yout letter was approved" << endl;
        clientList.push_back(buffer[1]);
        playerPrevPos.insert({buffer[1], 'K'});
        points = 99;
        int iPos, jPos;
        while (true)
        {
            cout << "Set your start   " << endl;
            cout << "Your i position:  ";
            cin >> iPos;
            cin.clear();
            cin.ignore();
            cout << "Your j position:  ";
            cin >> jPos;
            //cin >> jPos;
            if (iPos >= 1 && iPos <= 98 && jPos >= 1 && jPos <= 98)
            {
                initialized = true; //This set that the nick was initialized and can't do it again
                cout << "Were set correctly" << endl;
                break;
            }
            else
            {
                cout << "Invalid positions, they should be between 1 and 98" << endl;
            }
        }
        //Sending:      S J 003 005
        returning = "S";
        returning = returning + nickname + twoBytesreturn(iPos) + twoBytesreturn(jPos);
        sendToServer(returning);
        break;
    }
}

// N A 8080 7980 7880 7780 7680 7580 7480
void receivePlayerPositions(char *buffer)
{ //N J   003 005    070 090 ..
    int cont = 1;
    char user = buffer[cont++]; //getting the nick
    vector<pair<int, int>> newCoords;
    if (playerPrevPos.empty() || playerPrevPos.find(user) == playerPrevPos.end())
        playerPrevPos.insert({buffer[1], 'K'});

    for (int i = 0; i < 7; i++)
    {
        int posI, posJ; //2B    2B

        posI = getInt(buffer, cont, cont + 1);
        cont += 2;
        posJ = getInt(buffer, cont, cont + 1);
        cont += 2;
        newCoords.push_back(make_pair(posI, posJ));
    }
    playerPos.insert({user, newCoords});
    reDrawMap();
}

void getWinner(char buffer)
{
    for (auto &client : clientList)
    {
        if (client == buffer)
        {
            if (buffer == nickname[0])
            {
                cout << "\nGANASTE EL JUEGO DE LA CULEBRA REEEE\n";
                cout << "\nGANASTE EL JUEGO DE LA CULEBRA REEEE\n";
                cout << "\nGANASTE EL JUEGO DE LA CULEBRA REEEE\n";
                cout << "\nGANASTE EL JUEGO DE LA CULEBRA REEEE\n";
            }
            else
            {
                cout << "EL JUGADOR " << buffer << "GANO EL JUEGO DE LA CULEBRA\n";
            }
        }
    }
}

/// menu for recieving messages
void menuReceive(char *buffer)
{
    switch (buffer[0])
    {
    case 'E':               // error from server
        errorsHere(buffer); //like invalid nick
        break;
    case 'O':                     // response 200
        setInitialCoords(buffer); //valid nick
        cout << "HOLA" << endl;
        ;
        break;
    case 'N': // New positions are gotten here
              //positions of the rest     or      positions of a new user
        receivePlayerPositions(buffer);
        break;
    case 'W': // respond from move
        getMoveResponse(buffer);
        break;
    case 'G':
        getWinner(buffer[1]);
        break;
    case 'U': // update some client snake
        moveSnake(buffer[1], buffer[2]);
        break;
    case 'S': // ¿No es n? // Start pos of snake

        break;
    case 'X': //Quit
        endOF = 0;
    }
}

void socketRead()
{
    char buffer[MAXLINE];
    do
    {
        waitForMessage(buffer);

        menuReceive(buffer);
        //memset( action, '\0', 1 );
    } while (endOF != 0);
    close(clientSocket); // close socket
}

void socketWrite()
{
    string code;
    cout << "Write W to move up " << endl;
    cout << "Write A to move left  " << endl;
    cout << "Write D to move right  " << endl;
    cout << "Write S to move down  " << endl;
    cout << "Write Q to quit  " << endl;
    do
    {
        cin >> code;
        for (auto &c : code)
            menuWrite(c);

    } while (endOF != 0);
    close(clientSocket);
}

// Driver code
int main()
{
    struct hostent *host;

    host = (struct hostent *)gethostbyname((char *)"127.0.0.1");
    // host = (struct hostent *)gethostbyname((char *)"192.99.8.130");

    // Creating socket file descriptor
    if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr = *((struct in_addr *)host->h_addr);
    cout << "Enter your nickname: ";
    cin >> nickname;
    sendToServer(string("N" + nickname));

    // string word="N"+"k";
    // cout<<word<<endl;
    thread(socketRead).detach();
    //thread(socketWrite).detach();

    while (!initialized)
    {
    }
    while (endOF)
        socketWrite();
    close(clientSocket);
    return 0;
}

void cleanChar(char *var, int size)
{
    for (int i = 0; i < size; i++)
        var[i] = '\0';
}

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

void sendToServer(string buffer)
{
    cout << "\nSize: " << buffer.size() << endl;
    sendto(clientSocket, buffer.c_str(), buffer.size(),
           MSG_CONFIRM, (const struct sockaddr *)&servaddr,
           sizeof(servaddr));
}

void sendToServer(char *buffer, int size)
{
    sendto(clientSocket, buffer, size,
           MSG_CONFIRM, (const struct sockaddr *)&servaddr,
           sizeof(servaddr));
}
void sendToServer(char buffer, int size)
{
    sendto(clientSocket, (char *)&buffer, size,
           MSG_CONFIRM, (const struct sockaddr *)&servaddr,
           sizeof(servaddr));
}

int waitForMessage(char *buffer)
{
    memset(buffer, '\0', MAXLINE);
    int n = recvfrom(clientSocket, (char *)buffer, MAXLINE,
                     MSG_WAITALL, (struct sockaddr *)&servaddr,
                     &len);
    buffer[n] = '\0';
    cout << "From server: " << buffer << endl;
    return n;
}