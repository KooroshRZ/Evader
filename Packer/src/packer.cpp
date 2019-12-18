#include "packroutines.h"

const char sfxStubFile[] = "unpacksfxstub.exe";

void newfile(char* filename, long size) {
	printf("\xd  --> %s [%d]\n", filename, size);
}

void fileprogress(long pos) {
	printf("%ld                 \xd", pos);
}

int main(int argc, char* argv[]) {

	// display usage
	if (argc < 5) {
		printf("usage:\n"
			" Packer Path Mask ArchiveName.bin [sfx]\n"
			" example:\n"
			" Packer g:\\in g:\\archive.bin\n"
			" Packer g:\\in g:\\archive.bin\n sfx (this will make SFX)");
		return 0;
	}

	int KEY_SIZE;
	int START_ASCII;
	int END_ASCII;

	// set packer's callback info
	packcallbacks_t pcb;
	pcb.fileprogress = fileprogress;
	pcb.newfile = newfile;

	printf("EVADER project v1.0\n");

	// set encryption key size and complexicity
	KEY_SIZE = atoi(argv[4]);
	START_ASCII = atoi(argv[5]);
	END_ASCII = atoi(argv[6]);
	
	// create archive file
	int rc = packfilesEx(argv[1], (char*)("*.exe"), argv[2], KEY_SIZE, START_ASCII, END_ASCII, &pcb);
	printf("               \n");

	if (rc != packerrorSuccess) {
		printf("%s\n", packerrors_str[rc]);
		return 1;
	}

	// need to create SFX?
	if (argc > 3 && (strcmp(argv[3], "sfx") == 0)) {
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
	} else
		printf("Archive created: %s\n", argv[3]);


	return 0;
}