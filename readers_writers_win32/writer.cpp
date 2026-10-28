#include <iostream>
#include <fstream>
#include <Windows.h>
#include <time.h>
#include <stdio.h>
#pragma comment(lib, "winmm.lib")

using namespace std;

int main(int argc, char* argv[]) {
	const int numPage = 14;
	const int pageSize = 4096;
	const int bufferSize = numPage * pageSize;
	const int numOps = 5;
	SetConsoleOutputCP(CP_UTF8);

	int writeId = (argc > 1) ? atoi(argv[1]) : 0;
	srand((unsigned int)(time(NULL) + writeId));

	HANDLE hFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "SharedBuffer");
	if (hFile == NULL) {
		cout << "Ошибка открытия проецируемого файла" << GetLastError() << endl;
		return 1;
	}

	LPVOID buffer = MapViewOfFile(hFile, FILE_MAP_ALL_ACCESS, 0, 0, bufferSize);
	if (buffer == NULL) {
		cout << "Ошибка MapViewOfFile: " << GetLastError() << endl;
		CloseHandle(hFile);
		return 1;
	}

	cout << "Буфер отображён по адресу: " << buffer << endl;
	if (VirtualLock(buffer, bufferSize)) {
		cout << "Память заблокирована в оперативной памяти" << endl;
	}
	else {
		cout << "Предупреждение VirtualLock: " << GetLastError() << endl;
	}

	HANDLE wSem[numPage];
	HANDLE rSem[numPage];
	char semName[32];

	for (int i = 0; i < numPage; i++) {
		sprintf_s(semName, "wSem_%d", i);
		wSem[i] = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, semName);
		if (wSem[i] == NULL) {
			cout << "Ошибка открытия " << semName << ": " << GetLastError() << endl;
		}
	}

	for (int i = 0; i < numPage; i++) {
		sprintf_s(semName, "rSem_%d", i);
		rSem[i] = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, semName);
		if (rSem[i] == NULL) {
			cout << "Ошибка открытия " << semName << ": " << GetLastError() << endl;
		}
	}

	char logName[64];
	sprintf_s(logName, "writer_%d.log", writeId);
	ofstream log(logName);
	log << "Писатель " << writeId << " начал работу \n";

	for (int i = 0; i < numOps; i++) {
		log << timeGetTime() << " ожидание\n";
		cout << "[" << i + 1 << "/" << numOps << "] Ожидание свободной страницы..." << endl;

		DWORD result = WaitForMultipleObjects(numPage, wSem, FALSE, INFINITE);
		int pageIndex = result - WAIT_OBJECT_0;

		Sleep(500 + rand() % 1000);

		char* pageAddress = (char*)buffer + (pageIndex * pageSize);
		pageAddress[0] = 'A' + writeId;
		pageAddress[1] = '\0';

		log << timeGetTime() << " запись страница " << pageIndex << "\n";
		cout << "[" << i + 1 << "/" << numOps << "] Запись в страницу " << pageIndex
			<< " (буква '" << (char)('A' + writeId) << "')" << endl;

		ReleaseSemaphore(rSem[pageIndex], 1, NULL);
		log << timeGetTime() << " освобождение\n";
		cout << "[" << i + 1 << "/" << numOps << "] Страница " <<
			pageIndex << " открыта для чтения" << endl;
	}

	log << "=== Писатель " << writeId << " завершил работу ===\n";
	log.close();

	VirtualUnlock(buffer, bufferSize);
	UnmapViewOfFile(buffer);
	CloseHandle(hFile);
	for (int i = 0; i < numPage; i++) {
		CloseHandle(wSem[i]);
		CloseHandle(rSem[i]);
	}

	cout << "Писатель " << writeId << " завершил работу." << endl;
	system("pause");
	return 0;
}