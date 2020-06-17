#include "PackRoutines-Packer.h"

const char* packerrors_str[] = {
	"Success",
	"Archive empty; no files to extract",
	"Error in supplied Path",
	"Could not create output archive file",
	"Not a valid packed file",
	"Failed while extracting one of the files",
	"Input archive file failed to open",
	"Can not retrieve Encryption key"
};

char* key;
char mainsignature[] = "EVADERPROJECT";

int KEY_SIZE;
int START_ASCII;
int END_ASCII;

int packfilesEx(char* path, char* mask, char* archive, int KEYSIZE, int STARTASCII, int ENDASCII, int exeMethod, char* targetProgram, packcallbacks_t* pcb) {

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
	SIZE_T nfiles;

	KEY_SIZE = KEYSIZE;
	START_ASCII = STARTASCII;
	END_ASCII = ENDASCII;

	// Set a random key
	key = new char[KEY_SIZE]();
	setKey(key, KEY_SIZE);

	// Encrypt signature
	for (int i = 0; i < sizeof(SIG); i++)
		SIG[i] = mainsignature[i] ^ key[i % sizeof(key) / sizeof(char)];
		
	
	// Just somerefactoring for multiple files
	do {

		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) 
			continue;

		memset(&pdata, 0, sizeof(pdata));

		strcpy_s(pdata.filename, fd.cFileName);

		pdata.filesize = fd.nFileSizeLow;

		filesList.push_back(pdata);


	} while (FindNextFile(findHandle, &fd));

	FindClose(findHandle);

	FILE* fpArchive = fopen(archive, "wb");

	if (!fpArchive) return packerrorCannotCreateArchive;

	// Write key length and complexcity to file
	fwrite(&KEY_SIZE, sizeof(KEY_SIZE), 1, fpArchive);
	fwrite(&START_ASCII, sizeof(START_ASCII), 1, fpArchive);
	fwrite(&END_ASCII, sizeof(END_ASCII), 1, fpArchive);

	// Wirte encrypted signature
	fwrite(SIG, sizeof(char), sizeof(SIG), fpArchive);

	// write execution method
	fwrite(&exeMethod, sizeof(int), 1, fpArchive);

	//write targetprogram name
	if (exeMethod > 0)
		fwrite(targetProgram, sizeof(char), 80, fpArchive);
	//printf("%s", targetProgram);
	//Sleep(5000);

	// Write entries count
	nfiles = filesList.size();
	fwrite(&nfiles, sizeof(nfiles), 1, fpArchive);

	// Store files entries (since std::vector stores elements in a linear manner)
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
				buffer[i] = buffer[i] ^ key[i % sizeof(key) / sizeof(char)]; // scrambling string
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

int packfiles(char* path, char* mask, char* archive, int KEYSIZE, int STARTASCII, int ENDASCII, int exeMethod, char* targetProgram) {
	return packfilesEx(path, mask, archive, KEYSIZE, STARTASCII, ENDASCII, exeMethod, targetProgram, NULL);
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

	srand((unsigned int)time(NULL));

	std::cout << "encryption key : ";
	for (int i = 0; i < key_size; ++i) {
		key[i] = char(START_ASCII + rand() % (END_ASCII - START_ASCII + 1));
		std::cout << key[i];
	}

	std::cout << "\n";

}