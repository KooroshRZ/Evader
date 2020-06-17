#include "Execution.h"
/*
#ifdef _WIN64
LPCSTR DllPath = "C:\\Users\\k.rajabzadeh\\source\\repos\\Windows-DLL-Injector\\PayloadDLLBuild\\bin\\Debug\\x64\\PayloadDLL.dll";
//LPCSTR DllPath = "C:\\Users\\kourosh\\source\\repos\\WindowsIATHooking\\IATHookingBuild\\bin\\Debug\\x64\\WindowsIATHooking.dll";
#else
LPCSTR DllPath = "C:\\Users\\k.rajabzadeh\\source\\repos\\Windows-DLL-Injector\\PayloadDLLBuild\\bin\\Debug\\Win32\\PayloadDLL.dll";
//LPCSTR DllPath = "C:\\Users\\kourosh\\source\\repos\\WindowsIATHooking\\IATHookingBuild\\bin\\Debug\\Win32\\WindowsIATHooking.dll";
#endif
*/

// variables for Privilege Escalation
HANDLE hToken;
int dwRetVal = RTN_OK;

int initializeInjection(char * targetProgram, LPCSTR DllPath, int InjectionMethod) {

	/*
	printf("escalating Privileges...\n");
	Sleep(2000);
	int epResult = EscalatePrivilege();
	printf("Result of Privilege Escalation : %d\n", epResult);

	if (epResult == RTN_OK)
		printf("Successfully Escalated privileges to SYSTEM level...\n");
	*/


	char szProc[80];
	strcpy(szProc, targetProgram);

	PROCESSENTRY32 PE32{ sizeof(PROCESSENTRY32) };
	PE32.dwSize = sizeof(PE32);

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE) {
		printf("CreateToolhelp32Snapshot failed!");
		printf("LastError : 0x%x\n", GetLastError());
		system("PAUSE");
		return 0;
	}

	DWORD PID = 0;
	BOOL bRet = Process32First(hSnap, &PE32);
	char yn[3];

	while (bRet) {

		//printf("process: %s\n", PE32.szExeFile);
		if (!strcmp((LPCSTR)szProc, PE32.szExeFile)) {

			PID = PE32.th32ProcessID;
			printf("PID found for this process name ---> %d\n", PID);
			printf("Is this OK ? [Input Y to continue with this PID] : ");


			scanf_s("%2s", yn, 2);

			if (!strcmp((LPCSTR)yn, "y") || !strcmp((LPCSTR)yn, "Y"))
				break;

			printf("\n\n");

		}

		bRet = Process32Next(hSnap, &PE32);
	}

	CloseHandle(hSnap);

	printf("Target Program PID: %d\n\n", PID);


	HANDLE hProcess = OpenProcess(
		PROCESS_QUERY_INFORMATION |
		PROCESS_CREATE_THREAD |
		PROCESS_VM_OPERATION |
		PROCESS_VM_WRITE,
		FALSE, PID
	);

	//HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);

	if (!hProcess) {
		printf("Could not open Process for PID %d\n", PID);
		printf("LastError : 0X%x\n", GetLastError());
		system("PAUSE");
		return false;
	}

	// disable SeDebugPrivilege
	/*SetPrivilege(hToken, SE_DEBUG_NAME, FALSE);*/

	// close handles
	CloseHandle(hToken);

	switch (InjectionMethod)
	{
	case 1:
		CreateRemoteThread_Type1(DllPath, hProcess);
		break;
	case 2:
		NtCreateThreadEx_Type2(DllPath, hProcess);
		break;
	case 3:
		QueueUserAPC_Type3(DllPath, hProcess, PID);
		break;
	case 4:
		SetWindowsHookEx_type4(PID, DllPath);
		break;
	case 5:
		RtlCreateUsreThread_type5(hProcess, DllPath);
		break;
	default:
		printf("Choose a valid mathod\n");
		break;
	}


	CloseHandle(hProcess);

	/*if (!TerminateProcess(hProcess, 0xffffffff))
	{
		DisplayError("TerminateProcess");
		dwRetVal = RTN_ERROR;
	}*/

	return 0;
}