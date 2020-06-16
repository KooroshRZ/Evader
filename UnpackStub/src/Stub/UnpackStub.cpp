#include "PackRoutines-Stub.h"

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
	
	if (rc != packerrorSuccess)
		printf("%s\n", packerrors_str[rc]);
	else
		printf("\nOperation succeeded!\n");

	return 0;
}