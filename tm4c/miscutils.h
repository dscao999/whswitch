#ifndef MISCUTILS_DSCAO__
#define MISCUTILS_DSCAO__
#include <stdint.h>

#define likely(x)  __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

static inline int is_hexdigit(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
		(c >= 'A' && c <= 'F');
}

static inline uint32_t str2num_hex(const char *digits, int len)
{
	uint32_t result = 0;
	int i, d;
	const char *digit;

	for (i = 0, digit = digits; i < len && is_hexdigit(*digit); i++) {
		result <<= 4;
		d = 0;
		if (*digit >= '0' && *digit <= '9')
			d = *digit++ - '0';
		else if (*digit >= 'A' && *digit <= 'F')
			d = *digit++ - 'A' + 10;
		else if (*digit >= 'a' && *digit <= 'f')
			d = *digit++ - 'a' + 10;
		result += d;
	}
	return result;
}

static inline int str2num_dec(const char *digits, int len)
{
	uint32_t result = 0;
	int d, neg;
	char sign;
	const char *digit, *dend;

	if (len <= 0)
		return 0;

	dend = digits + len;
	digit = digits;
	while (*digit == ' ' && digit < dend)
		digit++;
	if (digit == dend)
		return 0;

	neg = 1;
	sign = *digit;
	if (sign == '-' || sign == '+') {
		digit++;
		if (sign == '-')
			neg = -1;
	}
	while (digit < dend && *digit >= '0' && *digit <= '9') {
		result *= 10;
		d = *digit++ - '0';
		result += d;
	}
	return neg < 0? -result : result;
}

static inline uint16_t swap16(uint16_t v16)
{
	return (v16 >> 8)|(v16 << 8);
}

static inline void memset(void *dst, int v, int len)
{
	uint8_t *pdst = dst;
	int i;

	for (i = 0; i < len; i++, pdst++)
		*pdst = v;
}

static inline int memchar(const char *str, int len, uint8_t token)
{
	const char *pstr = str;
	int tpos;

	for (tpos = 0; tpos < len; tpos++, pstr++)
		if (*pstr == token)
			break;
	return tpos;
}

static inline void memcpy(void *dst, const void *src, int len)
{
	const uint8_t *psrc = src;
	uint8_t *pdst = dst;
	int i;

	for(i = 0; i < len; i++, psrc++, pdst++)
		*pdst = *psrc;
}

static inline int memcmp(const void *a, const void *b, int len)
{
	int i, retv;
	const uint8_t *aa, *bb;

	retv = 0;
	aa = a;
	bb = b;
	for (i = 0; i < len; i++, aa++, bb++) {
		if (*aa > *bb) {
			retv = 1;
			break;
		} else if (*aa < *bb) {
			retv = -1;
			break;
		}
	}
	return retv;
}

static inline char num2char_hex(uint8_t v)
{
	if (v > 9)
		return 'a' + v - 10;
	else
		return '0' + v;
}

static inline int strlen(const char *str)
{
	const char *cstr;
	int i;

	for (i = 0, cstr = str; *cstr != 0; cstr++, i++)
		;
	return i;
}

static inline int bytes2str_hex(const uint8_t *pbyte, int len, char *buf)
{
	int i;
	const uint8_t *pcbyte;
	char *pcbuf;

	pcbuf = buf;
	for (i = 0, pcbyte = pbyte; i < len; i++, pcbyte++) {
		*pcbuf++ = num2char_hex(*pcbyte >> 4);
		*pcbuf++ = num2char_hex(*pcbyte & 0x0f);
		*pcbuf++ = ' ';
	}
	return pcbuf - buf;
}

static inline int num2str_hex(uint32_t v, char *buf)
{
	*(buf+7) = num2char_hex(v & 0x0f);
	*(buf+6) = num2char_hex((v>>=4) & 0x0f);
	*(buf+5) = num2char_hex((v>>=4) & 0x0f);
	*(buf+4) = num2char_hex((v>>=4) & 0x0f);
	*(buf+3) = num2char_hex((v>>=4) & 0x0f);
	*(buf+2) = num2char_hex((v>>=4) & 0x0f);
	*(buf+1) = num2char_hex((v>>=4) & 0x0f);
	*(buf) = num2char_hex((v>>=4) & 0x0f);
	return 8;
}

static inline int num2str_dec(const int v, char *buf, int len)
{
	int neg, rlen, i;
	uint32_t tv;
	char *cbuf, cctmp;

	if (len <= 0)
		return 0;

	neg = 0;
	if (v < 0) {
		neg = 1;
		tv = -v;
	} else
		tv = v;

	i = 0;
	cbuf = buf;
	while (tv > 0 && i < len) {
		*cbuf++ = '0' + tv % 10;
		tv /= 10;
		i++;
	}
	if (v == 0)
		*cbuf++ = '0';
	if (neg && i < len)
		*cbuf++ = '-';
	rlen = cbuf - buf;
	for (i = 0; i < rlen/2; i++) {
		cctmp = buf[i];
		buf[i] = buf[rlen-i-1];
		buf[rlen-i-1] = cctmp;
	}
	return rlen;
}

#endif /* MISCUTILS_DSCAO__ */
