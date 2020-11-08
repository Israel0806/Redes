#ifndef TRANSPORTPROTOCOL_H
#define TRANSPORTPROTOCOL_H

/*
Recibir Msg, TypeMsg
Manda al server: TypeMsg, MsgID, MsgSize, Msg, HASH, Padding
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <string>
#include <cstring>
#include <iostream>
#include <map>
#include <math.h>

using std::string;
using namespace std;

#define MsgSize 3
#define IDSize 5
#define HASH 666

string stringReturn(int, int);

/*
Type Msg
Request	Response	    1	2      v        
Solicitation	Replay	3	4
Send and Forget	        5   ---	
Notification		    --- 6
*/

class TransportProtocol
{
    int count; // secuencial ID
    map<int, int> IDSent;
    unsigned int socket;

public:
    TransportProtocol(unsigned int socket)
    {
        count = 0;
        this->socket = socket;
    };

    void setSocket(unsigned int socket)
    {
        this->socket = socket;
    }

    // Send msg to someone
    void sendMsg(int msgType, string msg, sockaddr_in sockAddr)
    {
        string ret = constructMsg(msgType, msg, sockAddr);
        sendToSomeone(ret, sockAddr);
    }

    // 1 y 3 mandamos mensaje
    // 2 y 4 mandamos id
    // 5 y 6 igual que 1 y 2
    string constructMsg(int msgType, string msg, sockaddr_in sockAddr, int ID = -1)
    {
        string returning = to_string(msgType); // MsgType
        if (ID == -1)
        {
            if (count == 65353)
                count = 0;

            IDSent.insert({count, 1});
            returning += stringReturn(5, count++); // msgID
        }
        else
        {
            returning += stringReturn(5, ID); // msgID
        }
        returning += stringReturn(3, msg.size()); // msg Size

        // minus 11 because maximun message size is 1000B
        if (msg.size() > 999 - 11)
            returning += msg.substr(0, 999 - 11);
        else
            returning += msg; // msg

        // compute HASH
        int checkingSum = 0;
        for (int i = 0; i < msg.size(); i++)
            checkingSum += int(msg[i]);

        // returning += stringReturn(3, abs(HASH - checkingSum)); // HASH
        returning += stringReturn(3, checkingSum % HASH);     // HASH
        returning += stringReturn(3, 1000 - 11 + msg.size()); // padding

        return returning;
    }

    // Server: Send response msg
    void msgType1(string msg, sockaddr_in sockAddr)
    {
        int ID = stoi(msg.substr(1, 5));                // extract ID
        string ret = constructMsg(2, "", sockAddr, ID); // send Response to client
        sendToSomeone(ret, sockAddr);
    }

    // Client: Compare with last ID sent
    void msgType2(string msg)
    {
        // comprobar que el ID que fue enviado es valido
        int ID = stoi(msg.substr(1, 5)); // extract ID
        auto encontro = IDSent.find(ID);
        if (encontro == IDSent.end())
            cout << "Network Error\n";
        else
            cout << "Data has been sent\n";
        // sendToSomeone(ret, sockAddr);
    }

    // Client: Send Replay
    void msgType3(string msg, sockaddr_in sockAddr)
    {
        int ID = stoi(msg.substr(1, 5));                // extract ID
        string ret = constructMsg(4, "", sockAddr, ID); // send Response to client
        sendToSomeone(ret, sockAddr);
    }

    // Server: Replay
    void msgType4(string msg)
    {
        // comprobar que el ID que fue enviado es valido
        int ID = stoi(msg.substr(1, 5)); // extract ID
        auto encontro = IDSent.find(ID);
        if (encontro == IDSent.end())
            cout << "Network Error\n";
        else
            cout << "Data has been sent\n";
        // sendToSomeone(ret, sockAddr);
    }

    void msgType5(string msg, sockaddr_in sockAddr) {}

    void msgType6(string msg, sockaddr_in sockAddr) {}

    void receiveMessage(int msgType, string msg, sockaddr_in sockAddr)
    {
        // constructMsg(msgType)
        switch (msgType)
        {
        case 1: //Request
            msgType1(msg, sockAddr);
            break;
        case 2: //Response
            msgType2(msg);
            break;
        case 3: //Solicitation
            msgType3(msg, sockAddr);
            break;
        case 4: //Replay
            msgType4(msg);
            break;
        case 5: //Send and Forget
            msgType5(msg, sockAddr);
            break;
        case 6: //Notification
            msgType6(msg, sockAddr);
            break;
        }
    }

    void sendToSomeone(string buffer, sockaddr_in sockAddr)
    {
        sendto(socket, buffer.c_str(), buffer.size(),
               MSG_CONFIRM, (const struct sockaddr *)&sockAddr,
               sizeof(sockAddr));
    }
};

//size= size of the string to return
//value= value to convert to string
string stringReturn(int size, int value)
{
    string tmp = to_string(value);
    string ret = "";
    for (int i = tmp.size(); i < size; ++i)
        ret += '0';

    ret += tmp;
    return ret;
}

#endif