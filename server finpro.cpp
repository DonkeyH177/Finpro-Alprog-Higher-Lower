#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <vector>
#include <thread>
#include <ctime>

#pragma comment(lib,"ws2_32.lib")

using namespace std;

CRITICAL_SECTION cs;

int secretNumber;
bool gameOver = false;

// =====================================
// ABSTRACT CLASS
// =====================================

class Player {
protected:
    string name;

public:
    Player(string n) {
        name = n;
    }

    virtual void showRole() = 0;

    string getName() {
        return name;
    }
};

// =====================================
// INHERITANCE + POLYMORPHISM
// =====================================

class HumanPlayer : public Player {
private:
    int totalGuess;

public:
    HumanPlayer(string n) : Player(n) {
        totalGuess = 0;
    }

    void addGuess() {
        totalGuess++;
    }

    int getGuess() {
        return totalGuess;
    }

    void showRole() override {
        cout << name << " adalah Human Player" << endl;
    }
};

// =====================================
// STRUCT DATA
// =====================================

struct Score {
    string playerName;
    int totalGuess;
};

// dynamic structure
vector<Score> leaderboard;

// =====================================
// MANUAL SEARCHING
// =====================================

int searchPlayer(string target) {

    for(int i = 0; i < leaderboard.size(); i++) {

        if(leaderboard[i].playerName == target) {
            return i;
        }
    }

    return -1;
}

// =====================================
// MANUAL SORTING
// =====================================

void bubbleSort() {

    for(int i = 0; i < leaderboard.size() - 1; i++) {

        for(int j = 0; j < leaderboard.size() - i - 1; j++) {

            if(leaderboard[j].totalGuess >
               leaderboard[j + 1].totalGuess) {

                Score temp = leaderboard[j];
                leaderboard[j] = leaderboard[j + 1];
                leaderboard[j + 1] = temp;
            }
        }
    }
}

// =====================================
// JSON BUILDER
// =====================================

string makeJSON(string result) {

    string json =
        "{ \"result\": \"" + result + "\" }";

    return json;
}

// =====================================
// THREAD CLIENT
// =====================================

DWORD WINAPI handleClient(LPVOID lpParam) {

    SOCKET client_socket = (SOCKET)lpParam;

    char playerName[100];

    recv(client_socket,
         playerName,
         sizeof(playerName),
         0);

    HumanPlayer player(playerName);

    player.showRole();

    while(true) {

        if(gameOver == true) {
            break;
        }

        char buffer[100] = {0};

        recv(client_socket,
             buffer,
             sizeof(buffer),
             0);

        int guess = atoi(buffer);

        player.addGuess();

        cout << player.getName()
             << " menebak "
             << guess
             << endl;

        string result;

        EnterCriticalSection(&cs);

        if(guess > secretNumber) {
            result = "Terlalu besar";
        }

        else if(guess < secretNumber) {
            result = "Terlalu kecil";
        }

        else {
            result = "BENAR!";
            gameOver = true;
        }

        LeaveCriticalSection(&cs);

        string json =
            makeJSON(result);

        send(client_socket,
             json.c_str(),
             json.length(),
             0);

        if(result == "BENAR!") {

            Score s;

            s.playerName =
                player.getName();

            s.totalGuess =
                player.getGuess();

            leaderboard.push_back(s);

            bubbleSort();

            cout << endl;
            cout << "=== LEADERBOARD ===" << endl;

            for(int i = 0; i < leaderboard.size(); i++) {

                cout << i + 1
                     << ". "
                     << leaderboard[i].playerName
                     << " - "
                     << leaderboard[i].totalGuess
                     << " tebakan"
                     << endl;
            }

            int index =
                searchPlayer(player.getName());

            if(index != -1) {

                cout << endl;
                cout << "Player ditemukan di leaderboard index "
                     << index
                     << endl;
            }

            break;
        }
    }

    closesocket(client_socket);

    return 0;
}

// =====================================
// MAIN
// =====================================

int main() {

    srand(time(0));

    secretNumber = rand() % 100 + 1;

    InitializeCriticalSection(&cs);

    WSADATA wsa;

    WSAStartup(MAKEWORD(2,2), &wsa);

    SOCKET server_socket;

    sockaddr_in server;

    server_socket =
        socket(AF_INET,
               SOCK_STREAM,
               0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    bind(server_socket,
         (sockaddr*)&server,
         sizeof(server));

    listen(server_socket, 5);

    cout << "=== SERVER AKTIF ===" << endl;
    cout << "Menunggu player..." << endl;

    while(true) {

        SOCKET client_socket;

        sockaddr_in client;

        int c = sizeof(sockaddr_in);

        client_socket =
            accept(server_socket,
                   (sockaddr*)&client,
                   &c);

        cout << "Player connect!" << endl;

        CreateThread(NULL,
                     0,
                     handleClient,
                     (LPVOID)client_socket,
                     0,
                     NULL);
    }

    closesocket(server_socket);

    WSACleanup();

    DeleteCriticalSection(&cs);

    return 0;
}