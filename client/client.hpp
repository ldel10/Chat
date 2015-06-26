//
//  client.h
//  Chat
//
//  Created by william delumpa on 2/18/15.
//  Copyright (c) 2015 william delumpa. All rights reserved.
//

#ifndef __Chat__client__
#define __Chat__client__
#include <netdb.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string.h>
#include "../helper.hpp"



class Client{
public:
    Client();
    
    virtual ~Client();
    
    int makeSock();
    
    void makeConnection();
    
    void conversation();

    void listenForMessages();

    void start();
    
    void createAccount();
    
    void login();

    void mainMenu();
    
private:
    struct sockaddr_in serverinfo;
    int sock;
    int connection;
    char buffer[1000];
    long sendAttempt;
    long receiveAttempt;
    std::string username;
    std::string password;
    std::string firstName;
    std::string lastName;
    std::string recipient;
    bool talking;
};


#endif /* defined(__Chat__client__) */
