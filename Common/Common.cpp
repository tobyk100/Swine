#include "stdafx.h"
#include "Common.h"

namespace InjecteeFuncs {
	int MyInjecteeFuncs::isProcess(DWORD processID, TCHAR *process_name) {
		TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
		// Get a handle to the process.
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
		// Get the process name.
		if (NULL != hProcess) {
			HMODULE hMod;
			DWORD cbNeeded;
			if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
				GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
			}
		}
		CloseHandle(hProcess);
		return (_tcscmp(process_name, szProcessName) == 0);
		// Print the process name and identifier.
		// Release the handle to the process.
	}
	int MyInjecteeFuncs::compare(const void * a, const void * b) {
		return (*(int*)a - *(int*)b);
	}
	int MyInjecteeFuncs::getNextProcessID(int current_process_id) {
		// Get the list of process identifiers.
		DWORD aProcesses[1024], cbNeeded, cProcesses;
		unsigned int i;
		if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
			return 1;
		}
		// Calculate how many process identifiers were returned.
		cProcesses = cbNeeded / sizeof(DWORD);
		// Print the name and process identifier for each process.
		std::qsort(aProcesses, cProcesses, sizeof(DWORD), compare);
		int doInjection = (current_process_id == 0);
		for (i = 0; i < cProcesses; i++) {
			if (aProcesses[i] != 0) {
				if (isProcess(aProcesses[i], _T("notepad.exe"))) {
					if (doInjection) {
						return aProcesses[i];
					}
					else if (aProcesses[i] == current_process_id) {
						doInjection = TRUE;
					}
				}
			}
		}
		return 0;
	}
	void MyInjecteeFuncs::inject(int process_id) {
		printf("Injecting process\n");
		HANDLE target_process_handle;
		HANDLE thread;
		int success;
		// TODO (change path, maybe make it relative?)
		char *filename = "C:\\Users\\Toby\\Documents\\Visual Studio 2013\\Projects\\Injectee\\Debug\\Injectee.dll";
		int size = strlen(filename);

		target_process_handle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, process_id);
		if (target_process_handle == NULL) { printf("OpenProcess %d failed.", process_id); exit(0); }

		void *target_filename = VirtualAllocEx(target_process_handle, NULL, size + 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (target_filename == NULL) { printf("VirtualAllocEx failed."); exit(0); }

		int written = WriteProcessMemory(target_process_handle, target_filename, filename, size, NULL);
		if (written == 0) { printf("WriteProcessMemory failed."); exit(0); }

		thread = CreateRemoteThread(target_process_handle, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, target_filename, 0, NULL);
		if (written == NULL) { printf("CreateRemoteThread failed."); exit(0); }

		success = CloseHandle(thread);
		if (!success) { printf("CloseHandle on thread failed."); exit(0); }
		success = CloseHandle(target_process_handle);
		if (!success) { printf("CloseHandle on process failed."); exit(0); }

		return;
	}
}
