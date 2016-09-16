#pragma once

#include <map>
#include <string.h>
#include <string>
#include <vector>
#include <iostream>
#include <server.h>

using namespace std;

class Message
{
public:
	Message();
	~Message();
	map<string, vector<pair<string, string>>> messageMap;

	void storeMessage(string name, string subject, string message);
	void listMessages(string name);
	void retrieveMessages(string name, int index);
	void resetMessages();

	bool needed() {
		if (value.length() == length) {
			return false;
		}
		else return true;
	};

	string command = "";
	string name = "";
	int length = 0;
	string value = "";
};

