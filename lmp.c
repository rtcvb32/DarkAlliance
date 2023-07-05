#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define BlockSize = 2048	//0x800 noted on all offsets
#define FileEntrySize 64
//#define FES 56
#define BUFFSIZE 16*1024

typedef struct TOCEntry {
	char filename[FileEntrySize - sizeof(int)*2];
	int offset;
	int filesize;
} TOCEntry;

char *buffer = NULL;

/*lmp format; From observation it's as the following
32bit count of enties/files.
Each entry in the TOC is 64 bytes, and uses a block size/padding to 2k.

This means the last entry can never hold anything because it has 62 of 64 bytes avaliable.

The last 8 bytes includes a file offset and the file size. this leaves 56 bytes for filename (including a requred C/null) and 8 bytes for size/location.

With this information we can extract and create our own for modding Dark Alliance games. It also makes a nice little minimal tar archive, though all meta information is lost. To note i haven't seen any directories within, so it's assumed to be flat filenames; Reminds me of the 8bit BASIC computers, and directories were a pain.

Also since there's nothing too odd about all this, we can use a minimum footprint of say 16-32k for everything. */

void extractFile(FILE *rfd, TOCEntry *toc) {
		FILE *wfd = NULL;
		int retval, size;

		//extract file here
		printf("%s, offset %d length %d\n", toc->filename, toc->offset, toc->filesize);
		wfd = fopen(toc->filename, "wb");
		assert(wfd);
		
		size = toc->filesize;
		
		fseek(rfd, toc->offset, SEEK_SET);

		while(size >= BUFFSIZE) {
			retval = fread(buffer, 1, BUFFSIZE, rfd);
			assert(retval == BUFFSIZE);
			retval = fwrite(buffer, 1, BUFFSIZE, wfd);
			assert(retval == BUFFSIZE);
			size -= BUFFSIZE;
		}
		
		if (size) {
			retval = fread(buffer, 1, size, rfd);
			assert(retval == size);
			retval = fwrite(buffer, 1, size, wfd);
			assert(retval == size);
		}
		fclose(wfd);
}

void extractFromArchive(char *filename, char **args, int extractCount) {
	FILE *rfd = fopen(filename, "rb");
	struct TOCEntry *toc = NULL;
	int filecount = 0, matches = 1, retval;
	
	assert(sizeof(TOCEntry) == FileEntrySize);
	
	if (!rfd) {
		printf("Error: Could not open %s for reading\n", filename);
		return;
	}
	
	retval = fread(&filecount, sizeof(int), 1, rfd);
	assert(filecount);

	toc = malloc(FileEntrySize * filecount);
	assert(toc);
	buffer = malloc(BUFFSIZE);
	assert(buffer);
	
	//read TOC
	retval = fread(toc, FileEntrySize, filecount, rfd);
	
	for(int i=0; i < filecount; i++) {
		//check, 0 means auto success and args is empty probably
		if (extractCount > 0) {
			matches = 0;
			for(int i2 = 0; i2 < extractCount; i2++) {
				if (strcmp(args[i2], toc[i].filename) == 0) {
					matches++;
					break;
				}
			}
		}
		
		if (!matches)
			continue;
		
		extractFile(rfd, toc+i);
	}
	
	if (toc)
		free(toc);
	if (buffer){
		free(buffer);
		buffer = NULL;
	}
}


void writeArchive(char **args, int fileCount);


void listTOC(TOCEntry *toc, int entries);


int main(int argc, char **argv) {
	if (argc < 3) {
		printf("%s [xcl] archive filelist...", argv[0]);
		return 0;
	}

	if (strcmp(argv[1], "x") == 0) {
		extractFromArchive(argv[2], argv+3, argc-3);
	} else if  (strcmp(argv[1], "c") == 0) {
		printf("Error: This feature hasn't been implimented yet");
		exit(1);
	} else if  (strcmp(argv[1], "l") == 0) {
		printf("Error: This feature hasn't been implimented yet");
		exit(1);
	} else {
		printf("Error: option not matching of x c or l");
		exit(1);
	}

	return 0;
}
