// IAT Hooking, Barak Gonen 2020
#include "stdio.h"
#include "windows.h"
//idan maman �
#define MAX 20
#define FILENAME "C:\\Users\\idang\\Downloads\\cool.txt"
int hook(PCSTR func_to_hook, PCSTR DLL_to_hook,
    DWORD new_func_address);
HANDLE __stdcall  show_msg(LPCSTR lpFileName,
    DWORD                 dwDesiredAccess,
    DWORD                 dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD                 dwCreationDisposition,
    DWORD                 dwFlagsAndAttributes,
    HANDLE                hTemplateFile
);
using LPCF = HANDLE(__stdcall*)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
DWORD_PTR  saved_hooked_func_addr;



int main()
{

    PCSTR func_to_hook = "CreateFileA";
    PCSTR DLL_to_hook = "KERNEL32.dll";

    DWORD new_func_address = (DWORD)&show_msg;
    HANDLE hFile;
    hook(func_to_hook, DLL_to_hook, new_func_address);




    // open the file for reading
    hFile = CreateFileA(FILENAME, // file name
        GENERIC_READ, // open for read
        0, // do not share
        NULL, // default security
        OPEN_EXISTING, // open only if exists
        FILE_ATTRIBUTE_NORMAL, // normal file
        NULL); // no attr. template
    char buf[300];
    DWORD red;
    ReadFile(hFile, buf, 300, &red, NULL);
    buf[red] = 0;
    puts(buf);

    // if file was not opened, print error code and return
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        printf("Error code: %d\n", error);
        return 0;
    }
    // read some bytes and print them
    CHAR buffer[MAX];
    DWORD num;
    LPDWORD numread = &num;
    BOOL result = ReadFile(hFile, // handle to open file
        buffer, // pointer to buffer
        MAX - 1, // bytes to read
        numread, // return value - bytes read
        NULL); // overlapped
    buffer[*numread] = 0;
    printf("%s\n", buffer);

    // close file
    CloseHandle(hFile);
    return 0;
};
int  hook(PCSTR func_to_hook, PCSTR DLL_to_hook,
    DWORD new_func_address) {
    PIMAGE_DOS_HEADER dosHeader;
    PIMAGE_NT_HEADERS NTHeader;
    PIMAGE_OPTIONAL_HEADER32 optionalHeader;
    IMAGE_DATA_DIRECTORY importDirectory;
    DWORD descriptorStartRVA;
    PIMAGE_IMPORT_DESCRIPTOR importDescriptor;
    int index;
    // Get base address of currently running .exe
    DWORD baseAddress = (DWORD)GetModuleHandle(NULL);
    // Get the import directory address
    dosHeader = (PIMAGE_DOS_HEADER)(baseAddress);
    if (((*dosHeader).e_magic) != IMAGE_DOS_SIGNATURE) {
        return 0;
    }
    NTHeader = (PIMAGE_NT_HEADERS)(baseAddress + (*dosHeader).e_lfanew);
    if (((*NTHeader).Signature) != IMAGE_NT_SIGNATURE) {
        return 0;
    }
    optionalHeader = &(*NTHeader).OptionalHeader;
    if (((*optionalHeader).Magic) != 0x10B) {
        return 0;

    }
    importDirectory =
        (*optionalHeader).DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    descriptorStartRVA = importDirectory.VirtualAddress;
    importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(descriptorStartRVA + baseAddress);
    index = 0;
    char* DLL_name;
    // Look for the DLL which includes the function for hooking
    while ((*(importDescriptor + index)).Characteristics != 0) {
        DLL_name = (char*)(baseAddress + importDescriptor->Name);
        printf("DLL name: %s\n", DLL_name);
        if (!strcmp(DLL_to_hook, DLL_name))
	  break;
        index++;
    }
    // exit if the DLL is not found in import directory
    if (importDescriptor[index].Characteristics == 0) {
        printf("DLL was not found");
        return 0;
    }
    // Search for requested function in the DLL
    PIMAGE_THUNK_DATA thunkILT; // Import Lookup Table- names
    PIMAGE_THUNK_DATA thunkIAT; // Import Address Table- addresses
    PIMAGE_IMPORT_BY_NAME nameData;
    thunkILT = (PIMAGE_THUNK_DATA)(baseAddress +
        importDescriptor[index].OriginalFirstThunk);
    thunkIAT = (PIMAGE_THUNK_DATA)(baseAddress + importDescriptor[index].FirstThunk);
    if ((thunkIAT == NULL) or (thunkILT == NULL)) {
        return 0;
    }
    while (((*thunkILT).u1.AddressOfData != 0) &
        (!((*thunkILT).u1.Ordinal & IMAGE_ORDINAL_FLAG))) {
        nameData = (PIMAGE_IMPORT_BY_NAME)(baseAddress +
	  (*thunkILT).u1.AddressOfData);
        if (!strcmp(func_to_hook, (char*)(*nameData).Name))
	  break;
        thunkIAT++;
        thunkILT++;
    }
    // Hook IAT: Write over function pointer
    DWORD dwOld = NULL;
    saved_hooked_func_addr = (*thunkIAT).u1.Function;
    VirtualProtect((LPVOID) & ((*thunkIAT).u1.Function),
        sizeof(DWORD), PAGE_READWRITE, &dwOld);
    (*thunkIAT).u1.Function = new_func_address;
    //VirtualProtect((LPVOID) & ((*thunkIAT).u1.Function),
    //  sizeof(DWORD), dwOld, NULL);

    return 1;
};
HANDLE __stdcall  show_msg(
    LPCSTR          lpFileName,
    DWORD                 dwDesiredAccess,
    DWORD                 dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD                 dwCreationDisposition,
    DWORD                 dwFlagsAndAttributes,
    HANDLE                hTemplateFile
) {
    MessageBoxA(0, "Hooked", "I Love Assembly", 0);
    LPCF pr = (LPCF)saved_hooked_func_addr;
    return pr(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}