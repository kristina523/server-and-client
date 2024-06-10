#define WIN32_LEAN_AND_MEAN // Исключите редко используемые заголовки Windows для ускорения процесса сборки
#include <Windows.h>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

int main() {
    WSADATA wsaData; // Хранит данные о реализации сокетов Windows
    ADDRINFO hints; // Подсказки для функции getaddrinfo
    ADDRINFO* addrResult; // Будет хранить адресную информацию результатов
    SOCKET ListenSocket = INVALID_SOCKET; // Разъем для прослушивания входящих подключений
    SOCKET ConnectSocket = INVALID_SOCKET; // Разъем для подключения к клиенту
    char recvBuffer[512]; // Буфер для приема данных

    const char* sendBuffer = "Hello from server"; // Сообщение для отправки клиенту

    // Инициализировать Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed with result: " << result << endl;
        return 1;
    }

    // Настройка структуры подсказок для getaddrinfo
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET; // Используйте IPv4
    hints.ai_socktype = SOCK_STREAM; // Использование TCP-сокет
    hints.ai_protocol = IPPROTO_TCP; // Использование TCP протокола
    hints.ai_flags = AI_PASSIVE; // Введите для меня IP-адрес

    // Определение локальный адрес и порт, который будет использоваться сервером
    result = getaddrinfo(NULL, "666", &hints, &addrResult);
    if (result != 0) {
        cout << "getaddrinfo failed with error: " << result << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Создание сокета для прослушивания входящих подключений
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Привязка сокета к адресу и порту
    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Bind failed, error: " << result << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Начните прослушивать входящие запросы на подключение
    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        cout << "Listen failed, error: " << result << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Принять входящее соединение
    ConnectSocket = accept(ListenSocket, NULL, NULL);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Accept failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    closesocket(ListenSocket); // Больше не нужен разъем для прослушивания

    //Получать и пересылать данные обратно клиенту
    do {
        ZeroMemory(recvBuffer, 512); // Очистите буфер приема
        result = recv(ConnectSocket, recvBuffer, 512, 0); // Получать данные от клиента
        if (result > 0) {
            cout << "Received " << result << " bytes" << endl;
            cout << "Received data: " << recvBuffer << endl;

            // Отправка данных обратно клиенту
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

    // Отключение соединения, так как закончили
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "Shutdown failed, error: " << result << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    closesocket(ConnectSocket); // Очистите подключенный сокет
    freeaddrinfo(addrResult); // Структура бесплатной адресной информации
    WSACleanup(); // Очистите Winsock
    return 0;
}