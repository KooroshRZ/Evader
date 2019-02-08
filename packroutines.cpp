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

		strcpy(pdata.filename, fd.cFileName);

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