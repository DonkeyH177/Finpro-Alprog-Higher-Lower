#include <iostream>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")

using namespace std;

// =====================================
// JSON PARSER
// =====================================

string getResult(string json) {

    int start =
        json.find(":");

    int end =
        json.find("}");

    string result =
        json.substr(start + 3,
                    end - start - 4);

    return result;
}

int main() {

    WSADATA wsa;

    WSAStartup(MAKEWORD(2,2), &wsa);

    SOCKET s;

    sockaddr_in server;

    s = socket(AF_INET,
               SOCK_STREAM,
               0);

    server.sin_addr.s_addr =
        inet_addr("127.0.0.1");

    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    connect(s,
            (sockaddr*)&server,
            sizeof(server));

    string name;

    cout << "Masukkan nama player: ";
    cin >> name;

    send(s,
         name.c_str(),
         name.length(),
         0);

    while(true) {

        int guess;

        cout << endl;
        cout << "Tebak angka (1-100): ";
        cin >> guess;

        char num[20];

        sprintf(num, "%d", guess);

        send(s,
             num,
             strlen(num),
             0);

        char response[100] = {0};

        recv(s,
             response,
             sizeof(response),
             0);

        string result =
            getResult(response);

        cout << "Server: "
             << result
             << endl;

        if(result == "BENAR!") {
            break;
        }
    }

    closesocket(s);

    WSACleanup();

    return 0;
}