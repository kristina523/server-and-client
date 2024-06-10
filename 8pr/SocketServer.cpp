#define WIN32_LEAN_AND_MEAN // ��������� ����� ������������ ��������� Windows ��� ��������� �������� ������
#include <Windows.h>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

int main() {
    WSADATA wsaData; // ������ ������ � ���������� ������� Windows
    ADDRINFO hints; // ��������� ��� ������� getaddrinfo
    ADDRINFO* addrResult; // ����� ������� �������� ���������� �����������
    SOCKET ListenSocket = INVALID_SOCKET; // ������ ��� ������������� �������� �����������
    SOCKET ConnectSocket = INVALID_SOCKET; // ������ ��� ����������� � �������
    char recvBuffer[512]; // ����� ��� ������ ������

    const char* sendBuffer = "Hello from server"; // ��������� ��� �������� �������

    // ���������������� Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed with result: " << result << endl;
        return 1;
    }

    // ��������� ��������� ��������� ��� getaddrinfo
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET; // ����������� IPv4
    hints.ai_socktype = SOCK_STREAM; // ������������� TCP-�����
    hints.ai_protocol = IPPROTO_TCP; // ������������� TCP ���������
    hints.ai_flags = AI_PASSIVE; // ������� ��� ���� IP-�����

    // ����������� ��������� ����� � ����, ������� ����� �������������� ��������
    result = getaddrinfo(NULL, "666", &hints, &addrResult);
    if (result != 0) {
        cout << "getaddrinfo failed with error: " << result << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // �������� ������ ��� ������������� �������� �����������
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // �������� ������ � ������ � �����
    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Bind failed, error: " << result << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // ������� ������������ �������� ������� �� �����������
    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        cout << "Listen failed, error: " << result << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // ������� �������� ����������
    ConnectSocket = accept(ListenSocket, NULL, NULL);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Accept failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    closesocket(ListenSocket); // ������ �� ����� ������ ��� �������������

    //�������� � ���������� ������ ������� �������
    do {
        ZeroMemory(recvBuffer, 512); // �������� ����� ������
        result = recv(ConnectSocket, recvBuffer, 512, 0); // �������� ������ �� �������
        if (result > 0) {
            cout << "Received " << result << " bytes" << endl;
            cout << "Received data: " << recvBuffer << endl;

            // �������� ������ ������� �������
            result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
            if (result == SOCKET_ERROR) {
                cout << "Send failed, error: " << result << endl;
                closesocket(ConnectSocket);
                freeaddrinfo(addrResult);
                WSACleanup();
                return 1;
            }
        }
        else if (result == 0) {
            cout << "Connection closing" << endl;
        }
        else {
            cout << "Recv failed, error: " << WSAGetLastError() << endl;
            closesocket(ConnectSocket);
            freeaddrinfo(addrResult);
            WSACleanup();
            return 1;
        }
    } while (result > 0);

    // ���������� ����������, ��� ��� ���������
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "Shutdown failed, error: " << result << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    closesocket(ConnectSocket); // �������� ������������ �����
    freeaddrinfo(addrResult); // ��������� ���������� �������� ����������
    WSACleanup(); // �������� Winsock
    return 0;
}