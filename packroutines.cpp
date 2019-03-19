#include "packroutines.h"
#include "cipher.h"

const char* packerrors_str[] = {
	"Success",
	"Archive empty; no files to extract",
	"Error in supplied Path",
	"Could not create output archive file",
	"Not a valid packed file",
	"Failed while extracting one of the files",
	"Input archive file failed to open"
};
char key[3] = { 'k', 'e', 'y' };

int packfilesEx(char* path, char* mask, char* archive, packcallbacks_t* pcb) {

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

	long lTemp;

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

	// write signature
	lTemp = 'KCPL';
	fwrite(&lTemp, sizeof(lTemp), 1, fpArchive);

	// write entries count
	lTemp = filesList.size();
	fwrite(&lTemp, sizeof(lTemp), 1, fpArchive);

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
	fclose(fpArchive);

	SetCurrentDirectory(szCurDir);
	return packerrorSuccess;

}

int packfiles(char* path, char* mask, char* archive) {
	return packfilesEx(path, mask, archive, NULL);
}

int unpackfiles(char* archive, char* dest, long startPos) {
	return unpackfilesEx(archive, dest, startPos, NULL);
}

int unpackfilesEx(char* archive, char* dest, long startPos, packcallbacks_t* pcb) {

	FILE* fpArchive = fopen(archive, "rb");

	if (!fpArchive) return packerrorCouldNotOpenArchive;

	long nFiles;

	if (startPos)
		fseek(fpArchive, startPos, SEEK_SET);

	// read signature
	fread(&nFiles, sizeof(nFiles), 1, fpArchive);

	if (nFiles != 'KCPL') return (fclose(fpArchive), packerrorNotAPackedFile);

	// read files entries count
	fread(&nFiles, sizeof(nFiles), 1, fpArchive);

	// no files?
	if (!nFiles) return (fclose(fpArchive), packerrorNoFiles);

	// read all files entries
	std::vector<packdata_t> filesList(nFiles);
	fread(&filesList[0], sizeof(packdata_t), nFiles, fpArchive);

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
		fpOut = fopen(buffer, "wb");

		if (!fpOut)
			return (fclose(fpArchive), packerrorExtractError);

		// how many chunks of Buffer_Size is there is in filesize?
		long size = pdata->filesize;
		long pos = 0;

		while (size > 0)
		{
			long toread = size > sizeof(buffer) ? sizeof(buffer) : size;
			fread(buffer, toread, 1, fpArchive);
			for (int i = 0; i < sizeof(buffer); i++) // for loop for scrambing bits in the string 
				buffer[i] = buffer[i] ^ key[i % sizeof(key) / sizeof(char)]; // scrambling/descrambling string
			fwrite(buffer, toread, 1, fpOut);
			pos += toread;
			size -= toread;
			if (pcb && pcb->fileprogress)
				pcb->fileprogress(pos);
		}
		fclose(fpOut);
		nFiles--;

	}

	fclose(fpArchive);
	return packerrorSuccess;

}


int SfxGetInsertPos(char *filename, long *pos)
{
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
		return packerrorCouldNotOpenArchive;

	IMAGE_DOS_HEADER idh;

	fread((void *)&idh, sizeof(idh), 1, fp);
	fclose(fp);
	*pos = *(long *)&idh.e_res2[0];
	return packerrorSuccess;
}

int SfxSetInsertPos(char *filename, long pos)
{
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
