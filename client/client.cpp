//
//  client.cpp
//  Chat
//
//  Created by william delumpa on 2/18/15.
//  Copyright (c) 2015 william delumpa. All rights reserved.
//

#include "client.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <thread>
#include <arpa/inet.h>
#include <termios.h>
#include "../helper.hpp"

#define PORT htons(8000)

using namespace std;


/**
 * [setPasswordInput Prevents cin from echoing what the user types]
 * @param password [If true, blocks echo. When false, allows cin to echo user input.]
 */
void setPasswordInput(bool password) {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);

    if (password)
        tty.c_lflag &= ~ECHO;
    else
        tty.c_lflag |= ECHO;

    (void) tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}


/**
 *Constructor for the client. This performs the functions necessary to set up of the socket connection.
 *Initializes struct for account information.
 **/
Client::Client(){
    struct hostent *server;
    memset(&serverinfo,0, sizeof(serverinfo));
    serverinfo.sin_family = AF_INET;
    serverinfo.sin_port = PORT;

    server = gethostbyname("107.170.201.38");

    if (server == NULL) {
        cout << "Error, no such host" << endl;
        exit(0);
    }
    
    bcopy((char *)server->h_addr, (char *)&serverinfo.sin_addr.s_addr, server->h_length);
}

Client::~Client(){
};


/**
 *Client::makeSock() creates the socket to connect to the server
 *@return sock [the integer value of the socket the will host the connection]
 **/
int Client::makeSock(){                             //create client socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    return sock;
}


/**
 *[Client:makeConnection() completes the connection to the server]
 **/
void Client::makeConnection(){                      //connect to the server
    connection = connect(sock, (struct sockaddr*) &serverinfo, sizeof(serverinfo));

    if(connection < 0) {
        cout << "The server is currently offline!" << endl;
        exit(0);
    }
}

/**
 *[Client::start() logs a user in and creates an account if they don't have one. Verifies that accounts exist]
 **/
void Client::start(){
    system("clear");

    string answer;
    cout << "Welcome to the LoneRangers chat client, created by Ryan Wise and William Delumpa." << endl;
    cout << "Type \\create to create a new account or enter in your credentials to log in.\n\n";

    while (true) {
        cout << "Username: ";
        getline(cin, username);

        if (username == "\\create") {
            createAccount();
            break;
        }

        cout << "Password: ";
        setPasswordInput(true);
        getline(cin, password);
        setPasswordInput(false);
        
        string loginStr = client_login(username, password);
        send(sock, loginStr.c_str(), loginStr.length(), 0);
        char response;
        recv(sock, &response, sizeof(response), 0);

        if (response == ERROR_ACCOUNT_DOES_NOT_EXIST || response == ERROR_INCORRECT_PASSWORD) {
            cout << "The username or password you entered is incorrect! Please try again.\n\n";
        } else {
            system("clear");
            break;
        }
    }

    mainMenu();
}


/**
 * [Client::createAccount Goes through the process of creating a new account.]
 */
void Client::createAccount() {
    cout << "Enter your desired username: ";
    getline(cin, username);
    cout << "Enter a password: ";

    setPasswordInput(true);
    getline(cin, password);
    setPasswordInput(false);

    cout << "\n";

    cout << "Enter your first name: ";
    getline(cin, firstName);
    cout << "Enter your last name: ";
    getline(cin, lastName);

    string create = client_createAccount(username, password, firstName, lastName);
    send(sock, create.c_str(), create.length(), 0);

    char response;
    recv(sock, &response, sizeof(response), 0);

    if (response == CREATE_ACCOUNT_SUCCESS) {
        string loginStr = client_login(username, password);
        send(sock, loginStr.c_str(), loginStr.length(), 0);
        char response;
        recv(sock, &response, sizeof(response), 0);
        system("clear");
    }
}


/**
 *[Client:mainMenu(): choose the user you want to chat with]
 *notifies user on success/failure of the connection to friend
 **/
void Client::mainMenu() {
    cout << "Welcome " << username << "!\n\n";

    while (true) {
        cout<< "Enter the username of the person you'd like to talk to (\\q to quit): ";
        getline(cin, recipient);

        if (recipient == "\\q") {
            close(sock);
            exit(0);
        }

        string checkRecipient = client_confirmUser(recipient, username);
        int res = send(sock, checkRecipient.c_str(), checkRecipient.length(), 0);

        if (res == -1) {
            cout << "Failed to communicate with the server! Exiting..." << endl;
            close(sock);
            exit(1);
        }

        char responseCode;
        size_t bytesReceived = recv(sock, &responseCode, sizeof(responseCode), 0);

        if (bytesReceived == -1) {
            cout << "Failed to receive a response from the server! Exiting..." << endl;
            close(sock);
            exit(1);
        }

        if (responseCode == USER_EXISTS)
            break;

        cout << "The user entered either does not exist or is not online\n\n";
    }

    conversation();
}


/**
 *[Client::listenForMessages(): allocates the memory for the incoming message]
 *prints the message if it was a successful reception
 **/
void Client::listenForMessages() {
    while(true) {
        memset(buffer, 0, sizeof(buffer));
        size_t bytesReceived = recv(sock, buffer, sizeof(buffer), 0);

        if (buffer[0] == CONVERSATION_ENDED)
            break;

        if (bytesReceived == 0) {
            cout << "Lost connection to the server, closing..." << endl;
            close(sock);
            exit(1);
            break;
        }

        if (bytesReceived == -1) {
            cout<<"Failed to receive message"<<endl;
            continue;
        }

        cout << recipient << ": " << buffer << endl;
    }

    talking = false;
}


/**
 *[Client::conversation()
 *response messages are sent to the server after receiving a message]
 **/
void Client::conversation() {
    string message;

    system("clear");
    cout << "Conversation with " << recipient << ":\n\n";

    talking = true;

    thread thr(&Client::listenForMessages, this);
    thr.detach();
    
    while(true) {    
        getline(cin, message);

        if (!talking)
            break;

        if (message.length() == 0)
            continue;

        if (message == "\\q") {
            string quitConvo = client_quitConversation(username, recipient);
            send(sock, quitConvo.c_str(), quitConvo.length(), 0);
            break;
        }

        string messageToServer = client_sendMessage(username, recipient, message);
        sendAttempt = send(sock, messageToServer.c_str(), messageToServer.length(), 0);

        if (sendAttempt == -1) {
            cout<<"Message failed to send"<<endl;
        }
    }

    system("clear");

    cout << "========================================================" << endl;
    cout << "The conversation with " << recipient << " has closed." << endl;
    cout << "========================================================\n\n";

    mainMenu();
}


/**
 *[Main starts the client]
 *@return exit code 
 **/
int main() {
    Client client;
    client.makeSock();
    client.makeConnection();
    client.start();
    client.conversation();

    return 0;
}