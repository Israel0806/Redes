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

#include <thread>
#include <chrono>

// arreglar el padding
// ACK
// 1	msg recibido sin problemas
// 2	msg recibido con errores en datos, segun el hash
// 3	Time Out rich 
// 4	Acuse con errorres en datos, segun hash

// using std::string;
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


int getInt(string buff, int a, int b);
string getString(string buff, int a, int b);


//filling to complete size of tam
void paddingZeros(string &msg, int tam){
    for(int i=msg.size();i<tam ; i++){
        msg.push_back('0');
    }
    return;
}



class TransportProtocol
{
    int count; // secuencial ID

    // idPaquete, (direccionCli, HASH)
    map<int, pair<sockaddr_in, int> > IDSent;
    unsigned int socket;

public:
    TransportProtocol(unsigned int socket)
    {
        count = 0;
        this->socket = socket;
    };

    void sendToSomeone(string buffer, sockaddr_in sockAddr)
    {
        sendto(socket, buffer.c_str(), buffer.size(),
               MSG_CONFIRM, (const struct sockaddr *)&sockAddr,
               sizeof(sockAddr));
    }

    void setSocket(unsigned int socket)
    {
        this->socket = socket;
    }

    // Send msg to someone
    void sendMsg(int msgType, string msg, sockaddr_in sockAddr)
    {
        string ret = constructMsg(msgType, msg, sockAddr);
        sendToSomeone(ret, sockAddr);
        if( msgType == 1 or msgType == 3)
            // thread(waitForACK, ret, getInt(ret, 1, 5)).detach();
            // std :: thread t (Process, a); // before modification
            std::thread (&TransportProtocol::waitForACK, this, ret, getInt(ret, 1, 5)).detach();   //After modifying //
    }

    // 1 y 3 mandamos mensaje
    // 2 y 4 mandamos id
    // 5 y 6 igual que 1 y 2
    string constructMsg(int msgType, string msg, sockaddr_in sockAddr, int ID = -1) {
        return constructMsg(to_string(msgType), msg, sockAddr, ID);
    }

    string constructMsg(string msgType, string msg, sockaddr_in sockAddr, int ID)
    {
        string returning = msgType; // MsgType
        if (ID == -1)
        {
            if (count == 65353)
                count = 0;

            // IDSent.insert({count, 1});
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
        checkingSum = checkingSum % HASH;
        // returning += stringReturn(3, abs(HASH - checkingSum)); // HASH
        returning += stringReturn(3, checkingSum);     // HASH

        //memset(&msg + msg.size(), '0', 1000 - 11 + msg.size()); 
        //std::fill( msg.begin() + msg.size(), str.begin() + msg.size() + 1000 - 11, '0' );

        //filling zeros
        paddingZeros(msg,1000);

        // returning += stringReturn(3, 1000 - 11 + msg.size()); // padding
        if(ID == -1 ) {
            IDSent.insert({count, pair<sockaddr_in, int>(sockAddr, checkingSum)});
        }
        return returning;
    }

    /// busy wait para esperar por el ACK
    void waitForACK(string buffer, int packetNumber)
    {
        /// send message with ACK
        // sendto(clientSocket, string(buffer + threeBytesreturn(idPaqueteTMP)).c_str(), size + 3,
        //     MSG_CONFIRM, (const struct sockaddr *)&servaddr,
        //     sizeof(servaddr));

        auto start = chrono::system_clock::now();
        int TO = 0;
        while (TO < 3)
        {
            auto end = chrono::system_clock::now();
            chrono::duration<float, milli> duration = end - start;
            /// si ya llego, salir
            // invalid use of member ‘TransportProtocol::IDSent’ in static member functionTransportProtocol.h: In static member function ‘static void TransportProtocol::waitForACK(std::string, int)’:
            //TransportProtocol.h:187:27: error: invalid use of member ‘TransportProtocol::IDSent’ in static member function
            //187 |             auto packet = IDSent.find(packetNumber);
            
            auto packet = IDSent.find(packetNumber);
            if ( packet == IDSent.end())
            {
                // cout << "Paquete con id " << idPaqueteTMP << " recibido\n";
                return;
            }
            // si aun no se sabe si llego, se manda otra vez
            if (duration > chrono::milliseconds(150))
            {
                // auto packet = paquetes.find(packetNumber);
                start = chrono::system_clock::now();
                sendToSomeone(buffer, packet->second.first);
                // sendto(socket, string(buffer + threeBytesreturn(idPaqueteTMP)).c_str(), size + 3,
                //     MSG_CONFIRM, (const struct sockaddr *)&packet.second.first,
                //     sizeof(packet.second.first));
            }
            ++TO;
        }
        if(TO >= 3) 
            cout<<"Conecction error, could not sent message\n";
        
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
        else {
            // verificacion del hash
            // map<int, pair<sockaddr_in, int> > IDSent;
            
            IDSent.erase(ID);
            cout << "Data has been sent\n";
        }
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
        else {
            cout << "Data has been sent\n";
            IDSent.erase(ID);
        }
    }
    /*
    Send and Forget
    9
    00088
    001
    1
    666
    000000000000000000000000
    */
    void msgType5(string msg, sockaddr_in sockAddr) {
        
        string ret = constructMsg(9, string("1"), sockAddr, getInt(msg, 1, 5));
        sendToSomeone(ret, sockAddr);
    }

    void msgType9(string msg, sockaddr_in sockAddr) {
        int ID = getInt(msg, 1, 5); // extract ID
        auto encontro = IDSent.find(ID);
        if (encontro == IDSent.end())
            cout << "Network Error\n";
        else {
            // V/F
            // if( msg[9] == '1') {
            cout << "Data has been sent\n";
            IDSent.erase(ID);
        }
    }


    void msgType6(string msg, sockaddr_in sockAddr) {
        string ret = constructMsg('A', string("1"), sockAddr, getInt(msg, 1, 5));
        sendToSomeone(ret, sockAddr);
    }

    void msgTypeA(string msg, sockaddr_in sockAddr) {
        int ID = getInt(msg, 1, 5); // extract ID
        auto encontro = IDSent.find(ID);
        if (encontro == IDSent.end())
            cout << "Network Error\n";
        else {
            // V/F
            if( msg[9] == '1')
                cout << "Data has been sent\n";
            else 
                cout << "Data has not been sent\n";
            IDSent.erase(ID);
        }
    }

    void receiveMessage(char msgType, string msg, sockaddr_in sockAddr)
    {
        bool rightData = verifyHash(msg);
        char err=' ';
        int idMsg=getInt(msg, 1, 5);
        // constructMsg(msgType)
        switch (msgType)
        {
        case '1': //Request 
            err='2'; 
            if(!rightData)
                break;

            msgType1(msg, sockAddr);        //ERROR 2
            break;
        case '2': //Response
            err='4'; 
            if(!rightData)
                break;

            msgType2(msg);                  //ERROR 4
            
            break;
        case '3': //Solicitation
            err='2'; 
            if(!rightData)
                break;

            msgType3(msg, sockAddr);        //ERROR 2
            break;
        case '4': //Replay
            err='4'; 
            if(!rightData)
                break;

            msgType4(msg);                  //ERROR 4
            break;
        case '5': //Send and Forget   
            err='4'; 
            if(!rightData)
                break;
            
            msgType5(msg, sockAddr);        //ERROR 4
            break;
        case '6': //Notification
            err='4'; 
            if(!rightData)
                break;
            
            msgType6(msg, sockAddr);        //ERROR 4
            break;
        case '9':
            msgType9(msg, sockAddr);
            break;
        case 'A': 
            msgTypeA(msg, sockAddr);
            break;
        }
    
        if(!rightData) {
            msg="";
            int idMsg=getInt(msg, 1, 5);
            constructMsg(err, msg, sockAddr, idMsg); //
        }
    }

    bool verifyHash (string msg) {
        int counter = 1, msg_size;
        //msg_size=getInt(msg,counter,counter+4);
        counter += 5; //MSG ID
        msg_size = getInt (msg, counter, counter + 2); //Getting msg size
        counter += 3;
        string data = getString (msg, counter, counter + msg_size);
        counter += msg_size;

        int hash_value = 0, right_hash;       //Right_hash is what I receive in the msg
        for ( int i = 0; i < data.size (); i++ ) {
            hash_value += int (data[i]);
        }
        hash_value = hash_value % 666;

        right_hash = getInt (msg, counter, counter + 2);
        counter += 3;

        if ( right_hash == hash_value ) {
            return true;
        } else {
            return false;
        }
    }


    int waitForMessage (char *buffer) {
        memset (buffer, '\0', MAXLINE);
        int n = recvfrom (socket, (char *)buffer, MAXLINE,
            MSG_WAITALL, (struct sockaddr *) & servaddr,
            sizeof(servaddr));
        buffer[n] = '\0';
        cout << "From server: " << buffer << endl;
        return n;
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

int getInt(string buff, int a, int b)
{
    string value;
    for (int i = a; i <= b; ++i)
        if (buff[i])
            value.push_back(buff[i]);

    int val = stoi(value);
    return val;
}

string getString(string buff, int a, int b)
{
    string value;
    for (int i = a; i <= b; ++i)
        if (buff[i])
            value.push_back(buff[i]);

    return value;
}

#endif