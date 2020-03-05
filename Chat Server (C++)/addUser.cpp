#include <iostream>
#include <fstream>
#include <string.h>
using namespace std;

int main()
{
	ofstream fileOut("Users/userList.txt", ios::app);
	fileOut.close();
	ifstream fileIn("Users/userList.txt");

	if (!fileIn)
	{
		return 1;
	}

	string usernameT, passwordT, username, password;

	cout << "Enter Username To Add: ";
	cin >> username;
	cout << "Enter Password To Add: ";
	cin >> password;

	while (!fileIn.eof())
	{
		fileIn >> usernameT >> passwordT;
		cout << usernameT << " " << passwordT << endl;

		if (usernameT == username)
		{
			cout << "Username Already Exists" << endl;
			fileIn.close();
			return 1;
		}
	}
	fileIn.close();

	fileOut.open("Users/userList.txt", ios::app);
	fileOut << username + " " + password << endl;
	fileOut.close();
	return 0;
}