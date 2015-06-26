/**
 * Server.cpp
 *
 * By Ryan Wise
 * March 11, 2015
 *
 * Implementation of a server that handles a simple chat client by sending messages
 * between two clients, storing and retrieving user accounts, and handling authentication
 * of user accounts.
 */


#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <cstring>
#include <unistd.h> 
#include <thread>
#include <vector>

#include "server.hpp"
#include "../helper.hpp"

using namespace std;

/**
 * Constructor for the Server class. Performs the first functions to set up
 * the server socket and initializes the file structure used for storing
 * user accounts.
 */
Server::Server() {
	memset(&serverInfo, 0, sizeof serverInfo);

	serverInfo.ai_family = AF_UNSPEC;
	serverInfo.ai_socktype = SOCK_STREAM;
	serverInfo.ai_flags = AI_PASSIVE;

	int status = getaddrinfo(NULL, PORT, &serverInfo, &serverInfoList);

	numCurrentConnections = 0;

	if (status != 0) {
		cout << "Error setting address info." << endl;
	}

	struct stat sb;
	string pathname = "accounts/";
	if (stat(pathname.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
	} else {
		system("mkdir accounts/");
	}
}


Server::~Server() {
}

/**
 * [Server::interpret Based on the data sent to the server, determines what action
 * should be performed.]
 *
 * @param  str        [The string of data received from the client.]
 * @param  connection [The connection ID of the client.]
 * @return            [The response code to send back to the client.]
 */
char Server::interpret(string str, long connection) {
	vector<string> data = parseString(str);
	string function = data.at(0);

	if (function == "create") {
		return createAccount(data);
	} else if (function == "login") {
		return login(data, connection);
	} else if (function == "send") {
		sendMessage(data);
	} else if (function == "confirm") {
		return checkUserExists(data);
	} else if (function == "quit") {
		return endConversation(data);
	}

	return 0;
}


/**
 * [Server::endConversation Notifies users that their conversation was closed.]
 * @param  data [The two users in the conversation]
 * @return      [Response code to send to the client.]
 */
char Server::endConversation(vector<string> data) {
	string closer = data.at(1);
	string other = data.at(2);

	for (int i = 0; i < connections.size(); i++) {
		if (connections.at(i).userName == other) {
			connections.at(i).currentConversation.clear();
			char resp = CONVERSATION_ENDED;
			send(connections.at(i).id, &resp, 1, 0);
		}

		if (connections.at(i).userName == closer) {
			connections.at(i).currentConversation.clear();
		}
	}

	return CONVERSATION_ENDED;
}


/**
 * [Server::createAccount Creates a new account based on the parameters sent
 * from the client.]
 * @param  data [The data sent from the client to create a new account.]
 * @return      [The response code of the request.]
 */
char Server::createAccount(vector<string> data) {
	string userName = data.at(1);
	string password = data.at(2);
	string firstName = data.at(3);
	string lastName = data.at(4);

	string cmd = "mkdir -p accounts/" + userName;
	system(cmd.c_str());

	ofstream accountData("accounts/" + userName + "/account.dat");
	accountData << password << endl;
	accountData << firstName << endl;
	accountData << lastName << endl;
	accountData.close();

	return CREATE_ACCOUNT_SUCCESS;
}


/**
 * [Server::login Logs a user in by checking if the entered user name exists
 * and whether the password entered is correct.]
 * @param  data       [The user name and password entered sent from the client.]
 * @param  connection [The connection ID of the client sending the request.]
 * @return            [Response code stating whether the login was successful or not.]
 */
char Server::login(vector<string> data, long connection) {
	struct stat sb;
	string path = "accounts/" + data.at(1);

	if (stat(path.c_str(), &sb) == 0) {   // Check if account exists and confirm password
		string correctPassword;
		ifstream accountData(path + "/account.dat");
		getline(accountData, correctPassword);
		accountData.close();

		if (data.at(2) == correctPassword) {
			Connection conn;
			conn.id = connection;
			conn.userName = data.at(1);
			connections.push_back(conn);

			return PASSWORD_CORRECT;
		} else {
			return ERROR_INCORRECT_PASSWORD;
		}
	}

	return ERROR_ACCOUNT_DOES_NOT_EXIST;
}


/**
 * [checkUserExists When the client initiates a conversation with another user,
 * this function first checks to make sure that the user they entered exists and is online.]
 * @param  data [The data sent to the server from the client.]
 * @return      [Response code stating whether the user is available.]
 */
char Server::checkUserExists(vector<string> data) {
	string user = data.at(1);
	string from = data.at(2);

	for (int i = 0; i < connections.size(); i++) {
		if (connections.at(i).userName == user) {
			for (int k = 0; k < connections.size(); k++) {
				if (connections.at(k).userName == from) {
					connections.at(k).currentConversation = user;
					break;
				}
			}

			connections.at(i).currentConversation = from;

			return USER_EXISTS;
		}
	}

	return ERROR_USER_DOES_NOT_EXIST;
}


/**
 * [Server::sendMessage Sends a message to a user from another user.]
 * @param data [Vector containing who sent the message, who the message is to,
 * and the contents of the message.]
 */
void Server::sendMessage(vector<string> data) {
	string sentFrom = data.at(1);
	string sendTo = data.at(2);
	string msg = data.at(3);
	bool success = false;

	const char *message = msg.c_str();

	for (int i = 0; i < connections.size(); i++) {
		if (connections.at(i).userName == sendTo && connections.at(i).currentConversation.length() != 0) {
			size_t bytesSent;
			int len = strlen(message);
			bytesSent = send(connections.at(i).id, message, len, 0);
		}
	}
}


/**
 * [Server::mainLoop For each user that connects to the server, this loop runs
 * that constantly listens for a request from the user and sends back responses.]
 * @param connection [The connection ID of the user.]
 */
void Server::mainLoop(long connection) {
	while (true) {
		size_t bytesReceived;
		char buffer[1024];
		bytesReceived = recv(connection, buffer, 1024, 0);

		if (bytesReceived < 0) {
			cout << "Error receiving data from client!" << endl;
			break;
		} else if (bytesReceived == 0) {
			break;
		}

		buffer[bytesReceived] = '\0';

		char responseCode = interpret(buffer, connection);

		if (responseCode != 0) {
			send(connection, &responseCode, 1, 0);
		}
	}

	for (int i = 0; i < connections.size(); i++) {
		if (connections.at(i).id == connection) {
			connections.erase(connections.begin() + i);
			break;
		}
	}

	close(connection);
}


/**
 * [Server::initConnection Initializes a new connection made to the server.]
 * @param connection [The connection ID of the client who connected.]
 */
void Server::initConnection(long connection) {
	if (numCurrentConnections == MAX_CONNECTIONS) {
		cout << "The max amount of connections have been established, new incoming connection failed." << endl;
		return;
	}

	thread thr(&Server::mainLoop, this, connection);
	thr.detach();
}


/**
 * [Server::start More initialization of the socket, begins waiting for connections]
 */
void Server::start() {
	sock = socket(serverInfoList->ai_family, serverInfoList->ai_socktype, serverInfoList->ai_protocol);

	int yes = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	int status = bind(sock, serverInfoList->ai_addr, serverInfoList->ai_addrlen);

	status = listen(sock, 5);
	waitForConnection();
}


/**
 * [Server::waitForConnection Waits for new connections and accepts incoming connections
 * from the client, sending each connection to initConnection() for further initialization.]
 */
void Server::waitForConnection() {
	long connection;
	struct sockaddr_storage clientAddr;
	socklen_t clientAddrSize = sizeof(clientAddr);
	cout << "Chat server initialized." << endl;

	while (true) {	
		connection = accept(sock, (struct sockaddr *) &clientAddr, &clientAddrSize);

		if (connection == -1)
			cout << "Error establishing connection with a client!" << endl;
		else
			initConnection(connection);
	}
}


/**
 * [main Starts the server]
 * @return [exit code]
 */
int main() {
	Server server;
	server.start();
	return 0;
}