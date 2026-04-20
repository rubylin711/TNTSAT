#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mt_base64.h"

// base64 encode & decode
static char g_mapkey[64];
static char g_unmapkey[128];
static int is_inited = 0;

static void __mt_base64_init(void)
{
	int i;
	
	if (is_inited)
		return;

	for(i = 0; i < 26; ++i)
		g_mapkey[i] = 'A' + i;

	for (i = 0; i < 26; ++i)
		g_mapkey[i +26] = 'a' + i;

	g_mapkey[52] = '0';
	for (i = 0; i < 9; ++i)
		g_mapkey[i + 53] = '1' + i;

	g_mapkey[62] = '+';
	g_mapkey[63] = '/';


	for (i = 0; i < 64; ++i)
		g_unmapkey[g_mapkey[i]] = i;

	is_inited = 1;
}

int mt_base64_encode(void* src, int len, void* dst)
{
	unsigned char* p_src = src;
	unsigned char* p_dst = dst;
	int i, cnt, rst;
	int p_idx, idx;
	unsigned char byte1, byte2, byte3;
	unsigned char ch1, ch2, ch3, ch4;

	__mt_base64_init();
	rst = len % 3;
	cnt = len - rst;

	p_idx = 0;
	for (i = 0; i < cnt; i += 3) {
		byte1 = p_src[i];
		byte2 = p_src[i + 1];
		byte3 = p_src[i + 2];

		ch1 = (byte1 >> 2) & 0x3f;
		ch2 = ((byte1 & 3) << 4) | (byte2 >> 4);
		ch3 = ((byte2 & 0xf) << 2) | (byte3 >> 6);
		ch4 = byte3 & 0x3f;
		
		p_dst[p_idx++] = g_mapkey[ch1];
		p_dst[p_idx++] = g_mapkey[ch2];
		p_dst[p_idx++] = g_mapkey[ch3];
		p_dst[p_idx++] = g_mapkey[ch4];
//		printf("\n[%X][%X][%X] %d %d %d %d ", byte1, byte2, byte3, ch1, ch2, ch3, ch4);
	}

	i = cnt;
	if (rst == 1) {
		byte1 = p_src[i];
		ch1 = (byte1 >> 2) & 0x3f;
		ch2 = (byte1 & 3) << 4;
		p_dst[p_idx++] = g_mapkey[ch1];
		p_dst[p_idx++] = g_mapkey[ch2];
		p_dst[p_idx++] = '=';
		p_dst[p_idx++] = '=';
	} else if (rst == 2) {
		byte1 = p_src[i];
		byte2 = p_src[i + 1];

		ch1 = (byte1 >> 2) & 0x3f;
		ch2 = ((byte1 & 3) << 4) | (byte2 >> 4);
		ch3 = (byte2 & 0xf) << 2;

		p_dst[p_idx++] = g_mapkey[ch1];
		p_dst[p_idx++] = g_mapkey[ch2];
		p_dst[p_idx++] = g_mapkey[ch3];
		p_dst[p_idx++] = '=';
	}

	return p_idx;
}

int mt_base64_decode(void* src, int len, void* dst)
{
	unsigned char* p_src = src;
	unsigned char* p_dst = dst;
	unsigned char ch1, ch2, ch3, ch4;
	int i, idx = 0;

	__mt_base64_init();

	if (len == 0 || len % 4)
		return -1;

	
	for (i = 0; i < len - 4; i += 4) {
		ch1 = g_unmapkey[p_src[i + 0]];
		ch2 = g_unmapkey[p_src[i + 1]];
		ch3 = g_unmapkey[p_src[i + 2]];
		ch4 = g_unmapkey[p_src[i + 3]];
		p_dst[idx++] = (ch1 << 2) | (ch2 >> 4);
		p_dst[idx++] = ((ch2 & 0xF) << 4) | (ch3 >> 2);
		p_dst[idx++] = ((ch3 & 3) << 6) | ch4;

	}

	if (p_src[len - 1] != '=') {
		ch1 = g_unmapkey[p_src[i + 0]];
		ch2 = g_unmapkey[p_src[i + 1]];
		ch3 = g_unmapkey[p_src[i + 2]];
		ch4 = g_unmapkey[p_src[i + 3]];

		p_dst[idx++] = (ch1 << 2) | (ch2 >> 4);
		p_dst[idx++] = ((ch2 & 0xF) << 4) | (ch3 >> 2);
		p_dst[idx++] = ((ch3 & 3) << 6) | ch4;
	} else if (p_src[len - 2] == '=') {
		ch1 = g_unmapkey[p_src[i + 0]];
		ch2 = g_unmapkey[p_src[i + 1]];
		p_dst[idx++] = (ch1 << 2) | (ch2 >> 4);
	} else {
		ch1 = g_unmapkey[p_src[i + 0]];
		ch2 = g_unmapkey[p_src[i + 1]];
		ch3 = g_unmapkey[p_src[i + 2]];
		p_dst[idx++] = (ch1 << 2) | (ch2 >> 4);
		p_dst[idx++] = ((ch2 & 0xF) << 4) | (ch3 >> 2);
	}
	
	return idx;
}

#ifdef BASE64TEST
int main(int argc, char *argv[])
{
	char e_buf[128];
	char d_buf[128];
	int i, len;

	mt_base64_init();
	
	len = mt_base64_encode(argv[1], strlen(argv[1]), e_buf);
	if (len < 128) {
		e_buf[len] = 0;
		printf("\nBefore encode:\t%s\n", argv[1]);
		printf("After encode:\t%s\n", e_buf);
	}

	len = mt_base64_decode(e_buf, len, d_buf);
	if (len > 0) {
		d_buf[len] = 0;
		printf("After encode:\t%s\n", d_buf);
	}
	return 0;
}

#endif
