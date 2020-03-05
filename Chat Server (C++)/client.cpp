#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <cstdlib>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "encrypt.hpp"

using namespace std;

#define MESSAGE_SIZE 200

string convertKey = "DANDIYA";

string convert(string input)
{
	int index = 0;
	for (int a = 0; a < input.size(); a++)
	{
		if (input[a] == ' ')
		{
			index = a + 1;
			break;
		}
	}
	string temp = input.substr(0, index);
	input = input.substr(index, input.size() - index);
	input = decrypt(input, convertKey);
	input = temp + input;

	return input;
}

int main(int argc, char** argv)
{
	//SETTING THE PORT NUMBER
	int SERV_PORT;
	cout << "Enter Port Number: ";
	cin >> SERV_PORT;

	//CREATING A SOCKET
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	//CONNECTING TO THE SERVER
	sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	if (connect(sockfd, (sockaddr*) &servaddr, sizeof(servaddr)) == -1)
	{
		cout <<"Error During Connecting" << endl;
		close(sockfd);
		exit(1);
	}
	cout << "Connected" << endl;

	//USER AUTHENTICATION
	char username[MESSAGE_SIZE], password[MESSAGE_SIZE], status;
	int loggedIn = 3;
	while (loggedIn--)
	{
		cout << "Enter Username: ";
		cin >> username;
		cout << "Enter Password: ";
		cin >> password;

		write(sockfd, username, sizeof(username));
		write(sockfd, password, sizeof(password));

		read(sockfd, &status, 1);
		if (status)
		{
			cout << "Successfully Logged In" << endl;
			break;
		}

		else
		{
			cout << "Incorrect" << endl << endl;
		}
	}
	if (loggedIn == -1)
	{
		cout << "Too Many Incorrect Attempts" << endl;
		close(sockfd);
		exit(1);
	}

	//CHOOSING A RECIPIENT
	char recipient[MESSAGE_SIZE];
	status = 0;
	while (!status)
	{
		cout << "Enter Recipient's Username: ";
		cin >> recipient;
		write(sockfd, recipient, sizeof(recipient));

		read(sockfd, &status, 1);
		if (status)
		{
			cout << "Successfully Opened Chat With " << recipient << endl;
			break;
		}
		else
		{
			cout << "Invalid Username" << endl << endl;
		}
	}

	bool firstTime = true;
	string temp;

	char outgoing[MESSAGE_SIZE], incoming[MESSAGE_SIZE] = "initial";
	while (1)
	{
		system("clear");
		//RECEIVE MESSAGES
		while (strlen(incoming))
		{
			read(sockfd, incoming, MESSAGE_SIZE);
			if (strlen(incoming) && !firstTime)
			{
				temp = string(incoming);
				cout << convert(temp) << endl;
			}
		}
		strcpy(incoming, "initial");

		//SENDING MESSAGES
		if (firstTime)
		{
			firstTime = false;
		}
		else
		{
			cout << "Enter Message: ";
		}
		cin.getline(outgoing, MESSAGE_SIZE, '\n');

		//CLOSING THE APPLICATION
		if (!strcmp(outgoing, "exit") || !strcmp(outgoing, "quit"))
		{
			cout << "Closing" << endl;
			write(sockfd, outgoing, sizeof(outgoing));
			break;
		}

		temp = string(outgoing);
		strcpy(outgoing, encrypt(temp, convertKey).c_str());
		write(sockfd, outgoing, sizeof(outgoing));
	}

	//CLOSING THE CONNECTION
	close(sockfd);
}