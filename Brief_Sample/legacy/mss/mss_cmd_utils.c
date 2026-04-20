/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "mss_cmd_utils.h"

void HexStrToByte(const char* source, unsigned char* dest, unsigned int *len)
{
    unsigned char highByte, lowByte;
    unsigned int i;
    unsigned int srclen;

    if (source[0] == '0' && (source[1] == 'x' || source[1] == 'X'))
        source +=2;

    srclen = strlen(source);
    if (srclen > (*len * 2))
        srclen = (*len * 2);

    for (i = 0; i < srclen; i += 2)
    {
        highByte = toupper(source[i]);
        lowByte = toupper(source[i + 1]);

        if (highByte >= '0' && highByte <= '9')
            highByte -= '0';
        else if (highByte >= 'A' && highByte <= 'F')
            highByte = highByte - 'A' + 0xa;

        if (lowByte >= '0' && lowByte <= '9')
            lowByte -= '0';
        else if (lowByte >= 'A' && lowByte <= 'F')
            lowByte = lowByte - 'A' + 0xa;

        dest[i / 2] = (highByte << 4) | lowByte;
    }
    *len = srclen / 2;
    return;
}

void mss_dump(const char *tag, unsigned char *buffer, unsigned int len)
{
    unsigned int i = 0;
    printf("\n%s(0x%lx):[%d]\n", tag, (unsigned long)buffer, len);
    for(i = 0; i < len; i++)
    {
        if((i%16) == 0 && i != 0)
            printf("\n");
        printf("%02X ", buffer[i]);
    }
    printf("\n");
}

