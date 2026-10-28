#include <iostream>
#include <Windows.h>
#include <stdio.h>

using namespace std;

void StartReader(int id) {
	STARTUPINFOA si = { sizeof(si) };
	PROCESS_INFORMATION pi;

	char cmdLine[64];
	sprintf_s(cmdLine, "reader.exe %d", id);

	if (CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		cout << "Читатель " << id << " запущен (PID: " << pi.dwProcessId << ")" << endl;
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else {
		cout << "Ошибка запуска читателя " << id << ": " << GetLastError() << endl;
	}
}

void StartWriter(int id) {
	STARTUPINFOA si = { sizeof(si) };
	PROCESS_INFORMATION pi;

	char cmdLine[64];
	sprintf_s(cmdLine, "writer.exe %d", id);

	if (CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		cout << "Писатель " << id << " запущен (PID: " << pi.dwProcessId << ")" << endl;
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else {
		cout << "Ошибка запуска писателя " << id << ": " << GetLastError() << endl;
	}
}


int main() {
	const int numPage = 14;
	const int pageSize = 4096;
	const int bufferSize = numPage * pageSize;
	SetConsoleOutputCP(CP_UTF8);

	HANDLE hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, bufferSize, "SharedBuffer");
	if (hMapFile == NULL) {
		cout << "Ошибка CreateFileMappingA: " << GetLastError() << endl;
		return 1;
	}
	cout << "Проецируемый файл создан. Размер: " << bufferSize << " байт" << endl;


	LPVOID buffer = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, bufferSize);
	if (buffer == NULL) {
		cout << "Ошибка MapViewOfFile: " << GetLastError() << endl;
		CloseHandle(hMapFile);
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
	char semName[32];

	for (int i = 0; i < numPage; i++) {
		sprintf_s(semName, "wSem_%d", i);
		wSem[i] = CreateSemaphoreA(NULL, 1, 1, semName);

		if (wSem[i] == NULL) {
			cout << "Ошибка создания " << semName << ": " << GetLastError() << endl;
		}
	}
	cout << "Создано " << numPage << " семафоров писателей (wSem_0.." << numPage - 1 << ")" << endl;

	HANDLE rSem[numPage];

	for (int i = 0; i < numPage; i++) {
		sprintf_s(semName, "rSem_%d", i);
		rSem[i] = CreateSemaphoreA(NULL, 0, 1, semName);

		if (rSem[i] == NULL) {
			cout << "Ошибка создания " << semName << ": " << GetLastError() << endl;
		}
	}
	cout << "Создано " << numPage << " семафоров читателей (rSem_0.." << numPage - 1 << ")" << endl;

	int numReaders = 7;
	int numWriters = 7;

	cout << "\nЗапуск процессов... " << endl;
	for (int i = 0; i < numReaders; i++) {
		StartReader(i);
	}
	for (int i = 0; i < numWriters; i++) {
		StartWriter(i);
	}

	cout << "\nВсе процессы запущены. Ожидание завершения..." << endl;
	cout << "(нажми любую клавишу после завершения всех процессов)" << endl;
	system("pause");

	VirtualUnlock(buffer, bufferSize);
	UnmapViewOfFile(buffer);
	CloseHandle(hMapFile);
	for (int i = 0; i < numPage; i++) {
		CloseHandle(wSem[i]);
		CloseHandle(rSem[i]);
	}

	return 0;
}