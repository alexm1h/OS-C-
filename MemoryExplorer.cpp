#include <windows.h>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <conio.h>
using namespace std;

void ShowSystemInfo() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    cout << "\n--- System information ---\n";
    cout << "Page size: " << si.dwPageSize << " bytes\n";
    cout << "Min app address: " << si.lpMinimumApplicationAddress << endl;
    cout << "Max app address: " << si.lpMaximumApplicationAddress << endl;
    cout << "Number of processors: " << si.dwNumberOfProcessors << endl;
    cout << "Processor type: " << si.dwProcessorType << endl;
    cout << "Allocation granularity: " << si.dwAllocationGranularity << " bytes\n";
}

void ShowMemoryStatus() {
    MEMORYSTATUS ms;
    GlobalMemoryStatus(&ms);

    cout << "\n--- Memory status ---\n";
    cout << "Memory load: " << ms.dwMemoryLoad << "%\n";
    cout << "Total physical RAM :" << ms.dwTotalPhys / (1024 * 1024) << "MB\n";
    cout << "Free physical RAM: " << ms.dwAvailPhys / (1024 * 1024) << " MB\n";
    cout << "Total virtual memory: " << ms.dwTotalVirtual / (1024 * 1024) << " MB\n";
    cout << "Free virtual memory: " << ms.dwAvailVirtual / (1024 * 1024) << " MB\n";
}

void DoVirtualQuery() {
    char input[20];
    cout << "\nEnter address (in HEX, e.g. 0x00400000): ";
    cin.ignore(1000, '\n');
    cin.getline(input, 20);

    LPCVOID addr = (LPCVOID)strtoull(input, NULL, 16);
    MEMORY_BASIC_INFORMATION mbi;
    SIZE_T result = VirtualQuery(addr, &mbi, sizeof(mbi));
    if (result == 0) {
        cout << "VirtualQuery error! Code: " << GetLastError() << endl;
        return;
    }
    cout << "\n--- MEMORY STATE AT " << addr << " ---\n";
    cout << "Base address: " << mbi.BaseAddress << endl;
    cout << "Region size: " << mbi.RegionSize << " bytes\n";

    cout << "State: ";
    switch (mbi.State) {
    case MEM_FREE: cout << "FREE"; break;
    case MEM_RESERVE: cout << "RESERVED"; break;
    case MEM_COMMIT: cout << "COMMITTED"; break;
    default: cout << "UNKNOWN";
    }
    cout << endl;

    cout << "Protection: ";
    switch (mbi.Protect) {
    case PAGE_NOACCESS: cout << "NO ACCESS"; break;
    case PAGE_READONLY: cout << "READ ONLY"; break;
    case PAGE_READWRITE: cout << "READ/WRITE"; break;
    case PAGE_EXECUTE: cout << "EXECUTE"; break;
    case PAGE_EXECUTE_READ: cout << "EXECUTE/READ"; break;
    case PAGE_EXECUTE_READWRITE: cout << "FULL ACCESS"; break;
    default: cout << "OTHER (" << mbi.Protect << ")";
    }
    cout << endl;

    cout << "Type: ";
    switch (mbi.Type) {
    case MEM_IMAGE: cout << "IMAGE (DLL/EXE)"; break;
    case MEM_MAPPED: cout << "MAPPED FILE"; break;
    case MEM_PRIVATE: cout << "PRIVATE (VirtualAlloc)"; break;
    default: cout << "UNKNOWN";
    }
    cout << endl;
}

void DoSeparateAlloc() {
    LPVOID pRegion = NULL;
    LPVOID pCommitted = NULL;
    DWORD error;
    char testData[] = "Hello, this is the data omg";
    cout << "\n--- Separate reserve and commit ---\n";

    cout << "1. Reserving address space (40 KB)...\n";
    pRegion = VirtualAlloc(NULL, 4096 * 10, MEM_RESERVE, PAGE_NOACCESS);

    if (pRegion == NULL) {
        error = GetLastError();
        cout << "Reservation failed! Code: " << error << endl;
        return;
    }

    cout << "Reserved at address: " << pRegion << endl;

    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery(pRegion, &mbi, sizeof(mbi));
    cout << "State after reserve: ";
    switch (mbi.State) {
    case MEM_RESERVE: cout << "RESERVED (correct)"; break;
    default: cout << "UNEXPECTED: " << mbi.State;
    }
    cout << endl;

    cout << "\n2. Committing first page (4096 bytes)...";
    pCommitted = VirtualAlloc(pRegion, 4096, MEM_COMMIT, PAGE_READWRITE);
    if (pCommitted == NULL) {
        error = GetLastError();
        cout << "Commit failed! Code: " << error << endl;
        VirtualFree(pRegion, 0, MEM_RELEASE);
        return;
    }
    cout << "Committed at: " << pCommitted << endl;

    VirtualQuery(pRegion, &mbi, sizeof(mbi));
    cout << "State after commit: ";
    switch (mbi.State) {
    case MEM_COMMIT: cout << "COMMITTED (correct)"; break;
    default: cout << "UNEXPECTED: " << mbi.State;
    }
    cout << ", Protection: ";
    switch (mbi.Protect) {
    case PAGE_READWRITE: cout << "READ/WRITE"; break;
    default: cout << "OTHER: " << mbi.Protect;
    }
    cout << endl;

    cout << "\n3. Writing data to committed memory...\n";
    memcpy(pRegion, testData, sizeof(testData));
    cout << "Written: \"" << (char*)pRegion << "\"" << endl;

    cout << "\n4. Freeing memory...\n";
    if (VirtualFree(pRegion, 0, MEM_RELEASE)) {
        cout << "Memory released successfully." << endl;
    }
    else {
        error = GetLastError();
        cout << "Free failed! Code: " << error << endl;
    }
}

void DoSimultaneousAlloc() {
    LPVOID pMemory = NULL;
    DWORD error;
    char testData[] = "Data for simultaneous allocation!";

    cout << "\n--- Simultanious reserve and commit ---\n";
    cout << "Allocating 40 KB with reserce+commit together...\n";
    pMemory = VirtualAlloc(NULL, 4096 * 10, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (pMemory == NULL) {
        error = GetLastError();
        cout << "Allocation failed! Code: " << error << endl;
        return;
    }
    cout << "Allocated at: " << pMemory << endl;
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery(pMemory, &mbi, sizeof(mbi));
    cout << "State: ";
    switch (mbi.State) {
    case MEM_COMMIT: cout << "COMMITTED"; break;
    default: cout << "UNEXPECTED: " << mbi.State;
    }
    cout << ", Protection: ";
    switch (mbi.Protect) {
    case PAGE_READWRITE: cout << "READ/WRITE"; break;
    default: cout << "OTHER: " << mbi.Protect;
    }
    cout << endl;

    cout << "\nWriting data...\n";
    memcpy(pMemory, testData, sizeof(testData));
    cout << "Written: \"" << (char*)pMemory << "\"" << endl;

    cout << "\nFreeing memory...\n";
    if (VirtualFree(pMemory, 0, MEM_RELEASE)) {
        cout << "Memory released successfully." << endl;
    }
    else {
        error = GetLastError();
        cout << "Free failed! Code: " << error << endl;
    }

}

void WriteToAddress() {
    char addrInput[20];
    char dataInput[256];
    LPVOID addr;
    DWORD error;

    cout << "\n--- Write to address ---\n";

    cin.ignore(1000, '\n');
    cout << "Enter address to write to (in HEX): ";
    cin.getline(addrInput, 20);
    addr = (LPVOID)strtoull(addrInput, NULL, 16);

    cout << "Enter data to write: ";
    cin.getline(dataInput, 256);

    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(addr, &mbi, sizeof(mbi)) == 0) {
        cout << "Cannot query address! Code: " << GetLastError() << endl;
        return;
    }

    cout << "\nMemory state at target address:\n";
    cout << "State: ";
    switch (mbi.State) {
    case MEM_COMMIT: cout << "COMMITTED"; break;
    case MEM_RESERVE: cout << "RESERVED (cannot write!)"; break;
    case MEM_FREE: cout << "FREE (cannot write!)"; break;
    default: cout << "UNKNOWN";
    }
    cout << endl;

    cout << "Protection: ";
    switch (mbi.Protect) {
    case PAGE_READWRITE: cout << "READ/WRITE (can write)"; break;
    case PAGE_READONLY: cout << "READ ONLY (cannot write!)"; break;
    case PAGE_NOACCESS: cout << "NO ACCESS (cannot write!)"; break;
    case PAGE_EXECUTE_READWRITE: cout << "FULL ACCESS (can write)"; break;
    default: cout << "OTHER - may not be writable";
    }
    cout << endl;

    if (mbi.State == MEM_COMMIT &&
        (mbi.Protect == PAGE_READWRITE || mbi.Protect == PAGE_EXECUTE_READWRITE)) {
        memcpy(addr, dataInput, strlen(dataInput) + 1);
        cout << "Write successful! Data written: \"" << (char*)addr << "\"" << endl;
    }
    else {
        cout << "Cannot write to this address due to state or protection." << endl;
    }
}

void DoVirtualProtect() {
    LPVOID pMemory = NULL;
    DWORD error;
    DWORD oldProtect;

    cout << "\n--- Virtual protect test ---\n";

    cout << "1. Allocating 1 page with READ/WRITE access...\n";
    pMemory = VirtualAlloc(NULL, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (pMemory == NULL) {
        error = GetLastError();
        cout << "Allocation failed! Code: " << error << endl;
        return;
    }
    cout << "Allocated at: " << pMemory << endl;

    char testData[] = "This is test data";
    memcpy(pMemory, testData, sizeof(testData));
    cout << "Written: \"" << (char*)pMemory << "\"" << endl;

    cout << "\n2. Changing protection to PAGE_READONLY...\n";
    if (!VirtualProtect(pMemory, 4096, PAGE_READONLY, &oldProtect)) {
        error = GetLastError();
        cout << "VirtualProtect failed! Code: " << error << endl;
        VirtualFree(pMemory, 0, MEM_RELEASE);
        return;
    }
    cout << "Protection changed. Old protection was: " << oldProtect << endl;

    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery(pMemory, &mbi, sizeof(mbi));
    cout << "New protection: ";
    switch (mbi.Protect) {
    case PAGE_READONLY: cout << "READ ONLY (correct)"; break;
    default: cout << "UNEXPECTED: " << mbi.Protect;
    }
    cout << endl;

    cout << "\n3. Attempting to write (should fail)...\n";

    if (mbi.Protect == PAGE_READONLY) {
        cout << "Memory is READONLY — cannot write (expected)." << endl;
    }

    cout << "\n4. Reading data (should work)...\n";
    cout << "Data read: \"" << (char*)pMemory << "\"" << endl;

    cout << "\n5. Changing protection back to READ/WRITE...\n";
    VirtualProtect(pMemory, 4096, PAGE_READWRITE, &oldProtect);

    cout << "Writing new data...\n";
    memcpy(pMemory, "New data after restore", 24);
    cout << "Now contains: \"" << (char*)pMemory << "\"" << endl;

    VirtualFree(pMemory, 0, MEM_RELEASE);
    cout << "\nMemory freed." << endl;
}

int main() {
	int choice;
    

	do {
        system("cls");
        cout << "--- VIRTUAL MEMORY EXPLORER ---\n";
        cout << "1. System Info (GetSystemInfo)\n";
        cout << "2. Memory Status (GlobalMemoryStatus)\n";
        cout << "3. Virtual Query (VirtualQuery)\n";
        cout << "4. Separate reserve/commit (VirtualAlloc/VirtualFree)\n";
        cout << "5. Simultaneous reserve/commit\n";
        cout << "6. Write to address\n";
        cout << "7. Change protection (VirtualProtect)\n";
        cout << "8. Exit\n";
        cout << "Choice: ";
        
        cin >> choice;

        switch (choice) {
        case 1: ShowSystemInfo();
            cout << "\nPress any key to continue...";
            _getch();
            system("cls");
            break;
        case 2: ShowMemoryStatus();
            cout << "\nPress any key to continue...";
            _getch();
            system("cls");
            break;
        case 3: DoVirtualQuery();
            cout << "\nPress any key to continue...";
            _getch();
            system("cls");
            break;
        case 4: DoSeparateAlloc();
            cout << "\nPress any key to continue...";
            _getch();
            system("cls");
            break;
        case 5: DoSimultaneousAlloc();
            cout << "\nPress any key to continue...";
            _getch();
            system("cls");
            break;
        case 6: WriteToAddress();
            cout << "\nPress any key to continue...";
            _getch();
            system("cls");
            break;
        case 7: DoVirtualProtect();
            cout << "\nPress any key to continue...";
            _getch();
            system("cls");
            break;
        case 8: cout << "Program finished.\n"; break;
        default: cout << "Invalid choice!\n";
        }
	} while (choice != 8);

    return 0;
}