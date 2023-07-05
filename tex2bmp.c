#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(1)

/* from ponytail
  6000600014400000380000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
  |++||++|????    |++|???->? unknown
  |++|height      header length
  width

  Colors:
  00 00 00 FE
  R  G  B  ?
  
  For BMP need BGR0
  
6000600014400000380000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
000000FE  - black

If we need to deal with widthline%4=0 we might also invert the height so it's the right direction. Though there's no data if there's some odd texture sizes or how it handles it (yet).

There's also no data if it's all 256 color palette or if there's some other ones. We'll just assume 256 for now.*/

typedef struct BMP {
	char HF[2];
	int filesize;
	char reserved[4];
	int pixelOffset;
} BMP;


typedef struct DIB {
	int DIBsize;
	unsigned short width,
		height,
		planes,
		bpp;
	
} DIB;

typedef struct BITMAPINFOHEADER {
	int headersize,
		width,
		height;
	unsigned short planes,
		bpp;
	int compression,
		imagesize,
		horizontalres,
		verticalres,
		colorsinpalette,
		importantcolorsused;
} BITMAPINFOHEADER;

typedef struct TEX {
	//60 00 60 00 14 40 00 00 38 00 00 00 00 00 00 00
	//0 (2 lines)
	//00 00 00 00 00 00 00 00 : palette start 00 00 00 fe 00 02 02 fe
	unsigned short width,
		height,
		sig,	//assumed signature, could be flags...
		res1,
		headersize,
		res2[23];
} TEX;


//two conversion functions... 
void tex2bmp(char *filename) {
	char newfile[128];
	int retval;
	
	//step 0, replace extension...
	memcpy(&newfile[0], filename, strlen(filename)+1);	//+1 should include a null.
	memcpy(&newfile[0] + strlen(filename)-3,"bmp",3);	//backtrack 3 letters
	
	//step 1, check size is 56 bytes and 14,40 sig?, that's our only real known value at this point
	
	FILE *rfd = fopen(filename, "rb");
	assert(rfd);
	TEX tex;
	
	retval = fread(&tex, 1, sizeof(TEX), rfd);
	assert(retval == sizeof(TEX));
	
	//check with known data?
/*	if (tex.sig != 0x4014 || tex.headersize != sizeof(TEX)) {
		printf("Error, %s file not matching expected values\n",filename);
		fclose(rfd);
		return;
	}
	*/
	//step 2, set up BMP header
	BMP bmp = {"BM", -1, "", sizeof(BMP)+sizeof(BITMAPINFOHEADER)+ 256*4};
	BITMAPINFOHEADER bmpinfo40 = {sizeof(BITMAPINFOHEADER), tex.width, tex.height,
		1,	//planes
		8,	//bpp
		0, 0, 1, 1, 256, 0};
	
	//filesize is wrong....
	bmp.filesize = sizeof(BMP)+sizeof(BITMAPINFOHEADER)+256*4+tex.width*tex.height;
	
	int widpad = 4 - tex.width%4;
	if (widpad==4)
		widpad = 0;

	bmp.filesize += tex.height * widpad;
	
	
	//assert((tex.width % 4) == 0);	//just makes sure we don't have to do any funky other work, if so we'll fix it.
	//otherwise bmp demands every width is divisible by 4, eating 0-4 extra bytes. But that depends on the texture.
	
	//step 3, copy palette
	unsigned char palette[256*4];	//magic numbers.....
	
	retval = fread(&palette[0], 1, 256*4,rfd);
	assert(retval == 256*4);
	
	//step 3b,modify palette order
	//Need to swap Red/Blue, 4th value FE can be ignored?
	int t;
	for(int i=0; i<256; i++) {
		t=palette[i*4 + 0];
		palette[i*4 + 0] = palette[i*4 + 2];
		palette[i*4 + 2] = t;;
	}

	FILE *wfd = fopen(newfile, "wb+");
	assert(wfd);
	
	//step 4, output header(s) and palette
	retval = fwrite(&bmp, 1, sizeof(BMP), wfd);
	retval += fwrite(&bmpinfo40, 1, sizeof(BITMAPINFOHEADER), wfd);
	
	assert(retval == sizeof(BMP)+sizeof(BITMAPINFOHEADER));
	
	retval = fwrite(&palette[0], 1, 256*4, wfd);
	assert(retval == 256*4);
	
	char *buff=malloc(tex.width*tex.height);
	assert(buff);
	
	//step 5, output bmp file, but in reverse, since BMP is bottom up vs everything else is top down
	retval = fread(buff, tex.width*tex.height,1,rfd);
	char padding[4] = {0};
	
	for(int i=tex.height; i--;) {
		retval = fwrite(buff+i*tex.width, 1, tex.width, wfd);
		assert(retval == tex.width);
		//padding,
		if (widpad) {
			retval = fwrite(&padding[0], 1, widpad, wfd);
		}
	}
	
	fclose(wfd);
}
void bmp2tex();

int main(int argc, char **argv) {
	//check packing is correct, should result in proper structs
	assert(sizeof(DIB)==12);
	assert(sizeof(BITMAPINFOHEADER)==40);
	assert(sizeof(BMP)==14);
	assert(sizeof(TEX)==56);
	
	//warning label.
	printf(	"This will try to convert tex and bmp files accordingly based on extention.\n"
			"As this is experimental, it is recommended you check the output is correct,\n"
			"and report any problems to rtcvb32@yahoo.com\n");
	
	for(int i=1; i<argc; i++) {
		if (strcasecmp(argv[i]+strlen(argv[i])-4, ".tex") == 0) {
			tex2bmp(argv[i]);
		} else if (strcasecmp(argv[i]+strlen(argv[i])-4, ".bmp") == 0) {

		} else {
			printf("Unknown or unsupported extension from file %s\n", argv[i]);
		}
	}
}
