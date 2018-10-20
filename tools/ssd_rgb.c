#include <stdio.h>
#include <string.h>

static unsigned short rgb0[96];
static unsigned short rgb1[96];
static unsigned short rgb2[96];

int main(int argc, char *argv[])
{
	const char *fname = argv[1];
	char fpath[128];
	FILE *of;
	unsigned short *cp;
	int i;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s file_name\n", argv[0]);
		return 1;
	}

	strcpy(fpath, fname);
	strcat(fpath, "_r.ssd");
	of = fopen(fpath, "wb");
	if (!of) {
		fprintf(stderr, "Unable to open the file: %s\n", fpath);
		return 100;
	}
	for (cp = rgb0, i = 0; i < 96; i++)
		*cp++ = 0x00f8;
	for (cp = rgb1, i = 0; i < 96; i++)
		*cp++ = 0x0;
	for (i = 0; i < 16; i++)
		fwrite(rgb0, 2, 96, of);
	for (i = 16; i < 32; i++)
		fwrite(rgb1, 2, 96, of);
	for (i = 32; i < 48; i++)
		fwrite(rgb0, 2, 96, of);
	for (i = 48; i < 64; i++)
		fwrite(rgb1, 2, 96, of);
	fclose(of);

	strcpy(fpath, fname);
	strcat(fpath, "_g.ssd");
	of = fopen(fpath, "wb");
	if (!of) {
		fprintf(stderr, "Unable to open the file: %s\n", fpath);
		return 100;
	}
	for (cp = rgb0, i = 0; i < 96; i++)
		*cp++ = 0xe007;
	for (i = 0; i < 64; i++)
		fwrite(rgb0, 2, 96, of);
	fclose(of);

	strcpy(fpath, fname);
	strcat(fpath, "_b.ssd");
	of = fopen(fpath, "wb");
	if (!of) {
		fprintf(stderr, "Unable to open the file: %s\n", fpath);
		return 100;
	}
	for (cp = rgb0, i = 0; i < 96; i++)
		*cp++ = 0x1f00;
	for (i = 0; i < 64; i++)
		fwrite(rgb0, 2, 96, of);
	fclose(of);

	return 0;
}
