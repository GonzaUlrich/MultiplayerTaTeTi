#include <iostream>
#include <WS2tcpip.h>
#include <list>
#include <vector>
#include <random>
#include <ctime>
#include <string>

/*
To do :
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

enum game {
	notPlaying,
	circle,
	cross = 2
};

struct User {
	string _name;
	int userID = -1;
	sockaddr_in _userip;
	sockaddr_in _userport;
	string room;
	int _isPlaying;
	bool _isWaiting;
};

struct PlayRoom {
	string _playroomname;
	char spaces[3][3];
	string _playerX;
	string _playerO;
	bool _arePlaying = false;
	bool _isXTurn;
	bool _gameEnded;
	bool _xReplay;
	bool _oReplay;
};

char serverAnswer[BUFLEN];
vector<struct sockaddr_in> generalusers;
vector<PlayRoom> playRooms;
vector<User> userList;
vector<int> waitingList;



void WriteServerAnswer(string answer) {
	int a = 0;
	for (size_t i = 0; i < BUFLEN; i++){
		if (serverAnswer[i] == '\0') {
			break;
		}
		else
			a++;
	}
	for (int j = 0; j < answer.length(); j++) {
		serverAnswer[a] = answer[j];
		a++;
	}
}

void SetUserName(sockaddr_in loginip, string newname) {
	User k;
	k._name = newname;
	k.userID = id;
	k._userip.sin_addr = loginip.sin_addr;
	k._userport.sin_port = loginip.sin_port;
	k.room = "general";
	k._isPlaying = notPlaying;
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
	vector<User>::iterator it = userList.begin();
	for (size_t i = 0; i < userList.size(); i++) {
		if (it->_userip.sin_addr.S_un.S_addr == user.sin_addr.S_un.S_addr && it->_userport.sin_port == user.sin_port) {
			return it->room;
		}
		it++;
	}
	return "none";
}


int GetUID(sockaddr_in user) {
	vector<User>::iterator it = userList.begin();
	for (size_t i = 0; i < userList.size(); i++) {
		if (it->_userip.sin_addr.S_un.S_addr == user.sin_addr.S_un.S_addr && it->_userport.sin_port == user.sin_port) {
			return it->userID;
		}
		it++;
	}
	return -1;
}

PlayRoom GetPlayerGameRoom(int user) {
	vector<PlayRoom>::iterator error = (playRooms.end() - 1);
	for (size_t i = 0; i < playRooms.size(); i++) {
		if (playRooms[i]._playroomname == userList[user].room) {
			return playRooms[i];
		}
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

void PlayerMoveMaker(int user, char place) {
	for (size_t i = 0; i < playRooms.size(); i++) {
		if (playRooms[i]._playroomname == GetPlayerGameRoom(user)._playroomname) {
			switch (place) {
			case '1':
				if (playRooms[i].spaces[0][0] == '1') {
					if (userList[user]._name == playRooms[i]._playerX)
						playRooms[i].spaces[0][0] = 'X';
					else if (userList[user]._name == playRooms[i]._playerO)
						playRooms[i].spaces[0][0] = 'O';
				}
				else {
					WriteServerAnswer("PERDISTE EL TURNO POR TRAMPOSO \n");
				}
				break;

			case '2':
				if (playRooms[i].spaces[0][1] == '2') {
					if (userList[user]._name == playRooms[i]._playerX)
						playRooms[i].spaces[0][1] = 'X';
					else if (userList[user]._name == playRooms[i]._playerO)
						playRooms[i].spaces[0][1] = 'O';
				}
				else {
					WriteServerAnswer("PERDISTE EL TURNO POR TRAMPOSO \n");
				}
				break;
			case '3':
				if (playRooms[i].spaces[0][2] == '3') {
					if (userList[user]._name == playRooms[i]._playerX)
						playRooms[i].spaces[0][2] = 'X';
					else if (userList[user]._name == playRooms[i]._playerO)
						playRooms[i].spaces[0][2] = 'O';
				}
				else {
					WriteServerAnswer("PERDISTE EL TURNO POR TRAMPOSO \n");
				}
				break;
			case '4':
				if (playRooms[i].spaces[1][0] == '4') {
					if (userList[user]._name == playRooms[i]._playerX)
						playRooms[i].spaces[1][0] = 'X';
					else if (userList[user]._name == playRooms[i]._playerO)
						playRooms[i].spaces[1][0] = 'O';
				}
				else {
					WriteServerAnswer("PERDISTE EL TURNO POR TRAMPOSO \n");
				}
				break;
			case '5':
				if (playRooms[i].spaces[1][1] == '5') {
					if (userList[user]._name == playRooms[i]._playerX)
						playRooms[i].spaces[1][1] = 'X';
					else if (userList[user]._name == playRooms[i]._playerO)
						playRooms[i].spaces[1][1] = 'O';
				}
				else {
					WriteServerAnswer("PERDISTE EL TURNO POR TRAMPOSO \n");
				}
				break;
			case '6':
				if (playRooms[i].spaces[1][2] == '6') {
					if (userList[user]._name == playRooms[i]._playerX)
						playRooms[i].spaces[1][2] = 'X';
					else if (userList[user]._name == playRooms[i]._playerO)
						playRooms[i].spaces[1][2] = 'O';
				}
				else {
					WriteServerAnswer("PERDISTE EL TURNO POR TRAMPOSO \n");
				}
				break;
			case '7':
				if (playRooms[i].spaces[2][0] == '7') {
					if (userList[user]._name == playRooms[i]._playerX)
						playRooms[i].spaces[2][0] = 'X';
					else if (userList[user]._name == playRooms[i]._playerO)
						playRooms[i].spaces[2][0] = 'O';
				}
				else {
					WriteServerAnswer("PERDISTE EL TURNO POR TRAMPOSO \n");
				}
				break;
			case '8':
				if (playRooms[i].spaces[2][1] == '8') {
					if (userList[user]._name == playRooms[i]._playerX)
						playRooms[i].spaces[2][1] = 'X';
					else if (userList[user]._name == playRooms[i]._playerO)
						playRooms[i].spaces[2][1] = 'O';
				}
				else {
					WriteServerAnswer("PERDISTE EL TURNO POR TRAMPOSO \n");
				}
				break;
			case '9':
				if (playRooms[i].spaces[2][2] == '9') {
					if (userList[user]._name == playRooms[i]._playerX)
						playRooms[i].spaces[2][2] = 'X';
					else if (userList[user]._name == playRooms[i]._playerO)
						playRooms[i].spaces[2][2] = 'O';
				}
				else {
					WriteServerAnswer("PERDISTE EL TURNO POR TRAMPOSO \n");
				}
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
				WriteServerAnswer(DrawBoard(GetPlayerGameRoom(user)) + "\nGAME ENDED! The winner of the match is " + result + "\n" + "try #help to know all the commands");
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


void Command(string _buf, int user) {
	char auxstring[UNLEN];
	int auxnum = 0;
	memset(auxstring, '\0', UNLEN);


	//#help
	if (_buf[1] == 'h' && _buf[2] == 'e' && _buf[3] == 'l' && _buf[4] == 'p') {
		WriteServerAnswer("Command List:\n #play : Looks for rooms to start playing\n");
	}
	//#play
	else if (_buf[1] == 'p' && _buf[2] == 'l' && _buf[3] == 'a' && _buf[4] == 'y') {
		
		waitingList.push_back(user);
		if (waitingList.size() == 2) {
			for (size_t i = 1; i < playRooms.size(); i++) {
				if (!playRooms[i]._arePlaying) {
					userList[waitingList[0]].room = playRooms[i]._playroomname;
					userList[waitingList[1]].room = playRooms[i]._playroomname;

					int random = rand() % 2 + 1;
					userList[waitingList[0]]._isPlaying = random;


					if (playRooms[i]._isXTurn) {
						WriteServerAnswer("The first one to start is X\n");
					}
					else {
						WriteServerAnswer("The first one to start is O\n");
					}


					if (random == cross) {
						userList[waitingList[1]]._isPlaying = circle;
						playRooms[i]._playerX = userList[waitingList[0]]._name;
						playRooms[i]._playerO = userList[waitingList[1]]._name;
						WriteServerAnswer("Game starting with " + 
							userList[waitingList[0]]._name + " as the X and " +
							userList[waitingList[1]]._name + " as the O " + '\n' +
							DrawBoard(playRooms[i]) + '\n');
					}
					else {
						userList[waitingList[1]]._isPlaying = cross;
						playRooms[i]._playerO = userList[waitingList[0]]._name;
						playRooms[i]._playerX = userList[waitingList[1]]._name;
						WriteServerAnswer("Game starting with " +
							userList[waitingList[1]]._name + " as the X and " +
							userList[waitingList[0]]._name + " as the O " + '\n' +
							DrawBoard(playRooms[i]) + '\n');
					}

					
					

					playRooms[i]._arePlaying = true;
					waitingList.clear();
					break;
				}
			}
		}
		else {
			WriteServerAnswer("A gameroom has been found! But you are alone, so you'll have to wait for another player\n");
		}
		
	}

	else {
		printf("Data: Invalid Command\n");
		WriteServerAnswer("Sorry, the command you used doesn't exist. Try using #help to check existing commands\n");
	}
}

void PlayCommand(string _buf, int user) {
	//#help
	if (_buf[1] == 'h' && _buf[2] == 'e' && _buf[3] == 'l' && _buf[4] == 'p') {
		WriteServerAnswer("Command list:\n#quit : Exists the current game room\n#surrender : Give up the current match\n#replay : When a game has ended, you can use this to choose to replay with your opponent\n#1-9 : Select the number of the space to put your piece\n");
	}
	//#quit
	else if (_buf[1] == 'q' && _buf[2] == 'u' && _buf[3] == 'i' && _buf[4] == 't') {

		if (userList[user]._name == GetPlayerGameRoom(user)._playerX)
		{
			WriteServerAnswer("X Player " + GetPlayerGameRoom(user)._playerX + " has left the play room\n");
			GetPlayerGameRoom(user)._playerX = "empty";
			userList[user].room = "general";
		}
		else
		{
			WriteServerAnswer("O Player " + GetPlayerGameRoom(user)._playerO + " has left the play room\n");
			GetPlayerGameRoom(user)._playerO = "empty";
			userList[user].room = "general";
		}
		
	}
	//#surrender
	else if (_buf[1] == 's' && _buf[2] == 'u' && _buf[3] == 'r' && _buf[4] == 'r' && _buf[5] == 'e' && _buf[6] == 'n' && _buf[7] == 'd' && _buf[8] == 'e' && _buf[9] == 'r') {
		if (!GetPlayerGameRoom(user)._gameEnded) {
			ChangeRoomBool(GetPlayerGameRoom(user), "_gameEnded", true);
			if (userList[user]._name == GetPlayerGameRoom(user)._playerX)
				WriteServerAnswer("X Player " + GetPlayerGameRoom(user)._playerX + " has surrendered, the winner is the O Player " + GetPlayerGameRoom(user)._playerO + ". Use #replay to start a new game\n");
			else if (userList[user]._name == GetPlayerGameRoom(user)._playerO)
				WriteServerAnswer("O Player " + GetPlayerGameRoom(user)._playerO + " has surrendered, the winner is the X Player " + GetPlayerGameRoom(user)._playerX + ". Use #replay to start a new game\n");
		}
		else {
			WriteServerAnswer("You can't surrender if the game has already ended\n");
		}
	}
	//#replay
	else if (_buf[1] == 'r' && _buf[2] == 'e' && _buf[3] == 'p' && _buf[4] == 'l' && _buf[5] == 'a' && _buf[6] == 'y') {
		if (GetPlayerGameRoom(user)._gameEnded) {
			if (userList[user]._name == GetPlayerGameRoom(user)._playerX) {
				WriteServerAnswer("Player X " + GetPlayerGameRoom(user)._playerX + " has accepted to rematch!\n");
				ChangeRoomBool(GetPlayerGameRoom(user), "_xReplay", true);
			}
			else if (userList[user]._name == GetPlayerGameRoom(user)._playerO) {
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
			if (userList[user]._isPlaying == cross && GetPlayerGameRoom(user)._isXTurn == true) {
				PlayerMoveMaker(user, _buf[1]);
			}
			else if (userList[user]._isPlaying == circle && GetPlayerGameRoom(user)._isXTurn == false) {
				PlayerMoveMaker(user, _buf[1]);
			}
			else {
				WriteServerAnswer("It's not your turn " + userList[user]._name + '\n');
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
			if(actualID>=0)
				cout << "Room: " << userList[actualID].room.c_str() << endl;
			cout << endl;



			if (actualID >= 0) {
				if (buf[0] == '#') {
					if (userList[actualID]._isPlaying > notPlaying)
						PlayCommand(buf, actualID);
					else
						Command(buf, actualID);
				}
				else {
					cout << userList[actualID]._name.c_str() << ": " << buf << endl;
					WriteServerAnswer(userList[actualID]._name + ": " + buf);
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
					WriteServerAnswer("Welcome user " + strName + '\n');

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
			//cout << GetURoom(sizeSocket).c_str() << " | " << userList[actualID]._name.c_str() << " : " << serverAnswer << endl;
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

