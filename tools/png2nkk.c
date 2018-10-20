#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <zlib.h>
#include <png.h>
#include <sys/mman.h>

void pngo_version(void)
{
	fprintf(stderr, "Compiled with libpng %s; using libpng %s.\n",
		PNG_LIBPNG_VER_STRING, png_libpng_ver);
	fprintf(stderr, "Compiled with zlib %s; using zlib %s.\n",
		ZLIB_VERSION, zlib_version);
}

struct png_handle {
	png_structp png;
	png_infop info;
	FILE *pngf;
	void *map;
	png_bytepp row;
	long mlen;
	unsigned short *oledmap;
	int width, height, depth, colortyp;
	int rbytes;
};

int pngo_read_init(const char *pngfname, struct png_handle *ph)
{
	unsigned char sig[8];
	int retv;

	retv = 0;
	ph->pngf = fopen(pngfname, "rb");
	if (!ph->pngf) {
		fprintf(stderr, "Cannot open file %s: %s\n", pngfname,
			strerror(errno));
		retv = 1;
		goto err_10;
	}
	fread(sig, 1, 8, ph->pngf);
	if (!png_check_sig(sig, 8)) {
		pngo_version();
		fprintf(stderr, "Not a valid PNG format!\n");
		retv = 2;
		goto err_20;
	}
	ph->png = png_create_read_struct(PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL);
	if (!ph->png) {
		fprintf(stderr, "Out of Memory!\n");
		retv = 4;
		goto err_20;
	}
	ph->info = png_create_info_struct(ph->png);
	if (!ph->info) {
		fprintf(stderr, "Out of Memory!\n");
		retv = 4;
		goto err_30;
	}
	fseek(ph->pngf, 0, SEEK_SET);
	png_init_io(ph->png, ph->pngf);
	png_read_info(ph->png, ph->info);
	ph->colortyp = png_get_color_type(ph->png, ph->info);
	ph->height = png_get_image_height(ph->png, ph->info);
	ph->width = png_get_image_width(ph->png, ph->info);
	ph->depth = png_get_bit_depth(ph->png, ph->info);
	if (ph->width != 96 || ph->height != 64) {
		fprintf(stderr, "Invalid size! Width: %d, Depth: %d\n",
			ph->width, ph->height);
		retv = 5;
		goto err_40;
	}
	if (ph->colortyp == PNG_COLOR_TYPE_GRAY && ph->depth < 8) {
		fprintf(stderr, "Cannot process gray color < 8 bits!\n");
		retv = 6;
		goto err_40;
	}

	return retv;

err_40:
	png_destroy_read_struct(&ph->png, &ph->info, NULL);
	ph->png = NULL;
err_30:
	if (!ph->png)
		png_destroy_read_struct(&ph->png, NULL, NULL);
err_20:
	fclose(ph->pngf);
err_10:
	return retv;
}

void pngo_transform(struct png_handle *ph)
{
	if (ph->colortyp == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(ph->png);
	if (png_get_valid(ph->png, ph->info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(ph->png);
	if (ph->depth == 16)
		png_set_strip_16(ph->png);
	if (ph->colortyp == PNG_COLOR_TYPE_GRAY ||
			ph->colortyp == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(ph->png);
	if (ph->colortyp & PNG_COLOR_MASK_ALPHA)
		png_set_strip_alpha(ph->png);
	png_read_update_info(ph->png, ph->info);
}

static inline int align16(int v)
{
	return (((v - 1) >> 4) + 1) << 4;
}

int pngo_read_image(struct png_handle *ph)
{
	int retv, i;
	png_bytep *rp, cy;

	retv = 0;
	ph->rbytes = png_get_rowbytes(ph->png, ph->info);
	ph->mlen = align16(ph->height*ph->rbytes) + sizeof(*rp)*ph->height +
			sizeof(short)*ph->height*ph->width;
	ph->map = mmap(NULL, ph->mlen, PROT_READ|PROT_WRITE,
		MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if (!ph->map) {
		fprintf(stderr, "Out of Memory!\n");
		retv = 1;
		goto exit_10;
	}
	ph->oledmap = ph->map + align16(ph->height*ph->rbytes)
			+ sizeof(*rp)*ph->height;

	cy = ph->map;
	ph->row = ph->map + align16(ph->height*ph->rbytes);
	rp = ph->row;
	for (i = 0; i < ph->height; i++) {
		*rp++ = cy;
		cy += ph->rbytes;
	}
	png_read_image(ph->png, ph->row);

exit_10:
	return retv;
}

void pngo_read_end(struct png_handle *ph)
{
	png_read_end(ph->png, ph->info);
	png_destroy_read_struct(&ph->png, &ph->info, NULL);
	if (ph->map)
		munmap(ph->map, ph->mlen);
	fclose(ph->pngf);
}

static unsigned short rgb2ssd(const unsigned char *rgb)
{
	static const double rb_ratio = 31.0/255.0, g_ratio = 63.0/255.0;
	unsigned int r, g, b;
	unsigned char lob, hib;

	r = (*rgb++)*rb_ratio;
	g = (*rgb++)*g_ratio;
	b = (*rgb)*rb_ratio;

	lob = ((r & 0x1f) << 3) | ((g & 0x3f) >> 3);
	hib = ((g & 0x07) << 5) | (b & 0x1f);
	return (hib << 8) | lob;
}

static void png2ssd1331(struct png_handle *ph)
{
	int i, j;
	unsigned char *pm;
	unsigned short *om, *em;

	pm = ph->map;
	em = ph->oledmap;
	om = ph->oledmap + (ph->width * ph->height) / 2;
	for (i = 0; i < ph->height; i+=2) {
		for (j = 0; j < ph->width; j++) {
			*em++ = rgb2ssd(pm);
			pm += 3;
		}
		for (j = 0; j < ph->width; j++) {
			*om++ = rgb2ssd(pm);
			pm += 3;
		}
	}
}


static void png_write(struct png_handle *ph)
{
	FILE *of;
	png_structp png;
	png_infop info;
	png_bytepp rowptr;

	of = fopen("./copy_test.png", "wb");
	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info = png_create_info_struct(png);
	png_init_io(png, of);

	png_set_IHDR(png, info, 96, 64, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	rowptr = ph->row;
	png_set_rows(png, info, rowptr);
	png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);

	png_destroy_write_struct(&png, &info);
	fclose(of);
}

static struct png_handle ph;
int main(int argc, char *argv[])
{
	const char *iname, *oname;
	int retv;
	FILE *ofh;

	retv = 0;
	if (argc != 3) {
		fprintf(stderr, "Usage: %s input output\n", argv[0]);
		retv = 1;
		goto exit_00;
	}
	iname = argv[1];
	oname = argv[2];

	if (pngo_read_init(iname, &ph) != 0) {
		retv = 2;
		goto exit_00;
	}
	pngo_transform(&ph);
	retv = pngo_read_image(&ph);
	if (retv)
		goto exit_10;

	ofh = fopen(oname, "wb");
	if (!ofh) {
		fprintf(stderr, "Cannot open file '%s' for write: %s.\n",
			oname, strerror(errno));
		retv = 4;
		goto exit_10;
	}
	png2ssd1331(&ph);
	fwrite(ph.oledmap, 1, 2*ph.width*ph.height, ofh);
	fclose(ofh);

	png_write(&ph);

exit_10:
	pngo_read_end(&ph);
exit_00:
	return retv;
}
