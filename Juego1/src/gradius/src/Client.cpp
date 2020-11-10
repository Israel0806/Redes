#include "Client.h"

ClientUDP::ClientUDP () { 
	thread (waitForMessage).detach ();
}

void ClientUDP::waitForMessage () {
    char buffer[MAXLINE];
    do {
        waitForMessage (buffer);

        menuReceive (buffer);
        //memset( action, '\0', 1 );
    } while ( true );
    close (clientSocket); // close socket

}
