/**
 * helper.cpp
 *
 * By Ryan Wise
 * March 11, 2015
 *
 * This helper file contains functions that create strings that can be sent
 * between the server and client through a socket that both programs can
 * interpret and understand to perform specific functions.
 *
 * Strings that are sent to the server have the character delimeter of |
 * to distinguish between different arguments
 *
 * Example: the string sent to the server to create an account will be:
 *
 * 		create|<userName>|<password>|<firstName>|<lastName>
 */

#include <cstring>
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;


/**
 * [convertToString Takes a list of data and constructs a string separating the data by a | character]
 * @param  data [The vector of data]
 * @return      [A string constructed from the data passed that can be sent over the socket]
 */
string convertToString(vector<string> data) {
	string str;

	for (int i = 0; i < data.size() - 1; i++) {
		str += data.at(i) + "|";
	}

	str += data.at(data.size() - 1);
	return str;
}


/**
 * [parseString The reverse of convertToString(), take an encoded string of data and converts it into a vector]
 * @param  data [The string of data to be converted to a vector]
 * @return      [The vector holding the data]
 */
vector<string> parseString(string data) {
	istringstream stream(data);
	string token;
	vector<string> vec;

	while (getline(stream, token, '|')) {
		vec.push_back(token);
	}

	return vec;
}


/**
 * [client_createAccount Creates a string that can be sent to the server to create an account]
 * @param userName  [User's inputted user name]
 * @param password  [User's inputted password]
 * @param firstName [User's first name]
 * @param lastName  [User's last name]
 */
string client_createAccount(string userName, string password, string firstName, string lastName) {
	vector<string> data;
	data.push_back("create");
	data.push_back(userName);
	data.push_back(password);
	data.push_back(firstName);
	data.push_back(lastName);

	return convertToString(data);
}


/**
 * [client_login Creates a string that can be sent to the server to log in a user]
 * @param  userName [The user's username]
 * @param  password [The user's password]
 * @return          [The string to be sent]
 */
string client_login(string userName, string password) {
	vector<string> data;
	data.push_back("login");
	data.push_back(userName);
	data.push_back(password);

	return convertToString(data);
}


/**
 * [client_sendMessage Creates a string to be sent to the server that sends a message between two users]
 * @param  from    [The username of the user that this message is from]
 * @param  to      [The username of the user that this message is to]
 * @param  message [The message to be sent]
 * @return         [The string to be sent to send a message]
 */
string client_sendMessage(string from, string to, string message) {
	vector<string> data;
	data.push_back("send");
	data.push_back(from);
	data.push_back(to);
	data.push_back(message);

	return convertToString(data);
}


/**
 * [client_confirmUser Confirms that a user this client is trying to talk to exists and is online. Returns
 * the string that should be sent over the socket to the server.]
 * @param  confirm [The user this client wants to talk to]
 * @param  from    [This user's username]
 * @return         [The string to send to the server that makes the request]
 */
string client_confirmUser(string confirm, string from) {
	vector<string> data;
	data.push_back("confirm");
	data.push_back(confirm);
	data.push_back(from);

	return convertToString(data);
}


/**
 * [client_quitConversation Returns the string that notifies the server that this user quit the current conversation.]
 * @param  from [This client's username]
 * @param  to   [The username of the person this client is talking to.]
 * @return      [The string to send to the server.]
 */
string client_quitConversation(string from, string to) {
	vector<string> data;
	data.push_back("quit");
	data.push_back(from);
	data.push_back(to);

	return convertToString(data);
}