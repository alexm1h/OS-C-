#include <iostream>
#include <Windows.h>

using namespace std;

VOID CALLBACK ReadComplete(DWORD dwError, DWORD bytesRead, LPOVERLAPPED lpOverlapped) {
	if (dwError == 0) {
		cout << "Асинхронное чтение завершено. Прочитано: " << bytesRead << " байт" << endl;
	}
	else {
		cout << "Ошибка чтения: " << dwError << endl;
	}
}

int main() {
	SetConsoleOutputCP(CP_UTF8);

	int choice = 0;
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	char buffer[256];

	do {
		cout << "\n---Меню клиента---\n";
		cout << "1. Подключиться к серверу\n";
		cout << "2. Прочитать данные\n";
		cout << "0. Выход\n";
		cin >> choice;

		switch (choice) {
		case 1:
			hPipe = CreateFile(TEXT("\\\\.\\pipe\\myPipe"),
				GENERIC_READ | GENERIC_WRITE | FILE_FLAG_OVERLAPPED,
				0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
			if (hPipe != INVALID_HANDLE_VALUE) {
				cout << "Успешное подключение к серверу.\n" << endl;
			}
			else {
				cout << "Ошибка подключения." << GetLastError() << endl;
			}
			break;
		case 2: {
			if (hPipe == INVALID_HANDLE_VALUE) {
				cout << "Сначала подключитесь к серверу (пункт 1).\n" << endl;
				break;
			}

			OVERLAPPED overlapped = { 0 };

			ZeroMemory(buffer, sizeof(buffer));

			cout << "Запуск асинхронного чтения...\n" << endl;

			ReadFileEx(hPipe, buffer, 255, &overlapped, ReadComplete);

			SleepEx(2000, TRUE);

			buffer[255] = '\0';
			cout << "Получено сообщение: " << buffer << endl;

			break;
		}
		

		case 0: cout << "Выход из программы" << endl; break;
		default: cout << "Неверный пункт меню."; break;

		}

	} while (choice != 0);

	if (hPipe != NULL) {
		CloseHandle(hPipe);
	}

	return 0;
}