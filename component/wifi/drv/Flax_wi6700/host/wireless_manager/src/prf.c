/* modfiied from sha1.c of hostapd */
/*
 * SHA1 hash implementation and interface functions
 * Copyright (c) 2003-2005, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#ifdef CONFIG_LYNX_OS_UCOS
/* ucos */
#include <mt_type.h>
#include <os/mtos_misc.h>
#include <porting.h>
#include <string.h>
#elif defined(CONFIG_LYNX_OS_LINUX)
#include <linux/types.h>
#include <linux/string.h>
#endif

#include <wlan_def.h>
#include <wpa.h>
#include <sha1.h>

/**
 * sha1_prf - SHA1-based Pseudo-Random Function (PRF) (IEEE 802.11i, 8.5.1.1)
 * @key: Key for PRF
 * @key_len: Length of the key in bytes
 * @label: A unique label for each purpose of the PRF
 * @data: Extra data to bind into the key
 * @data_len: Length of the data
 * @buf: Buffer for the generated pseudo-random key
 * @buf_len: Number of bytes of key to generate
 *
 * This function is used to derive new, cryptographically separate keys from a
 * given key (e.g., PMK in IEEE 802.11i).
 */
void sha1_prf(const u8 *key, u32 key_len, const char *label,
	      const u8 *data, u32 data_len, u8 *buf, u32 buf_len)
{
	u8 counter = 0;
	u32 pos, plen;
	u8 hash[SHA1_MAC_LEN];
	u32 label_len = strlen(label) + 1;
	const unsigned char *addr[3];
	u32 len[3];

	addr[0] = (u8 *) label;
	len[0] = label_len;
	addr[1] = data;
	len[1] = data_len;
	addr[2] = &counter;
	len[2] = 1;

	pos = 0;
	while (pos < buf_len) {
		plen = buf_len - pos;
		if (plen >= SHA1_MAC_LEN) {
			hmac_sha1_vector(key, key_len, 3, addr, len,
					 &buf[pos]);
			pos += SHA1_MAC_LEN;
		} else {
			hmac_sha1_vector(key, key_len, 3, addr, len,
					 hash);
			memcpy(&buf[pos], hash, plen);
			break;
		}
		counter++;
	}
}

static void pbkdf2_sha1_f(const char *passphrase, const char *ssid,
			  u32 ssid_len, int iterations, unsigned int count,
			  u8 *digest)
{
	unsigned char tmp[SHA1_MAC_LEN], tmp2[SHA1_MAC_LEN];
	int i, j;
	unsigned char count_buf[4];
	const u8 *addr[2];
	u32 len[2];
	u32 passphrase_len = strlen(passphrase);

	addr[0] = (u8 *) ssid;
	len[0] = ssid_len;
	addr[1] = count_buf;
	len[1] = 4;

	/* F(P, S, c, i) = U1 xor U2 xor ... Uc
	 * U1 = PRF(P, S || i)
	 * U2 = PRF(P, U1)
	 * Uc = PRF(P, Uc-1)
	 */

	count_buf[0] = (count >> 24) & 0xff;
	count_buf[1] = (count >> 16) & 0xff;
	count_buf[2] = (count >> 8) & 0xff;
	count_buf[3] = count & 0xff;
	hmac_sha1_vector((u8 *) passphrase, passphrase_len, 2, addr, len, tmp);
	memcpy(digest, tmp, SHA1_MAC_LEN);
	
	for (i = 1; i < iterations; i++) {
		hmac_sha1(tmp, SHA1_MAC_LEN, (u8 *) passphrase, passphrase_len, 
			  tmp2);
		memcpy(tmp, tmp2, SHA1_MAC_LEN);
		for (j = 0; j < SHA1_MAC_LEN; j++)
			digest[j] ^= tmp2[j];
	}
}


/**
 * pbkdf2 - SHA1-based key derivation function (PBKDF2) for IEEE 802.11i
 * @passphrase: ASCII passphrase
 * @ssid: SSID
 * @ssid_len: SSID length in bytes
 * @iterations: Number of iterations to run
 * @buf: Buffer for the generated key
 * @buflen: Length of the buffer in bytes
 *
 * This function is used to derive PSK for WPA-PSK. For this protocol,
 * iterations is set to 4096 and buflen to 32. This function is described in
 * IEEE Std 802.11-2004, Clause H.4. The main construction is from PKCS#5 v2.0.
 */
void pbkdf2(const char *passphrase, const char *ssid, u32 ssid_len,
		 int iterations, u8 *buf, u32 buflen)
{
	unsigned int count = 0;
	unsigned char *pos = buf;
	u32 left = buflen, plen;
	unsigned char digest[SHA1_MAC_LEN];

	while (left > 0) {
		count++;
		pbkdf2_sha1_f(passphrase, ssid, ssid_len, iterations, count,
			      digest);
		plen = left > SHA1_MAC_LEN ? SHA1_MAC_LEN : left;
		
		memcpy(pos, digest, plen);
		pos += plen;
		left -= plen;
	}
}

