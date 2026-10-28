#include <iostream>
#include <Windows.h>

using namespace std;

int main() {
	SetConsoleOutputCP(CP_UTF8);

	int choice = 0;
	HANDLE hPipe = NULL;
	char message[256];

	do {
		cout << "\n---Меню сервера---\n";
		cout << "1. Создать канал\n";
		cout << "2. Подключить клиента\n";
		cout << "3. Отправить данные\n";
		cout << "4. Отключить клиента\n";
		cout << "0. Выход\n";
		cin >> choice;

		switch (choice) {
			case 1: 
				if (hPipe != NULL) {
					CloseHandle(hPipe);
				}
				hPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\myPipe"), PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE, 1, 256, 256, 0, NULL);
				if (hPipe != INVALID_HANDLE_VALUE) {
					cout << "Канал создан успешно.";
				}
				else {
					cout << "Ошибка создания канала." << GetLastError() << endl;
					break;
				}
				break;
			case 2:
				 if (hPipe == NULL) {
                    cout << "Ошибка: сначала создайте канал (пункт 1)!" << endl;
                    break;
                }
				cout << "Ожидание подключения клиента...\n";
				if (ConnectNamedPipe(hPipe, NULL)) {
					cout << "Клиент подключился успешно.";
				}
				else {
					if (GetLastError() == ERROR_PIPE_CONNECTED) {
						cout << "Клиент уже был подключен." << endl;
					}
					else {
						cout << "Ошибка подключения" << GetLastError() << endl;
					}
				}
				cout << "Клиент подключён успешно.";
				break;
			case 3: {
				if (hPipe == NULL) {
					cout << "Ошибка: сначала создайте канал (пункт 1)!" << endl;
					break;
				}
				cout << "Введите сообщение: \n";
				cin.ignore();
				cin.getline(message, 256);

				HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
				OVERLAPPED overlapped = { 0 };
				overlapped.hEvent = hEvent;

				DWORD bytesWritten;
				BOOL result = WriteFile(hPipe, message, strlen(message), &bytesWritten, &overlapped);

				if (!result && GetLastError() == ERROR_IO_PENDING) {
					cout << "Идёт асинхронная отправка..." << endl;
					WaitForSingleObject(hEvent, INFINITE);
					cout << "Отправлено успешно.";
				}
				else if (result) {
					cout << "Отправлено: " << bytesWritten << " байт" << endl;
				}
				else {
					cout << "Ошибка отправки. Код: " << GetLastError() << endl;
				}
				CloseHandle(hEvent);
				break;
			}
			case 4:
				if (hPipe == NULL) {
					cout << "Сначала создайте канал (пункт 1)." << endl;
					break;
				}
				DisconnectNamedPipe(hPipe);
				cout << "Клиент успешно отключен." << endl;
				break;
			case 0: cout << "Выход из программы" << endl; break;
			default: cout << "Неверный пункт меню."; break;
		}

	} while (choice != 0);

	if (hPipe != NULL) {
		CloseHandle(hPipe);
	}

	return 0;
}