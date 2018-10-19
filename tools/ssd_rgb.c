#include <stdio.h>
#include <string.h>

static unsigned short rgb[96];

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

	for (cp = rgb, i = 0; i < 96; i++)
		*cp++ = 0x00f8;
	for (i = 0; i < 64; i++)
		fwrite(rgb, 2, 96, of);
	fclose(of);
}
