#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>

using namespace std;

#define MESSAGE_SIZE 200

//RETURNS 1 IF THE USERNAME AND PASSWORD ARE AN EXACT MATCH, -1 IF ONLY THE USERNAME IS AN EXACT MATCH AND 0 OTHERWISE
int checkPassword(char username[], char password[])
{
	ifstream fileIn("Users/userList.txt");

	if (!fileIn)
	{
		return 0;
	}

	string usernameT, passwordT;
	string user = string(username);
	string pass = string(password);

	while (!fileIn.eof())
	{
		fileIn >> usernameT >> passwordT;

		if (usernameT == user)
		{
			if (passwordT == pass)
			{
				fileIn.close();
				return 1;
			}

			else
			{
				fileIn.close();
				return -1;
			}
		}
	}

	fileIn.close();
	return 0;
}

void* handleUser(void* arg)
{
	int connfd = *((int*) arg);

	//USER AUTHENTICATION
	char username[MESSAGE_SIZE], password[MESSAGE_SIZE], status = 0;  
	int loggedIn = 3;
	while (loggedIn--)
	{
		read(connfd, username, MESSAGE_SIZE);
		read(connfd, password, MESSAGE_SIZE);
		cout << "Received Username: " << username << endl;
		cout << "Received Password: " << password << endl;

		if (checkPassword(username, password) == 1)
		{
			status = 1;
		}		

		write(connfd, &status, 1);
		if (status)
		{
			cout << "Login Successful By " << username << endl;
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
		close(connfd);
		pthread_exit(NULL);
	}

	//GET RECIPIENT NAME AND OPEN RESPECTIVE FILES
	ifstream push;// input file stream
	ofstream pull;// output file stream
	char recipient[MESSAGE_SIZE];
	status = 0;
	while (!status)
	{
		read(connfd, recipient, MESSAGE_SIZE);

		if (checkPassword(recipient, recipient) && strcmp(recipient, username))
		{
			status = 1;
		}	

		write(connfd, &status, 1);
	}
	string pushT = "Users/" + string(recipient) + string(username) + ".txt";
	pull.open(pushT.c_str(), ios::app);
	pull.close();

	string pullT = "Users/" + string(username) + string(recipient) + ".txt";

	string temp;

	char outgoing[MESSAGE_SIZE], incoming[MESSAGE_SIZE];
	while (1)
	{
		//PUSH MESSAGES
		push.open(pushT.c_str());
		while (!push.eof())
		{
			push.getline(outgoing, MESSAGE_SIZE, '\n');
			if (strlen(outgoing))
			{
				cout << "Outgoing Message To " << username << ": " << outgoing << endl;
			}
			write(connfd, outgoing, sizeof(outgoing));
		}
		push.close();

		//PULL MESSAGES
		read(connfd, incoming, MESSAGE_SIZE);
		if (!strcmp(incoming, "exit") || !strcmp(incoming, "quit"))
		{
			cout << username << " Has Terminated The Connection" << endl;
			break;
		}
		if (strlen(incoming))
		{
			cout << "Incoming Message From " << username << ": " << incoming << endl;

			pull.open(pullT.c_str(), ios::app);
			pull << (string(username) + ": " + string(incoming)).c_str() << endl;
			pull.close();

			pull.open(pushT.c_str(), ios::app);
			pull << ("you: " + string(incoming)).c_str() << endl;
			pull.close();
		}
	}

	//CLOSE CONNECTION AND FILES
	close(connfd);
	push.close();

	pthread_exit(NULL);
}

int main()
{
	//SETTING THE PORT NUMBER
	int SERV_PORT;
	cout << "Enter Port Number: ";
	cin >> SERV_PORT;

	///CREATING A SOCKET
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);//AF_INET - IPv4 protocol, SOCK_STREAM - TCP connection, 0(for IP) - protocol

	//BINDING THE SOCKET
	
	//struct sockaddr_in {
	// sa_family_t sin_family;    /* Address Family */ - AF_INET - IPv4
	// uint16_t sin_port;         /* Port number */
	// struct in_addr sin_addr;   /* Internet address */
	// unsigned char sin_zero[8]; /* Pad bytes */
		
		
	sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);//htons - takes IP port number - converts 16 but IP address converts from network byte order to host order
	if (bind(listenfd, (sockaddr*) &servaddr, sizeof(servaddr)) == -1)// used to bind IP address and port of the host to the socket
	{
		cout << "Error During Binding" << endl;
		exit(1);
	}

	pthread_t threadId;// pthread_t - data type of thread
	int* arg;
	while (1)
	{
		//LISTENING FOR A CONNECTION
		if (listen(listenfd, 1) == -1)
		{
			cout << "Error During Listening" << endl;
			exit(1);
		}
		cout << "Listening" << endl;

		//ACCEPTING THE CONNECTION
		sockaddr_in cliaddr;
		unsigned int clilen = sizeof(cliaddr);
		bzero(&cliaddr, clilen);
		int connfd = accept(listenfd, (sockaddr*) &cliaddr, &clilen);
		cout << "Connected" << endl;

		arg = new int;
		*arg = connfd;
		pthread_create(&threadId, NULL, handleUser, arg);// Creates another thread, NULL - signifies Null attributes, handleUser - function to be called whne each thread
								 // is created, 
	}
}
