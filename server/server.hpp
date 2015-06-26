/**
 * server.hpp
 *
 * By Ryan Wise
 * March 11, 2015
 *
 * Header file for the server of a simple chat client.
 */

#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <netdb.h>
#include <iostream>
#include <thread>
#include <vector>
#include <pthread.h>

#define MAX_CONNECTIONS 10 // The max amount of connections that can be made

struct Connection {
	long id;  // Connection ID of this connection
	std::string userName; // The user name of the user on this connection.
	std::string currentConversation = ""; // The user name of the person this client is talking to.
};

class Server {
public:
	Server();
	virtual ~Server();

	void start();

	void waitForConnection();

	void mainLoop(long connection);

	void initConnection(long connection);

	char interpret(std::string data, long connection);

	void sendMessage(std::vector<std::string> data);

	char login(std::vector<std::string> data, long connection);

	char createAccount(std::vector<std::string> data);

	char checkUserExists(std::vector<std::string> data);

	char endConversation(std::vector<std::string> data);

private:
	const char* PORT = "8000";   // The port the server will run on
	struct addrinfo serverInfo;  // Server socket info
	struct addrinfo* serverInfoList;
	int sock;  // ID of the socket used for sending and receiving data on the socket
	int numCurrentConnections;  // Number of clients currently connected.

	std::vector<Connection> connections; // Vector of connections made to the server
};


#endif