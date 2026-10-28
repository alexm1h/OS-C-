#include <windows.h>
#include <iostream>
#include <string>
#include <iomanip>

using namespace std;

void CreateFile() {
	system("CLS");
	cout << "--- Create file ---\n";
	cout << "Enter file path: \n";
	string path;
	cin >> path;
	HANDLE hFile = CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE) {
        cout << "Error! Code: " << GetLastError() << endl;
        return;
    }

	cout << "File create successfully!\n";
	CloseHandle(hFile);
}

void ShowDrivers() {
	system("CLS");
	cout << "--- Drive list ---\n";

	// Способ 1 через GetLogicalDrives
	cout << "Using GetLogicalDrives:\n";
	DWORD drives = GetLogicalDrives();
	for (int i = 0; i < 26; i++) {
		if (drives & (1 << i)) {
			cout << "Drive " << char('A' + i) << ":\\" << endl;
		}
	}

	// Способ 2 через GetLogicalDrivesStringsA
	cout << "Using GetLogicalDriveStrings:\n";
	char buffer[256]; 
	DWORD result = GetLogicalDriveStringsA(sizeof(buffer), buffer);

	if (result == 0) {
		cout << "Error getting drive strings!" << endl;
		return;
	}
	else {
		char* p = buffer;
		while (*p) {
			cout << "Drive " << p << endl;
			p += strlen(p) + 1;
		}
	}
}

void DriveInfo() {
	system("CLS");
	cout << "--- Drive infrormation ---\n";
	char drive[4];
	cout << "Enter drive letter: ";
	cin >> drive[0];
	drive[1] = ':';
	drive[2] = '\\';
	drive[3] = '\0';

	// GetDriveTypeA
	UINT type = GetDriveTypeA(drive);
	cout << "Drive: " << drive << endl;
	cout << "Drive type: ";
	switch (type) {
	case DRIVE_FIXED: cout << "Fixed (HDD/SSD)"; break;
	case DRIVE_REMOVABLE: cout << "Removable (USB)"; break;
	case DRIVE_CDROM: cout << "CD-ROM"; break;
	case DRIVE_REMOTE: cout << "Network drive"; break;
	case DRIVE_RAMDISK: cout << "RAM disk"; break;
	case DRIVE_UNKNOWN: cout << "Unknown"; break;
	case DRIVE_NO_ROOT_DIR: cout << "Drive does not exist"; break;
	}
	cout << endl;

	if (type == DRIVE_NO_ROOT_DIR) return;

	// GetVolumeInformation
	char volumeName[256] = "";
	char fileSystem[256] = "";
	DWORD serialNumber = 0;
	DWORD maxComponentLength = 0;
	DWORD flags = 0;
	struct {
		DWORD flag;
		const char* name;
	} fs_flags[] = {
		{FILE_CASE_SENSITIVE_SEARCH, "Case-sensitive search"},
		{FILE_CASE_PRESERVED_NAMES, "Preserves case in names"},
		{FILE_UNICODE_ON_DISK, "Unicode on disk"},
		{FILE_PERSISTENT_ACLS, "Persistent ACLs"},
		{FILE_FILE_COMPRESSION, "File compression"},
		{FILE_VOLUME_QUOTAS, "Disk quotas"},
		{FILE_SUPPORTS_SPARSE_FILES, "Sparse files"},
		{FILE_SUPPORTS_REPARSE_POINTS, "Reparse points"},
		{FILE_SUPPORTS_REMOTE_STORAGE, "Remote storage"},
		{FILE_READ_ONLY_VOLUME, "Read-only volume"},
		{FILE_SUPPORTS_OBJECT_IDS, "Object IDs"},
		{FILE_SUPPORTS_ENCRYPTION, "Encryption (EFS)"},
		{FILE_NAMED_STREAMS, "Named streams (ADS)"},
		{FILE_SUPPORTS_TRANSACTIONS, "Transactions (TxF)"},
		{FILE_SUPPORTS_HARD_LINKS, "Hard links"},
		{FILE_SUPPORTS_EXTENDED_ATTRIBUTES, "Extended attributes"},
		{FILE_SUPPORTS_OPEN_BY_FILE_ID, "Open by file ID"},
		{FILE_SUPPORTS_USN_JOURNAL, "USN journal (change tracking)"}
	};

	if (GetVolumeInformationA(drive, volumeName, sizeof(volumeName),
		&serialNumber, &maxComponentLength,
		&flags, fileSystem, sizeof(fileSystem))) {
		if (strlen(volumeName) == 0) cout << "Volume label: <no label>" << endl;
		else cout << "Volume label: " << volumeName << endl;
		cout << "Serial number: " << hex << serialNumber << dec << endl;
		cout << "File system: " << fileSystem << endl;
	}

	cout << "\nFile system flags (0x" << hex << flags << dec << "):\n";
	for (int i = 0; i < sizeof(fs_flags) / sizeof(fs_flags[0]); i++) {
		if (flags & fs_flags[i].flag) {
			cout << " + " << fs_flags[i].name << "\n";
		}
		else {
			cout << " - " << fs_flags[i].name << "\n";
		}
	}

	// GetDiskFreeSpace
	ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;

	if (GetDiskFreeSpaceExA(drive, &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
		double totalGB = (double)totalBytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
		double freeGB = (double)totalFreeBytes.QuadPart / (1024.0 * 1024.0 * 1024.0);

		cout << "--- Disk space ---\n";
		cout << "Total space: " << fixed << setprecision(2) << totalGB << " GB" << endl;
		cout << "Free space:  " << fixed << setprecision(2) << freeGB << " GB" << endl;
	}
}

void CreateDirectory() {
	system("CLS");
	cout << "--- Create Directory ---\n";

	string path;
	cout << "Enter path for new directory (Example: C:\\) ";
	cin >> path;

	if (CreateDirectoryA(path.c_str(), NULL)) {
		cout << "Directory created successfully!\n" << endl;
	}
	else {
		DWORD error = GetLastError();
		cout << "Error creating directoty! Code: " << error << endl;

		switch (error) {
		case ERROR_ALREADY_EXISTS:
			cout << "Directory already exists!" << endl;
			break;
		case ERROR_PATH_NOT_FOUND:
			cout << "Path not found! Parent directory doesn't exist." << endl;
			break;
		case ERROR_ACCESS_DENIED:
			cout << "Access denied! (maybe need admin rights)" << endl;
			break;
		}
	}
}

void RemoveDirectory() {
	system("CLS");
	cout << "--- Delete directory ---\n";

	string path;
	cout << "Enter path to directory to delete: ";
	cin >> path;

	if (RemoveDirectoryA(path.c_str())) {
		cout << "Directory deleted successfully!" << endl;
	}
	else {
		DWORD error = GetLastError();
		cout << "✗ Error! Code: " << error << endl;

		switch (error) {
		case ERROR_FILE_NOT_FOUND:
			cout << "Directory not found!" << endl; break;
		case ERROR_DIR_NOT_EMPTY:
			cout << "Directory is not empty!" << endl; break;
		case ERROR_ACCESS_DENIED:
			cout << "Access denied!" << endl; break;
		case ERROR_SHARING_VIOLATION:
			cout << "Directory is in use by another program!" << endl; break;
		default:
			cout << "Unknown error" << endl;
		}
	}
}

void CopyFile() {
	system("CLS");
	cout << "--- Copy file ---\n";

	string source, destination;
	cout << "Enter source file path: \n";
	cin >> source;

	cout << "Enter destination file path: \n ";
	cin >> destination;

	if (source == destination) {
		cout << "Error: Source and destination are the same file!\n" << endl;
		return;
	}

	if (CopyFileA(source.c_str(), destination.c_str(), TRUE)) {
		cout << "File copied successfully!\n";
	}
	else {
		DWORD error = GetLastError();
		cout << "Error! Code: " << error << endl;
		switch (error) {
			case ERROR_FILE_NOT_FOUND:
			cout << "Source file not found!" << endl; break;
			case ERROR_FILE_EXISTS:
			cout << "Destination file already exists!" << endl; break;
			case ERROR_PATH_NOT_FOUND:
			cout << "Destination path not found!" << endl; break;
			case ERROR_ACCESS_DENIED:
			cout << "Access denied! (maybe need admin rights)" << endl; break;
			default: cout << "Unknown error" << endl;
		}
	}
}

void MoveFile() {
	system("CLS");
	cout << "--- Move file ---\n";

	string source, destination;
	cout << "Enter source file path: \n";
	cin >> source;

	cout << "Enter destination file path: \n ";
	cin >> destination;

	if (source == destination) {
		cout << "Error: Source and destination are the same file!" << endl;
		return;
	}

	if (MoveFileExA(source.c_str(), destination.c_str(), MOVEFILE_REPLACE_EXISTING)) {
		cout << "File moved successfully!" << endl;
		cout << "Original file deleted, new file at: " << destination << endl;
	}
	else {
		DWORD error = GetLastError();
		cout << "Error! Code: " << error << endl;

		switch (error) {
			case ERROR_FILE_NOT_FOUND:
			cout << "Source file not found!" << endl; break;
			case ERROR_ALREADY_EXISTS:
			cout << "Destination already exists!" << endl; break;
			case ERROR_PATH_NOT_FOUND:
			cout << "Destination path not found!" << endl; break;
			case ERROR_ACCESS_DENIED:
			cout << "Access denied! (maybe need admin rights)" << endl; break;
			default: cout << "Unknown error" << endl;
		}
	}
}

void FileAttributes() {
	system("CLS");
	cout << "--- File attributes ---\n";

	string path;
	cout << "Enter file path: \n";
	cin >> path;

	DWORD attr = GetFileAttributesA(path.c_str());

	if (attr == INVALID_FILE_ATTRIBUTES) {
		cout << "Error: File not found or cannot access!" << endl;
		return;
	}

	cout << "\nCurrent attributes:\n";
	cout << (attr & FILE_ATTRIBUTE_READONLY ? "Read-only\n" : "Not read-only\n");
	cout << (attr & FILE_ATTRIBUTE_HIDDEN ? "Hidden\n" : "Not hidden\n");
	cout << (attr & FILE_ATTRIBUTE_SYSTEM ? "System\n" : "Not system\n");
	cout << (attr & FILE_ATTRIBUTE_ARCHIVE ? "Archive\n" : "Not archive\n");
	cout << (attr & FILE_ATTRIBUTE_DIRECTORY ? "Is directory\n" : "Is file\n");

	HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE) {
		cout << "Cannot open file for detailed info. Code: " << GetLastError() << endl;
		return;
	}

	cout << "--- GetFileInformationByHandle ---\n";
	BY_HANDLE_FILE_INFORMATION info;

	if (GetFileInformationByHandle(hFile, &info)) {
		cout << "File size: " << info.nFileSizeHigh << " * 2^32 + "
			<< info.nFileSizeLow << " bytes" << endl;

		ULARGE_INTEGER fileSize;
		fileSize.HighPart = info.nFileSizeHigh;
		fileSize.LowPart = info.nFileSizeLow;
		cout << "Total size: " << fileSize.QuadPart << " bytes" << endl;

		cout << "Number of links: " << info.nNumberOfLinks << endl;
		cout << "Index: " << info.nFileIndexHigh << ":" << info.nFileIndexLow << endl;
	}

	cout << "\n--- GetFileTime ---\n";
	FILETIME createTime, accessTime, writeTime;

	if (GetFileTime(hFile, &createTime, &accessTime, &writeTime)) {
		SYSTEMTIME stCreate, stAccess, stWrite;

		FileTimeToSystemTime(&createTime, &stCreate);
		FileTimeToSystemTime(&accessTime, &stAccess);
		FileTimeToSystemTime(&writeTime, &stWrite);

		cout << "Created:  "
			<< stCreate.wDay << "." << stCreate.wMonth << "." << stCreate.wYear
			<< " " << stCreate.wHour << ":" << stCreate.wMinute << endl;

		cout << "Last access: "
			<< stAccess.wDay << "." << stAccess.wMonth << "." << stAccess.wYear
			<< " " << stAccess.wHour << ":" << stAccess.wMinute << endl;

		cout << "Last write: "
			<< stWrite.wDay << "." << stWrite.wMonth << "." << stWrite.wYear
			<< " " << stWrite.wHour << ":" << stWrite.wMinute << endl;
	}

	cout << "Do you want to set new file time? (y/n): ";
	char answer;
	cin >> answer;

	if (answer == 'y' || answer == 'Y') {
		SYSTEMTIME st;
		GetSystemTime(&st);

		FILETIME ft;
		SystemTimeToFileTime(&st, &ft);

		if (SetFileTime(hFile, &ft, &ft, &ft)) {
			cout << "File time updated to current system time!" << endl;
		}
		else {
			cout << "Error setting file time! Code: " << GetLastError() << endl;
		}
	}

	cout << "Do you want to change attributes? (y/n): ";
	cin >> answer;

	if (answer == 'y' || answer == 'Y') {
		DWORD newAttr = 0;
		char choice;

		cout << "Read-only? (y/n): ";
		cin >> choice;
		if (choice == 'y') newAttr |= FILE_ATTRIBUTE_READONLY;

		cout << "Hidden? (y/n): ";
		cin >> choice;
		if (choice == 'y') newAttr |= FILE_ATTRIBUTE_HIDDEN;

		CloseHandle(hFile);

		if (SetFileAttributesA(path.c_str(), newAttr)) {
			cout << "Attributes changed successfully!" << endl;
		}
		else {
			cout << "Error changing attributes! Code: " << GetLastError() << endl;
		}

		return;
	}

	CloseHandle(hFile);

}

int main() {
	int choice;

	do {
		system("CLS");
		cout << "--- Main menu ---\n";
		cout << "1. Show all drives\n";
		cout << "2. Drive information\n";
		cout << "3. Create directory\n";
		cout << "4. Delete directory\n";
		cout << "5. Create file\n";
		cout << "6. Copy file\n";
		cout << "7. Move file\n";
		cout << "8. File attributes\n";
		cout << "0. Exit\n";
		cout << "Choose option: ";
		cin >> choice;

		switch (choice) {
			case 0: cout << "Goodbye!\n"; break;
			case 1: ShowDrivers(); break;
			case 2: DriveInfo(); break;
			case 3: CreateDirectory(); break;
			case 4: RemoveDirectory(); break;
			case 5: CreateFile(); break;
			case 6: CopyFile(); break;
			case 7: MoveFile(); break;
			case 8: FileAttributes(); break;
			default: cout << "Invalid ioption, try again.";
		}

		if (choice != 0) {
			cout << "\nPress Enter to continue...";
			cin.ignore();
			cin.get();
		}

    } while (choice != 0);

	return 0;
}