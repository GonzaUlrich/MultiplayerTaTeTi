#include <iostream>
#include <WS2tcpip.h>
#include <list>
#include <vector>
#include <random>
#include <ctime>
#include <string>

/*
To do :
Chequeo de posisiones en el ta te ti
Chequeo de nombres de usuario.
Limpieza de codigo
Sala de espera una vez que el partido finaliza

*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data
#define UNLEN 17	//Max length of user name 
#define ROOMLEN 25	//Max length of room name
#define MAXPR 20	//Max amount of game rooms

using namespace std;
int id = 0;

struct User {
	string _name;
	int userID;
	sockaddr_in _userip;
	sockaddr_in _userport;
	string room;
	bool _isPlaying;
	bool _isWaiting;
};

struct PlayRoom {
	string _playroomname;
	char spaces[3][3];
	string _playerX;
	string _playerO;
	bool _arePlaying;
	bool _isXTurn;
	bool _gameEnded;
	bool _xReplay;
	bool _oReplay;
};

char serverAnswer[BUFLEN];
vector<struct sockaddr_in> generalusers;
vector<PlayRoom> playRooms;
list<User> userList;



void WriteServerAnswer(string answer) {
	for (size_t i = 0; i < answer.length(); i++) {
		serverAnswer[i] = answer[i];
	}
}

void SetUserName(sockaddr_in loginip, string newname) {
	User k;
	k._name = newname;
	k.userID = id;
	k._userip.sin_addr = loginip.sin_addr;
	k._userport.sin_port = loginip.sin_port;
	k.room = "general";
	k._isPlaying = false;
	k._isWaiting = false;
	userList.push_back(k);

	id++;
}

void CleanSpacesInPlayRoom(PlayRoom selectedroom) {
	for (int i = MAXPR - 1; i >= 0; i--) {
		if (i >= 0 && i < MAXPR) {
			if (playRooms[i]._playroomname == selectedroom._playroomname) {
				playRooms[i].spaces[0][0] = '1';
				playRooms[i].spaces[0][1] = '2';
				playRooms[i].spaces[0][2] = '3';
				playRooms[i].spaces[1][0] = '4';
				playRooms[i].spaces[1][1] = '5';
				playRooms[i].spaces[1][2] = '6';
				playRooms[i].spaces[2][0] = '7';
				playRooms[i].spaces[2][1] = '8';
				playRooms[i].spaces[2][2] = '9';
			}
		}
	}
}

PlayRoom CreateGameRoom(string newname) {
	PlayRoom l;
	l._gameEnded = false;
	srand(time(NULL));
	int x = rand() % 2;
	if (x == 1) { l._isXTurn = true; }
	else { l._isXTurn = false; }
	l._oReplay = false;
	l._xReplay = false;
	l._playerO = "empty";
	l._playerX = "empty";
	l._playroomname = newname;

	return l;
}

string GetURoom(sockaddr_in user) {
	list<User>::iterator it = userList.begin();
	for (size_t i = 0; i < userList.size(); i++) {
		if (it->_userip.sin_addr.S_un.S_addr == user.sin_addr.S_un.S_addr && it->_userport.sin_port == user.sin_port) {
			return it->room;
		}
		it++;
	}
	return "none";
}

string GetUName(sockaddr_in userip) {
	list<User>::iterator it = userList.begin();
	for (size_t i = 0; i < userList.size(); i++) {
		if (it->_userip.sin_addr.S_un.S_addr == userip.sin_addr.S_un.S_addr && it->_userport.sin_port == userip.sin_port) {
			return it->_name.c_str();
		}
		it++;
	}
	return "none";
}

bool GetUPlayStatus(sockaddr_in user) {
	list<User>::iterator it = userList.begin();
	for (size_t i = 0; i < userList.size(); i++) {
		if (it->_userip.sin_addr.S_un.S_addr == user.sin_addr.S_un.S_addr && it->_userport.sin_port == user.sin_port) {
			return it->_isPlaying;
		}
		it++;
	}
	return false;
}

int GetUID(sockaddr_in user) {
	list<User>::iterator it = userList.begin();
	for (size_t i = 0; i < userList.size(); i++) {
		if (it->_userip.sin_addr.S_un.S_addr == user.sin_addr.S_un.S_addr && it->_userport.sin_port == user.sin_port) {
			return it->userID;
		}
		it++;
	}
	return -1;
}

void MoveUserRoom(sockaddr_in user, string room) {
	list<User>::iterator it = userList.begin();
	for (size_t i = 0; i < userList.size(); i++) {
		if (it->_userip.sin_addr.S_un.S_addr == user.sin_addr.S_un.S_addr && it->_userport.sin_port == user.sin_port) {
			it->room = room;
		}
		it++;
	}
}

bool FindEmptyRoom(sockaddr_in user) {
	int aux = 0;
	list<User>::iterator it = userList.begin();
	for (size_t i = 0; i < playRooms.size(); i++) {
		if (playRooms[i]._playerX == "empty" && playRooms[i]._playerO != GetUName(user)) {
			for (size_t j = 0; j < userList.size(); i++) {
				if (it->_userip.sin_addr.S_un.S_addr == user.sin_addr.S_un.S_addr && it->_userport.sin_port == user.sin_port) {
					it->room = playRooms[j]._playroomname;
					playRooms[j]._playerX = it->_name;
					it->_isPlaying = true;
					return true;
				}
				it++;
			}
		}
		else if (playRooms[i]._playerO == "empty" && playRooms[i]._playerX != GetUName(user)) {
			for (size_t k = 0; k < userList.size(); i++) {
				if (it->_userip.sin_addr.S_un.S_addr == user.sin_addr.S_un.S_addr && it->_userport.sin_port == user.sin_port) {
					it->room = playRooms[k]._playroomname;
					playRooms[k]._playerO = it->_name;
					it->_isPlaying = true;
					return true;
				}
				it++;
			}
		}
	}
	return false;

}

bool CheckUserIdentity(sockaddr_in checkip) {
	bool exists = false;
	list<User>::iterator it = userList.begin();
	for (size_t i = 0; i < userList.size(); i++) {
		if (it->_userip.sin_addr.S_un.S_addr == checkip.sin_addr.S_un.S_addr && it->_userport.sin_port == checkip.sin_port) {
			exists = true;
			break;
		}
		it++;
	}
	return exists;
}

PlayRoom GetPlayerGameRoom(sockaddr_in user) {
	list<User>::iterator itu = userList.begin();
	vector<PlayRoom>::iterator error = (playRooms.end() - 1);
	for (size_t i = 0; i < userList.size(); i++) {
		if (itu->_userip.sin_addr.S_un.S_addr == user.sin_addr.S_un.S_addr && itu->_userport.sin_port == user.sin_port) {
			for (size_t i = 0; i < playRooms.size(); i++) {
				if (playRooms[i]._playroomname == itu->room) {
					return playRooms[i];
				}
			}
		}
		itu++;
	}
	return *error;
}

string DrawBoard(PlayRoom selectedroom) {
	string completeGrid;
	completeGrid.push_back(selectedroom.spaces[0][0]);
	completeGrid += " | ";
	completeGrid.push_back(selectedroom.spaces[0][1]);
	completeGrid += " | ";
	completeGrid.push_back(selectedroom.spaces[0][2]);
	completeGrid += " ";
	completeGrid.push_back('\n');
	completeGrid += "----------\n";
	completeGrid.push_back(selectedroom.spaces[1][0]);
	completeGrid += " | ";
	completeGrid.push_back(selectedroom.spaces[1][1]);
	completeGrid += " | ";
	completeGrid.push_back(selectedroom.spaces[1][2]);
	completeGrid += " ";
	completeGrid.push_back('\n');
	completeGrid += "----------\n";
	completeGrid.push_back(selectedroom.spaces[2][0]);
	completeGrid += " | ";
	completeGrid.push_back(selectedroom.spaces[2][1]);
	completeGrid += " | ";
	completeGrid.push_back(selectedroom.spaces[2][2]);
	completeGrid += " ";
	completeGrid.push_back('\n');


	return completeGrid;
}

string CheckIfWinnerInRoom(PlayRoom selectedroom) {
	for (size_t i = 0; i < playRooms.size(); i++) {
		if (playRooms[i]._playroomname == selectedroom._playroomname) {
			if (playRooms[i].spaces[0][0] == 'X' && playRooms[i].spaces[0][1] == 'X' && playRooms[i].spaces[0][2] == 'X') {
				playRooms[i]._gameEnded = true;
				return playRooms[i]._playerX;
			}
			else if (playRooms[i].spaces[1][0] == 'X' && playRooms[i].spaces[1][1] == 'X' && playRooms[i].spaces[1][2] == 'X') {
				playRooms[i]._gameEnded = true;
				return playRooms[i]._playerX;
			}
			else if (playRooms[i].spaces[2][0] == 'X' && playRooms[i].spaces[2][1] == 'X' && playRooms[i].spaces[2][2] == 'X') {
				playRooms[i]._gameEnded = true;
				return playRooms[i]._playerX;
			}
			else if (playRooms[i].spaces[0][0] == 'X' && playRooms[i].spaces[1][0] == 'X' && playRooms[i].spaces[2][0] == 'X') {
				playRooms[i]._gameEnded = true;
				return playRooms[i]._playerX;
			}
			else if (playRooms[i].spaces[0][1] == 'X' && playRooms[i].spaces[1][1] == 'X' && playRooms[i].spaces[2][1] == 'X') {
				playRooms[i]._gameEnded = true;
				return playRooms[i]._playerX;
			}
			else if (playRooms[i].spaces[0][2] == 'X' && playRooms[i].spaces[1][2] == 'X' && playRooms[i].spaces[2][2] == 'X') {
				playRooms[i]._gameEnded = true;
				return playRooms[i]._playerX;
			}
			else if (playRooms[i].spaces[0][0] == 'X' && playRooms[i].spaces[1][1] == 'X' && playRooms[i].spaces[2][2] == 'X') {
				playRooms[i]._gameEnded = true;
				return playRooms[i]._playerX;
			}
			else if (playRooms[i].spaces[0][2] == 'X' && playRooms[i].spaces[1][1] == 'X' && playRooms[i].spaces[2][0] == 'X') {
				playRooms[i]._gameEnded = true;
				return playRooms[i]._playerO;
			}
			else if (playRooms[i].spaces[0][0] == 'O' && playRooms[i].spaces[0][1] == 'O' && playRooms[i].spaces[0][2] == 'O') {
				playRooms[i]._gameEnded = true;
				return playRooms[i]._playerO;
			}
			else if (playRooms[i].spaces[1][0] == 'O' && playRooms[i].spaces[1][1] == 'O' && playRooms[i].spaces[1][2] == 'O') {
				playRooms[i]._gameEnded = true;
				return playRooms[i]._playerO;
			}
			else if (playRooms[i].spaces[2][0] == 'O' && playRooms[i].spaces[2][1] == 'O' && playRooms[i].spaces[2][2] == 'O') {
				playRooms[i]._gameEnded = true;
				return playRooms[i]._playerO;
			}
			else if (playRooms[i].spaces[0][0] == 'O' && playRooms[i].spaces[1][0] == 'O' && playRooms[i].spaces[2][0] == 'O') {
				playRooms[i]._gameEnded = true;
				return playRooms[i]._playerO;
			}
			else if (playRooms[i].spaces[0][1] == 'O' && playRooms[i].spaces[1][1] == 'O' && playRooms[i].spaces[2][1] == 'O') {
				playRooms[i]._gameEnded = true;
				return playRooms[i]._playerO;
			}
			else if (playRooms[i].spaces[0][2] == 'O' && playRooms[i].spaces[1][2] == 'O' && playRooms[i].spaces[2][2] == 'O') {
				playRooms[i]._gameEnded = true;
				return playRooms[i]._playerO;
			}
			else if (playRooms[i].spaces[0][0] == 'O' && playRooms[i].spaces[1][1] == 'O' && playRooms[i].spaces[2][2] == 'O') {
				playRooms[i]._gameEnded = true;
				return playRooms[i]._playerO;
			}
			else if (playRooms[i].spaces[0][2] == 'O' && playRooms[i].spaces[1][1] == 'O' && playRooms[i].spaces[2][0] == 'O') {
				playRooms[i]._gameEnded = true;
				return playRooms[i]._playerO;
			}
			//tie
			else if (playRooms[i].spaces[0][0] != '1' && playRooms[i].spaces[0][1] != '2' && playRooms[i].spaces[0][2] != '3' && playRooms[i].spaces[1][0] != '4' && playRooms[i].spaces[1][1] != '5' && playRooms[i].spaces[1][2] != '6' && playRooms[i].spaces[2][0] != '7' && playRooms[i].spaces[2][1] != '8' && playRooms[i].spaces[2][2] != '9') {
				playRooms[i]._gameEnded = true;
				return "tie";
			}
			return "none";
		}
	}
}

void ChangeRoomBool(PlayRoom room, string value, bool status) {
	for (int i = MAXPR - 1; i >= 0; i--) {
		if (i >= 0 && i < MAXPR) {
			if (playRooms[i]._playroomname == room._playroomname) {
				if (value == "_gameEnded")
					playRooms[i]._gameEnded = status;
				if (value == "_isXTurn")
					playRooms[i]._isXTurn = status;
				if (value == "_oReplay")
					playRooms[i]._oReplay = status;
				if (value == "_xReplay")
					playRooms[i]._xReplay = status;
			}
		}
	}
}

void PlayerMoveMaker(sockaddr_in user, char place) {
	for (size_t i = 0; i < playRooms.size(); i++) {
		if (playRooms[i]._playroomname == GetPlayerGameRoom(user)._playroomname) {
			switch (place) {
			case '1':
				if (GetUName(user) == playRooms[i]._playerX)
					playRooms[i].spaces[0][0] = 'X';
				else if (GetUName(user) == playRooms[i]._playerO)
					playRooms[i].spaces[0][0] = 'O';
				break;
			case '2':
				if (GetUName(user) == playRooms[i]._playerX)
					playRooms[i].spaces[0][1] = 'X';
				else if (GetUName(user) == playRooms[i]._playerO)
					playRooms[i].spaces[0][1] = 'O';
				break;
			case '3':
				if (GetUName(user) == playRooms[i]._playerX)
					playRooms[i].spaces[0][2] = 'X';
				else if (GetUName(user) == playRooms[i]._playerO)
					playRooms[i].spaces[0][2] = 'O';
				break;
			case '4':
				if (GetUName(user) == playRooms[i]._playerX)
					playRooms[i].spaces[1][0] = 'X';
				else if (GetUName(user) == playRooms[i]._playerO)
					playRooms[i].spaces[1][0] = 'O';
				break;
			case '5':
				if (GetUName(user) == playRooms[i]._playerX)
					playRooms[i].spaces[1][1] = 'X';
				else if (GetUName(user) == playRooms[i]._playerO)
					playRooms[i].spaces[1][1] = 'O';
				break;
			case '6':
				if (GetUName(user) == playRooms[i]._playerX)
					playRooms[i].spaces[1][2] = 'X';
				else if (GetUName(user) == playRooms[i]._playerO)
					playRooms[i].spaces[1][2] = 'O';
				break;
			case '7':
				if (GetUName(user) == playRooms[i]._playerX)
					playRooms[i].spaces[2][0] = 'X';
				else if (GetUName(user) == playRooms[i]._playerO)
					playRooms[i].spaces[2][0] = 'O';
				break;
			case '8':
				if (GetUName(user) == playRooms[i]._playerX)
					playRooms[i].spaces[2][1] = 'X';
				else if (GetUName(user) == playRooms[i]._playerO)
					playRooms[i].spaces[2][1] = 'O';
				break;
			case '9':
				if (GetUName(user) == playRooms[i]._playerX)
					playRooms[i].spaces[2][2] = 'X';
				else if (GetUName(user) == playRooms[i]._playerO)
					playRooms[i].spaces[2][2] = 'O';
				break;
			}
			string result = CheckIfWinnerInRoom(GetPlayerGameRoom(user));
			if (result == "none") {
				if (playRooms[i]._isXTurn && playRooms[i]._gameEnded == false) {
					ChangeRoomBool(GetPlayerGameRoom(user), "_isXTurn", false);
					WriteServerAnswer(DrawBoard(GetPlayerGameRoom(user)) + "\nIt's now Player O turn\n");
				}
				else if (!playRooms[i]._isXTurn && playRooms[i]._gameEnded == false) {
					ChangeRoomBool(GetPlayerGameRoom(user), "_isXTurn", true);
					WriteServerAnswer(DrawBoard(GetPlayerGameRoom(user)) + "\nIt's now Player X turn\n");
				}
			}
			else if (result == "tie") {
				WriteServerAnswer(DrawBoard(GetPlayerGameRoom(user)) + "\nNo more moves to make! It's a TIE\n");
			}
			else if (result != "tie" && result != "none") {
				WriteServerAnswer(DrawBoard(GetPlayerGameRoom(user)) + "\nGAME ENDED! The winner of the match is " + result + "\n");
			}

		}
	}
}

bool StartRoomReplay(PlayRoom selectedroom) {
	srand(time(NULL));
	int i = rand() % 2;
	for (size_t i = 0; i < playRooms.size(); i++) {
		if (playRooms[i]._playroomname == selectedroom._playroomname) {
			if (playRooms[i]._gameEnded == true && playRooms[i]._xReplay == true && playRooms[i]._oReplay == true) {
				CleanSpacesInPlayRoom(selectedroom);
				playRooms[i]._xReplay = false;
				playRooms[i]._oReplay = false;
				playRooms[i]._gameEnded = false;
				if (i = 1) { playRooms[i]._isXTurn = true; }
				else { playRooms[i]._isXTurn = false; }
				return true;
			}
		}
	}
	return false;
}


void Command(string _buf, sockaddr_in user) {
	char auxstring[UNLEN];
	int auxnum = 0;
	memset(auxstring, '\0', UNLEN);


	//#help
	if (_buf[1] == 'h' && _buf[2] == 'e' && _buf[3] == 'l' && _buf[4] == 'p') {
		WriteServerAnswer("Command List:\n#play : Looks for rooms to start playing\n");
	}
	//#play
	else if (_buf[1] == 'p' && _buf[2] == 'l' && _buf[3] == 'a' && _buf[4] == 'y') {
		WriteServerAnswer(GetUName(user) + " is looking for a game " + '\n');

		if (true)
		{

		}

		if (FindEmptyRoom(user)) {

			/*
			//printf("Player %s has joined a game room",GUserName(user));
			if (GUserName(user) == GetPlayerGameRoom(user)._playerX && GetPlayerGameRoom(user)._playerO == "empty"){
				WriteServerAnswer("A gameroom has been found! But you are alone, so you'll have to wait for another player\n");
				GetPlayerGameRoom(user)._playerX = "full";
			}
			if (GUserName(user) == GetPlayerGameRoom(user)._playerO && GetPlayerGameRoom(user)._playerX == "empty"){
				WriteServerAnswer("A gameroom has been found! But you are alone, so you'll have to wait for another player\n");
				GetPlayerGameRoom(user)._playerX = "full";
			}
			if (GUserName(user) == GetPlayerGameRoom(user)._playerX && GetPlayerGameRoom(user)._playerO != "empty"){
				WriteServerAnswer("A gameroom has been found, and with someone in it at that!\nGame starting with " + GetPlayerGameRoom(user)._playerX + " as the X and " + GetPlayerGameRoom(user)._playerO + " as the O \n" + DrawBoard(GetPlayerGameRoom(user)) + "\n");
				GetPlayerGameRoom(user)._playerX = "full";
			}
			if (GUserName(user) == GetPlayerGameRoom(user)._playerO && GetPlayerGameRoom(user)._playerX != "empty"){
				WriteServerAnswer("A gameroom has been found, and with someone in it at that!\nGame starting with " + GetPlayerGameRoom(user)._playerX + " as the X Player and " + GetPlayerGameRoom(user)._playerO + " as the O Player\n" + DrawBoard(GetPlayerGameRoom(user)) + "\n");
				GetPlayerGameRoom(user)._playerX = "full";
			}*/
		}
		else {
			WriteServerAnswer("Sorry, but all the rooms are filled, please try again later\n");
		}
	}
	//#createroom Creates a new room
	/*
	else if (_buf[1] == 'c' && _buf[2] == 'r' && _buf[3] == 'e' && _buf[4] == 'a' && _buf[5] == 't' && _buf[6] == 'e' && _buf[7] == 'r' && _buf[8] == 'o' && _buf[9] == 'o' && _buf[10] == 'm' &&_buf[11] == ' ') {
		printf("Creating new room\n");
		if (CheckUserIdentity(user)) {
			if (_buf.length() - 7 > ROOMLEN - 1) {
				WriteServerAnswer("Name of the room too long. Max characters is " + ROOMLEN + '\n');
			}
			else {
				for (size_t i = 12; i < _buf.length(); i++) {
					auxstring[auxnum] = _buf[i];
					auxnum++;
				}
				if (CheckExistingRooms(auxstring)) {
					printf("User tried to create a room with a same name to an existing one\n");
					WriteServerAnswer("Name for room already taken");
				}
				else {
					chatrooms.push_back(auxstring);
					printf("Created new room %s\n", auxstring);
					WriteServerAnswer("Room was created succesfully\n");
				}
			}
		}
	}*/
	//#checkrooms Check aexisting and current rooms
	/*
	else if (_buf[1] == 'c' && _buf[2] == 'h' && _buf[3] == 'e' && _buf[4] == 'c' && _buf[5] == 'k' && _buf[6] == 'r' && _buf[7] == 'o' && _buf[8] == 'o' && _buf[9] == 'm' &&_buf[10] == 's') {
		if (CheckUserIdentity(user)) {
			printf("User %s is checking their room\n", GUserName(user));
			WriteServerAnswer("You are currently in the " + GUserRoom(user) + " room");
		}
	}*/
	//#movetoroom Moves to another room
	/*
	else if (_buf[1] == 'm' && _buf[2] == 'o' && _buf[3] == 'v' && _buf[4] == 'e' && _buf[5] == 't' && _buf[6] == 'o' && _buf[7] == 'r' && _buf[8] == 'o' && _buf[9] == 'o' && _buf[10] == 'm' &&_buf[11] == ' ') {
		if (CheckUserIdentity(user)) {
			for (size_t i = 12; i < _buf.length(); i++) {
				auxstring[auxnum] = _buf[i];
				auxnum++;
			}
			if (!CheckExistingRooms(auxstring)) {
				printf("User tried to join a room the doesnt exist\n");
				WriteServerAnswer("The room you want to join doesnt exist");
			}
			else {
				printf("Moving %s to another room\n", GUserName(user));
				MoveUserRoom(user, auxstring);
				WriteServerAnswer("User " + GUserName(user) + " was moved to room " + auxstring);
			}
		}
	}*/
	else {
		printf("Data: Invalid Command\n");
		WriteServerAnswer("Sorry, the command you used doesn't exist. Try using #help to check existing commands\n");
	}
}

void PlayCommand(string _buf, sockaddr_in user) {
	//#help
	if (_buf[1] == 'h' && _buf[2] == 'e' && _buf[3] == 'l' && _buf[4] == 'p') {
		WriteServerAnswer("Command list:\n#quit : Exists the current game room\n#surrender : Give up the current match\n#replay : When a game has ended, you can use this to choose to replay with your opponent\n#1-9 : Select the number of the space to put your piece\n");
	}
	//#quit
	else if (_buf[1] == 'q' && _buf[2] == 'u' && _buf[3] == 'i' && _buf[4] == 't') {
		if (GetUName(user) == GetPlayerGameRoom(user)._playerX) {
			WriteServerAnswer("X Player " + GetPlayerGameRoom(user)._playerX + " has left the play room\n");
			GetPlayerGameRoom(user)._playerX = "empty";
			MoveUserRoom(user, "general");
		}
		else if (GetUName(user) == GetPlayerGameRoom(user)._playerO) {
			WriteServerAnswer("O Player " + GetPlayerGameRoom(user)._playerO + " has left the play room\n");
			GetPlayerGameRoom(user)._playerO = "empty";
			MoveUserRoom(user, "general");
		}
	}
	//#surrender
	else if (_buf[1] == 's' && _buf[2] == 'u' && _buf[3] == 'r' && _buf[4] == 'r' && _buf[5] == 'e' && _buf[6] == 'n' && _buf[7] == 'd' && _buf[8] == 'e' && _buf[9] == 'r') {
		if (!GetPlayerGameRoom(user)._gameEnded) {
			ChangeRoomBool(GetPlayerGameRoom(user), "_gameEnded", true);
			if (GetUName(user) == GetPlayerGameRoom(user)._playerX)
				WriteServerAnswer("X Player " + GetPlayerGameRoom(user)._playerX + " has surrendered, the winner is the O Player " + GetPlayerGameRoom(user)._playerO + ". Use #replay to start a new game\n");
			else if (GetUName(user) == GetPlayerGameRoom(user)._playerO)
				WriteServerAnswer("O Player " + GetPlayerGameRoom(user)._playerO + " has surrendered, the winner is the X Player " + GetPlayerGameRoom(user)._playerX + ". Use #replay to start a new game\n");
		}
		else {
			WriteServerAnswer("You can't surrender if the game has already ended\n");
		}
	}
	//#replay
	else if (_buf[1] == 'r' && _buf[2] == 'e' && _buf[3] == 'p' && _buf[4] == 'l' && _buf[5] == 'a' && _buf[6] == 'y') {
		if (GetPlayerGameRoom(user)._gameEnded) {
			if (GetUName(user) == GetPlayerGameRoom(user)._playerX) {
				WriteServerAnswer("Player X " + GetPlayerGameRoom(user)._playerX + " has accepted to rematch!\n");
				ChangeRoomBool(GetPlayerGameRoom(user), "_xReplay", true);
			}
			else if (GetUName(user) == GetPlayerGameRoom(user)._playerO) {
				WriteServerAnswer("Player O " + GetPlayerGameRoom(user)._playerO + " has accepted to rematch!\n");
				ChangeRoomBool(GetPlayerGameRoom(user), "_oReplay", true);
			}
			if (StartRoomReplay(GetPlayerGameRoom(user))) {
				if (GetPlayerGameRoom(user)._isXTurn) { WriteServerAnswer("A new match has started!\n" + DrawBoard(GetPlayerGameRoom(user)) + "\nPlayer X, It's now your turn\n"); }
				else { WriteServerAnswer("A new match has started!\n" + DrawBoard(GetPlayerGameRoom(user)) + "\nPlayer 0, It's now your turn\n"); }

			}
		}
		else {
			WriteServerAnswer("No need to choose to replay, game is still going\n");
		}
	}
	//#"Number of placing"
	else if (_buf[1] == '1' || _buf[1] == '2' || _buf[1] == '3' || _buf[1] == '4' || _buf[1] == '5' || _buf[1] == '6' || _buf[1] == '7' || _buf[1] == '8' || _buf[1] == '9') {
		if (!GetPlayerGameRoom(user)._gameEnded) {
			if (GetUName(user) == GetPlayerGameRoom(user)._playerX && GetPlayerGameRoom(user)._isXTurn == true) {
				PlayerMoveMaker(user, _buf[1]);
			}
			else if (GetUName(user) == GetPlayerGameRoom(user)._playerO && GetPlayerGameRoom(user)._isXTurn == false) {
				PlayerMoveMaker(user, _buf[1]);
			}
			else {
				WriteServerAnswer("It's not your turn " + GetUName(user) + '\n');
			}
		}
		else {
			WriteServerAnswer("You can't play if the match has ended\n");
		}
	}
	else {
		printf("Data: Invalid Command\n");
		WriteServerAnswer("Sorry, the command you used doesn't exist. Try using #help to check existing commands\n");
	}
}

int main() {
	SOCKET _socket;
	struct sockaddr_in server, sizeSocket;
	int _socketLenght, recv_len;
	char buf[BUFLEN];
	WSADATA wsa;

	_socketLenght = sizeof(sizeSocket);


	cout << "Initialising Winsock...";
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		cout << "Failed. Error Code: " << WSAGetLastError() << endl;
		exit(EXIT_FAILURE);
	}
	cout << "Done" << endl;

	cout << "Createing socket...";
	if ((_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		cout << "Could not create socket: " << WSAGetLastError() << endl;
	}
	cout << "Done" << endl;

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	cout << "Binding socket...";
	if (bind(_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
		cout << "Bind failed with error code: " << WSAGetLastError() << endl;
		exit(EXIT_FAILURE);
	}
	cout << "Done" << endl;

	//Create game room
	string name;
	for (size_t i = 0; i < MAXPR; i++) {
		name = "GameRoom" + to_string(i);
		playRooms.push_back(CreateGameRoom(name));
	}
	for (size_t i = 0; i < MAXPR; i++) {
		CleanSpacesInPlayRoom(playRooms[i]);
	}

	//keep listening
	while (true) {

		//try to receive some data, this is a blocking call
		FD_SET fds;
		struct timeval tv;

		FD_ZERO(&fds);
		FD_SET(_socket, &fds);

		tv.tv_sec = 0;
		tv.tv_usec = 30000;

		int n = select(_socket, &fds, NULL, NULL, &tv);
		if (n > 0) {
			if ((recv_len = recvfrom(_socket, buf, BUFLEN, 0, (struct sockaddr*)&sizeSocket, &_socketLenght)) == SOCKET_ERROR) {
				cout << "recvfrom() failed with error code: " << WSAGetLastError() << endl;;
				exit(EXIT_FAILURE);
			}

			int actualID = GetUID(sizeSocket);

			//print details of the client
			int ipP1 = sizeSocket.sin_addr.S_un.S_un_b.s_b1;
			int ipP2 = sizeSocket.sin_addr.S_un.S_un_b.s_b2;
			int ipP3 = sizeSocket.sin_addr.S_un.S_un_b.s_b3;
			int ipP4 = sizeSocket.sin_addr.S_un.S_un_b.s_b4;

			cout << "Received packet from :";
			cout << "IP: " << ipP1 << "." << ipP2 << "." << ipP3 << "." << ipP4 << " ";
			cout << "Port: " << ntohs(sizeSocket.sin_port) << endl;
			cout << "Room: " << GetUName(sizeSocket) << endl;
			cout << endl;



			if (CheckUserIdentity(sizeSocket)) {
				if (buf[0] == '#') {
					if (GetUPlayStatus(sizeSocket))
						PlayCommand(buf, sizeSocket);
					else
						Command(buf, sizeSocket);
				}
				else {
					cout << GetUName(sizeSocket).c_str() << ": " << buf << endl;
					WriteServerAnswer(GetUName(sizeSocket) + ": " + buf);
				}
			}
			else {
				if (buf[0] == '#' && buf[1] == 'l' && buf[2] == 'o' && buf[3] == 'g' && buf[4] == 'i' && buf[5] == 'n' && buf[6] == ' ') {
					
					string strName;
					for (size_t i = 7; i < BUFLEN; i++){
						if (buf[i] != '\0')
							strName.push_back(buf[i]);
						else
							break;
					}
					SetUserName(sizeSocket, strName);
					generalusers.push_back(sizeSocket);
					WriteServerAnswer("Welcome user " + GetUName(sizeSocket) + '\n');

				}
				else {
					cout << "User not registered, use #login " << endl;;
					WriteServerAnswer("User not registered, use #login + your desired name\n");
					if (sendto(_socket, serverAnswer, BUFLEN, 0, (struct sockaddr*)&sizeSocket, _socketLenght) == SOCKET_ERROR) {
						cout << "sendto() failed with error code: " << WSAGetLastError() << endl;;
						exit(EXIT_FAILURE);
					}
				}
			}
			//now reply the client with the same data
			for (size_t i = 0; i < generalusers.size(); i++) {
				if (GetURoom(sizeSocket) == GetURoom(generalusers[i])) {
					if (sendto(_socket, serverAnswer, BUFLEN, 0, (struct sockaddr*)&generalusers[i], _socketLenght) == SOCKET_ERROR) {
						cout << "sendto() failed with error code: " << WSAGetLastError() << endl;
						exit(EXIT_FAILURE);
					}
				}
			}
			cout << GetURoom(sizeSocket).c_str() << " | " << GetUName(sizeSocket).c_str() << " : " << serverAnswer << endl;
			//printf("\n %s", serverAnswer);
		}

		else if (n < 0) {
			cout << "error" << endl;;
		}

		//Clear the buffer 
		memset(buf, '\0', BUFLEN);
		memset(serverAnswer, '\0', BUFLEN);
		fflush(stdin);
	}

	closesocket(_socket);
	WSACleanup();

	return 0;
}

