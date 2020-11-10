#ifndef CLIENT_H
#define CLIENT_H

#include "TransportProtocol.h"
#include <thread>

#define MAXLINE 1000

class ClientUDP {
	TransportProtocol TP;
public:
	// definir el UDP setup
	// comandos
	// funciones como getInt, ya estan dentro de TransportProtocol
	ClientUDP ();

	void waitForMessage ();
};

#endif
