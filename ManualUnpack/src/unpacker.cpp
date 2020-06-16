/*
	Ignore this section
	just for debug
*/

#include "packroutines.h"

void newfile_cb(char *fn, long size)
{
	printf("\n --> %s [%ld] ...", fn, size);
}

int main(int argc, char *argv[])
{
	packcallbacks_t pcb;

	pcb.fileprogress = NULL;
	pcb.newfile = newfile_cb;

	if (argc < 2)
	{
		printf("Usage:\nUnpacker PackedFile.bin OutPutDir");
		return 0;
	}

	char outDir[MAX_PATH];
	strcpy(outDir, argv[2]);
	if (outDir[strlen(outDir) - 1] != '\\')
		strcat(outDir, "\\");

	printf("Unpacker of Evader v1.0\n");

	std::vector<packdata_t> fileslist(long(10));

	int rc = unpackfilesEx(argv[1], outDir, fileslist, 0, &pcb);
	if (rc != packerrorSuccess)
		printf("%s\n", packerrors_str[rc]);
	else
		printf("\nOperation succeeded!\n");

	return 0;
}