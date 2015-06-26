/**
 * helper.hpp
 *
 * By Ryan Wise
 * March 11, 2015
 *
 * The header file for the helper. Defines constants that represent response
 * codes from the server that tell the client to perform a specific action.
 */

#ifndef HELPER_HPP_
#define HELPER_HPP_

#include <cstring>
#include <vector>
#include <iostream>

// Response codes sent back to the client from the server

#define ERROR_ACCOUNT_DOES_NOT_EXIST 1
#define ERROR_INCORRECT_PASSWORD 2
#define ERROR_USER_DOES_NOT_EXIST 3

#define PASSWORD_CORRECT 4
#define CREATE_ACCOUNT_SUCCESS 5
#define USER_EXISTS 6
#define CONVERSATION_ENDED 7


std::string convertToString(std::vector<std::string> data);

std::vector<std::string> parseString(std::string data);

std::string client_createAccount(std::string userName, std::string password, std::string firstName, std::string lastName);

std::string client_login(std::string userName, std::string password);

std::string client_sendMessage(std::string from, std::string to, std::string message);

std::string client_confirmUser(std::string confirm, std::string from);

std::string client_quitConversation(std::string from, std::string to);

#endif