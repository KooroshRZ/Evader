/*#include "packroutines.h"

const char sfxStubFile[] = "unpacksfxstub.exe";

void newfile(char* filename, long size) {
	printf("\xd  --> %s [%d]\n", filename, size);
}

void fileprogress(long pos) {
	printf("%ld                 \xd", pos);
}

int main(int argc, char* argv[]) {

	// display usage
	if (argc < 3) {
		printf("usage:\n"
			" Packer Path Mask ArchiveName.bin [sfx]\n"
			" example:\n"
			" Packer c:\\games\\mygame *.* c:\\test.bin\n"
			" Packer c:\\games\\mygame *.* c:\\test.bin\n sfx (this will make SFX)");
		return 0;
	}

	// set packer's callback info
	packcallbacks_t pcb;
	pcb.fileprogress = fileprogress;
	pcb.newfile = newfile;

	printf("Packer v1.0 (c) lallous\n");

	// create archive file
	int rc = packfilesEx(argv[1], argv[2], argv[3], &pcb);
	printf("               \n");

	if (rc != packerrorSuccess) {
		printf("%s\n", packerrors_str[rc]);
		return 1;
	}

	// need to create SFX?
	if (argc > 4 && (strcmp(argv[4], "sfx") == 0)) {
		if (GetFileAttributes(sfxStubFile) == (DWORD)-1)
		{
			printf("SFX stub file not found!");
			return 1;
		}

		// open archive file
		FILE *fpArc = fopen(argv[3], "rb");
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
		strcpy(sfxName, argv[3]);
		strcat(sfxName, ".sfx.exe");
			
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
		DeleteFile(argv[3]);

		printf("SFX created: %s\n", sfxName);
	} else
		printf("Archive created: %s\n", argv[3]);


	return 0;
}*/