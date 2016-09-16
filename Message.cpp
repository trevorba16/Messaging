#include "Message.h"

Message::Message()
{
	messageMap = map<string, vector<pair<string, string>>>();
}


Message::~Message()
{
}

void Message::storeMessage(string name, string subject, string message)
{
	if (messageMap.find(name) == messageMap.end()) {
		//not found in map
		vector<pair<string, string>> messageHistory = vector<pair<string, string>>();
		pair<string, string> messagePair = make_pair(subject, message);
		messageHistory.push_back(messagePair);
		messageMap.insert(make_pair(name, messageHistory));
	}
	else {
		messageMap[name].push_back(message);
	}
}

void Message::listMessages(string name)
{
	vector<pair<string, string>> messageHistory = messageMap[name];
	for (int i = 0; i < messageHistory.size(); ++i) {
		string subject = messageHistory.at(i).second;
		cout << i << ' ' << subject << '\n';
	}
}

void Message::retrieveMessages(string name, int index) {
	vector<pair<string, string>> messageHistory = messageMap[name];
	string subject = messageHistory.at(index).first;
	string message = messageHistory.at(index).second;
	int messageLength = message.length;
}

void Message::resetMessages() {
	messageMap.clear();
}
