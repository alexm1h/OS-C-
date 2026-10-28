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

	int readId = (argc > 1) ? atoi(argv[1]) : 0;
	srand((unsigned int)(time(NULL) + readId));

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

	HANDLE rSem[numPage];
	HANDLE wSem[numPage];
	char semName[32];

	for (int i = 0; i < numPage; i++) {
		sprintf_s(semName, "rSem_%d", i);
		rSem[i] = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, semName);
		if (rSem[i] == NULL) {
			cout << "Ошибка открытия " << semName << ": " << GetLastError() << endl;
		}
	}

	for (int i = 0; i < numPage; i++) {
		sprintf_s(semName, "wSem_%d", i);
		wSem[i] = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, semName);
		if (wSem[i] == NULL) {
			cout << "Ошибка открытия " << semName << ": " << GetLastError() << endl;
		}
	}

	char logName[64];
	sprintf_s(logName, "reader_%d.log", readId);
	ofstream log(logName);
	log << "Читатель " << readId << " начал свою работу \n";

	for (int i = 0; i < numOps; i++) {
		log << timeGetTime() << " ожидание\n";
		cout << "[" << i + 1 << "/" << numOps << "] Ожидание данных для чтения..." << endl;

		DWORD result = WaitForMultipleObjects(numPage, rSem, FALSE, INFINITE);
		int pageIndex = result - WAIT_OBJECT_0;

		Sleep(500 + rand() % 1000);

		char* pageAddress = (char*)buffer + (pageIndex * pageSize);
		char letter = pageAddress[0];

		log << timeGetTime() << " чтение страница " << pageIndex << "\n";
		cout << "[" << i + 1 << "/" << numOps << "] Чтение страницы " << pageIndex
			<< " (буква '" << letter << "')" << endl;

		ReleaseSemaphore(wSem[pageIndex], 1, NULL);
		log << timeGetTime() << " освобождение\n";
		cout << "[" << i + 1 << "/" << numOps << "] Страница " <<
			pageIndex << " возвращена писателям" << endl;
	}

	log << "=== Читатель " << readId << " завершил работу ===\n";
	log.close();

	VirtualUnlock(buffer, bufferSize);
	UnmapViewOfFile(buffer);
	CloseHandle(hFile);
	for (int i = 0; i < numPage; i++) {
		CloseHandle(rSem[i]);
		CloseHandle(wSem[i]);
	}

	cout << "Читатель " << readId << " завершил работу." << endl;
	system("pause");
	return 0;
}