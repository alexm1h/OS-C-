#include <windows.h>
#include <iostream>
#include <cmath>

using namespace std;

const long long N = 100000000;
const int STUD_NUM = 431316;
const int BLOCK_SIZE = 10 * STUD_NUM;
const long long TOTAL_BLOCKS = (N + BLOCK_SIZE - 1) / BLOCK_SIZE;

double totalSum = 0.0;
long long currentBlock = 0;
CRITICAL_SECTION cs;

volatile bool* threadSleeping;
HANDLE* threadHandles;
int threadsCount;

DWORD WINAPI ThreadProc(LPVOID param) {
    int idx = (int)(intptr_t)param;
    long long blockNum;
    double localSum = 0.0;
    double x;

    while (true) {
        EnterCriticalSection(&cs);
        blockNum = currentBlock;
        if (blockNum < TOTAL_BLOCKS) {
            currentBlock++;
        }
        LeaveCriticalSection(&cs);

        if (blockNum >= TOTAL_BLOCKS) {
            threadSleeping[idx] = true;
            SuspendThread(threadHandles[idx]);
            continue;
        }

        localSum = 0.0;
        long long start = blockNum * BLOCK_SIZE;
        long long end = start + BLOCK_SIZE;
        if (end > N) end = N;

        for (long long i = start; i < end; i++) {
            x = (i + 0.5) / N;
            localSum += 4.0 / (1.0 + x * x);
        }

        EnterCriticalSection(&cs);
        totalSum += localSum;
        LeaveCriticalSection(&cs);

        threadSleeping[idx] = true;
        SuspendThread(threadHandles[idx]);
    }
    return 0;
}

double RunWithThreads(int numThreads) {
    threadsCount = numThreads;
    totalSum = 0.0;
    currentBlock = 0;

    InitializeCriticalSection(&cs);

    threadSleeping = new bool[numThreads];
    threadHandles = new HANDLE[numThreads];

    for (int i = 0; i < numThreads; i++) {
        threadSleeping[i] = true;
        threadHandles[i] = CreateThread(NULL, 0, ThreadProc, (LPVOID)(intptr_t)i, CREATE_SUSPENDED, NULL);
    }

    LARGE_INTEGER freq, startCount, endCount;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&startCount);

    int initialWake = (numThreads < TOTAL_BLOCKS) ? numThreads : (int)TOTAL_BLOCKS;
    for (int i = 0; i < initialWake; i++) {
        threadSleeping[i] = false;
        ResumeThread(threadHandles[i]);
    }

    while (currentBlock < TOTAL_BLOCKS) {
        for (int i = 0; i < numThreads; i++) {
            if (threadSleeping[i] && currentBlock < TOTAL_BLOCKS) {
                threadSleeping[i] = false;
                ResumeThread(threadHandles[i]);
            }
        }
        Sleep(1);
    }

    for (int i = 0; i < numThreads; i++) {
        while (!threadSleeping[i]) {
            Sleep(1);
        }
        TerminateThread(threadHandles[i], 0);
        CloseHandle(threadHandles[i]);
    }

    QueryPerformanceCounter(&endCount);
    double elapsed = (double)(endCount.QuadPart - startCount.QuadPart) / freq.QuadPart;

    double pi = totalSum / N;
    cout << numThreads << "\t\t" << elapsed << "\t\t" << pi << endl;

    delete[] threadSleeping;
    delete[] threadHandles;
    DeleteCriticalSection(&cs);
    return pi;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);

    int threadsToTest[] = { 1, 2, 4, 8, 12, 16 };
    int numTests = sizeof(threadsToTest) / sizeof(threadsToTest[0]);

    cout << "=== Вычисление числа пи ===" << endl;
    cout << "N = " << N << endl;
    cout << "Размер блока = 10 * " << STUD_NUM << " = " << BLOCK_SIZE << endl;
    cout << "Всего блоков = " << TOTAL_BLOCKS << endl;
    cout << "Потоки\tВремя (сек)\tРезультат" << endl;
    cout << "----------------------------------------" << endl;

    for (int i = 0; i < numTests; i++) {
        RunWithThreads(threadsToTest[i]);
    }

    cout << "----------------------------------------" << endl;
    cout << "Нажмите Enter для выхода...";
    cin.get();

    return 0;
}