#include "packroutines.h"

void newfile_cb(char *fn, long size)
{
	printf("\n --> %s [%ld] ...", fn, size);
}

int main(int argc, char *argv[])
{
	char outDirectory[MAX_PATH];
	
	char arcFn[MAX_PATH];
	long pos;
	
#ifdef MYDEBUG
	//strcpy(arcFn, "e:\\sources\\packer\\arc1.bin.sfx.exe");
	//pos = 0xc000;
#else
	// get self file name
	GetModuleFileName(NULL, arcFn, sizeof(arcFn));
	// get position of archive data inside yourself
	SfxGetInsertPos(arcFn, &pos);
	//printf("going for: %s @ %lX\n", arcFn, pos);
#endif

	// setup callbacks
	packcallbacks_t pcb;
	pcb.fileprogress = NULL;
	pcb.newfile = newfile_cb;

	// fix output directory
	strcpy(outDirectory, "./");
	if (outDirectory[strlen(outDirectory) - 1] != '\\')
		strcat(outDirectory, "\\");

	
	std::vector<packdata_t> filesList(long (10));

	// start unpacking
	int rc = unpackfilesEx(arcFn, outDirectory, filesList, pos, &pcb);
	
	/*for (unsigned int i = 0; i < filesList.size(); i++) {
		std::cout << filesList[i].filename << "\n";
		if (strstr(filesList[i].filename, "exe")) startup((LPCTSTR) filesList[i].filename);
		Sleep(1500);
	}*/

	
	if (rc != packerrorSuccess)
		printf("%s\n", packerrors_str[rc]);
	else
		printf("\nOperation succeeded!\n");

	return 0;
}

VOID startup(LPCTSTR lpApplicationName)
{
	// additional information
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	// set the size of the structures
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// start the program up
	CreateProcess(lpApplicationName,   // the path
		NULL,			// Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
	);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	
}