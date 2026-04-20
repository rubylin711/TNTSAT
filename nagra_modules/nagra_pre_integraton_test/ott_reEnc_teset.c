#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ca_sec.h"
#include "mt_common.h"

extern unsigned char *mt_unf_cipher_malloc(unsigned int length);
extern int mt_unf_cipher_free(unsigned char *p);

#if 1
#define OTT_REENC_TEST_MSG(fmt, ...)  printf("[OTT_REENC]: %s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define OTT_REENC_TEST_MSG(fmt, ...)
#endif


#define MAX_CHUNKS 16
#define LINE_BUFFER_SIZE 256
#define TEST_OTT_REENC_BUFFER_MAX (500 *1024)

typedef struct {
    int chunk_id;
    int clear_size;
    int protected_size;
    int kid;
    int skip_blocks;
    int crypto_blocks;
    unsigned char iv[16];
    char iv_str[64];
} DataChunk;

typedef struct {
    int buffer_size;
    unsigned char* data;
    int parsed;
    int first_line_number;
    char first_line_content[LINE_BUFFER_SIZE];
} EncryptedBuffer;

// ½âÎöÊ®Áù½øÖÆ×Ö½Ú
int parse_hex_byte(const char *hex_str) {
    int value;
    sscanf(hex_str, "%2x", &value);
    return value;
}

// ÌáÈ¡IVÖµ
int extract_iv(const char *line, unsigned char *iv, char *iv_str) {
    char *pos = strstr(line, "IV{");
	int i = 0, j = 0;
    if (pos == NULL) return -1;

    pos += 3; // Ìø¹ý"IV{"

    // ±£´æÔ­Ê¼IV×Ö·û´®
    char *end_pos = strchr(pos, '}');
    if (end_pos) {
        int len = end_pos - pos;
        if (len < 63) {
            strncpy(iv_str, pos, len);
            iv_str[len] = '\0';
        }
    }

    // ´¦Àí¿ÕIVµÄÇé¿ö
    if (*pos == '}') {
        memset(iv, 0, 16);
        iv_str[0] = '\0';
        return 0;
    }

    // ½âÎö16×Ö½ÚIV
    i =  0;
    while (*pos && *pos != '}' && i < 16) {
        // Ìø¹ý¿Õ¸ñ
        while (*pos == ' ') pos++;

        // ¼ì²éÊÇ·ñ»¹ÓÐÊý¾Ý
        if (*pos == '}' || *pos == '\0') {
            // Ê£Óà×Ö½ÚÌî³ä0
            for (j = i; j < 16; j++) {
                iv[j] = 0;
            }
            break;
        }

        // ÌáÈ¡Á½¸öÊ®Áù½øÖÆ×Ö·û
        char hex_pair[3] = {0};
        int char_count = 0;

        // µÚÒ»¸ö×Ö·û
        if ((*pos >= '0' && *pos <= '9') ||
            (*pos >= 'A' && *pos <= 'F') ||
            (*pos >= 'a' && *pos <= 'f')) {
            hex_pair[char_count++] = *pos++;
        }

        // µÚ¶þ¸ö×Ö·û
        if ((*pos >= '0' && *pos <= '9') ||
            (*pos >= 'A' && *pos <= 'F') ||
            (*pos >= 'a' && *pos <= 'f')) {
            hex_pair[char_count++] = *pos++;
        }

        iv[i++] = parse_hex_byte(hex_pair);
    }

    // Ìî³äÊ£Óà×Ö½ÚÎª0
    for (; i < 16; i++) {
        iv[i] = 0;
    }

    return 0;
}

// ÌáÈ¡²ÎÊýÖµ
int extract_parameter(const char *line, const char *param_name) {
    char *pos = strstr(line, param_name);
    if (pos == NULL) return -1;

    // ÕÒµ½²ÎÊýÖµ¿ªÊ¼Î»ÖÃ
    pos = strchr(pos, '{');
    if (pos == NULL) return -1;

    int value;
    sscanf(pos, "{%d}", &value);
    return value;
}

// ½âÎöÒ»ÐÐÊ®Áù½øÖÆÊý¾Ý
int parse_hex_line(const char *line, unsigned char *buffer, int max_size) {
    int count = 0;
    const char *ptr = line;

    // Ìø¹ýÐÐÊ×¿Õ¸ñ
    while (*ptr == ' ' || *ptr == '\t') ptr++;

    // ½âÎöÊ®Áù½øÖÆÊý¾Ý
    while (*ptr && count < max_size) {
        // Ìø¹ý¿Õ¸ñ
        while (*ptr == ' ') ptr++;

        // ¼ì²éÊÇ·ñÊÇÊ®Áù½øÖÆ×Ö·û
        if ((*ptr >= '0' && *ptr <= '9') ||
            (*ptr >= 'A' && *ptr <= 'F') ||
            (*ptr >= 'a' && *ptr <= 'f')) {

            char hex_pair[3] = {0};
            hex_pair[0] = *ptr++;
            if (*ptr && ((*ptr >= '0' && *ptr <= '9') ||
                         (*ptr >= 'A' && *ptr <= 'F') ||
                         (*ptr >= 'a' && *ptr <= 'f'))) {
                hex_pair[1] = *ptr++;
            }

            buffer[count++] = parse_hex_byte(hex_pair);
        } else {
            ptr++;
        }
    }

    return count;
}

// ½âÎöËùÓÐChunkCountÊý¾Ý
int parse_all_chunks(DataChunk* chunks, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        OTT_REENC_TEST_MSG("open failed: %s\n", filename);
        return -1;
    }

    char line[LINE_BUFFER_SIZE];
    int chunk_count = 0;

    while (fgets(line, sizeof(line), file) && chunk_count < MAX_CHUNKS) {
        // ²éÕÒChunkCount
        if (strstr(line, "ChunkCount")) {
            DataChunk chunk;
            memset(&chunk, 0, sizeof(DataChunk));

            // ÌáÈ¡chunk_id
            char *pos = strstr(line, "ChunkCount {");
            if (pos) {
                sscanf(pos, "ChunkCount {%d}", &chunk.chunk_id);
            }

            // ÌáÈ¡ÆäËû²ÎÊý
            chunk.clear_size = extract_parameter(line, "ClearSize");
            chunk.protected_size = extract_parameter(line, "protectedSize");
            chunk.kid = extract_parameter(line, "kid");
            chunk.skip_blocks = extract_parameter(line, "skipBlocks");
            chunk.crypto_blocks = extract_parameter(line, "cryptoBlocks");

            // ÌáÈ¡IV
            extract_iv(line, chunk.iv, chunk.iv_str);

            chunks[chunk_count] = chunk;
            chunk_count++;
        }
    }

    fclose(file);
    return chunk_count;
}

// ½âÎö¼ÓÃÜ»º³åÇøÊý¾Ý
int parse_encrypted_buffers(const char* filename, EncryptedBuffer* buffers) {
    FILE* file = fopen(filename, "r");
	int i = 0;
    if (!file) {
        OTT_REENC_TEST_MSG("open failed: %s\n", filename);
        return -1;
    }

    char line[LINE_BUFFER_SIZE];
    int current_buffer = -1;
    int in_encrypted_block = 0;
    int buffer_index = 0;
    int line_number = 0;

    // ³õÊ¼»¯»º³åÇø
    for (i =  0; i < 2; i++) {
        buffers[i].data = mt_unf_cipher_malloc(TEST_OTT_REENC_BUFFER_MAX);
        OTT_REENC_TEST_MSG("buffers[i].data = %p", buffers[i].data);
        memset(buffers[i].data, 0x00, TEST_OTT_REENC_BUFFER_MAX);
        OTT_REENC_TEST_MSG("\n");
        buffers[i].buffer_size = 0;
        buffers[i].parsed = 0;
        buffers[i].first_line_number = 0;
        buffers[i].first_line_content[0] = '\0';
        if (!buffers[i].data) {
            OTT_REENC_TEST_MSG("alloc fialed\n");
            fclose(file);
            return -1;
        }
    }

    while (fgets(line, sizeof(line), file)) {
        line_number++;
//OTT_REENC_TEST_MSG("line_number = %d", line_number);
        // ÒÆ³ý»»ÐÐ·û
        line[strcspn(line, "\n")] = 0;

        // ¼ì²â¼ÓÃÜÊý¾Ý¿é¿ªÊ¼
        if (strstr(line, "Encrypted BufferSize")) {
            current_buffer++;
            if (current_buffer < 2) {
                // »ñÈ¡»º³åÇø´óÐ¡
                char *pos = strstr(line, "Encrypted BufferSize {");
                if (pos) {
                    sscanf(pos, "Encrypted BufferSize {%d}", &buffers[current_buffer].buffer_size);
                }
                in_encrypted_block = 1;
                buffer_index = 0;
            }
            continue;
        }

        // ¼ì²â¼ÓÃÜÊý¾Ý¿é½áÊø
        if (in_encrypted_block && strchr(line, '}')) {
            in_encrypted_block = 0;
            if (current_buffer < 2) {
                buffers[current_buffer].parsed = 1;
            }
            continue;
        }

        // ½âÎö¼ÓÃÜÊý¾Ý²¢¼ÇÂ¼µÚÒ»ÐÐÐÅÏ¢
        if (in_encrypted_block && current_buffer < 2 && strlen(line) > 0) {
            // ¼ÇÂ¼µÚÒ»¸öÊý¾ÝÐÐµÄÐÅÏ¢
            if (buffer_index == 0 && current_buffer >= 0) {
                buffers[current_buffer].first_line_number = line_number;
                strncpy(buffers[current_buffer].first_line_content, line, LINE_BUFFER_SIZE-1);
                buffers[current_buffer].first_line_content[LINE_BUFFER_SIZE-1] = '\0';
            }

            int count = parse_hex_line(line, buffers[current_buffer].data + buffer_index,
                                     TEST_OTT_REENC_BUFFER_MAX - buffer_index);
            buffer_index += count;
        }
    }

    fclose(file);
    return current_buffer + 1;
}

// ´òÓ¡ËùÓÐchunks
void print_chunks(DataChunk* chunks, int chunk_count) {
	int i = 0;
    OTT_REENC_TEST_MSG("get Chunk  %d):\n", chunk_count);
    OTT_REENC_TEST_MSG("============================================================\n");
    for (i = 0; i < chunk_count; i++) {
        OTT_REENC_TEST_MSG("Chunk %d:\n", chunks[i].chunk_id);
        OTT_REENC_TEST_MSG("  ClearSize: %d\n", chunks[i].clear_size);
        OTT_REENC_TEST_MSG("  ProtectedSize: %d\n", chunks[i].protected_size);
        OTT_REENC_TEST_MSG("  KID: %d\n", chunks[i].kid);
        OTT_REENC_TEST_MSG("  SkipBlocks: %d\n", chunks[i].skip_blocks);
        OTT_REENC_TEST_MSG("  CryptoBlocks: %d\n", chunks[i].crypto_blocks);
        OTT_REENC_TEST_MSG("  IV: %s\n", strlen(chunks[i].iv_str) > 0 ? chunks[i].iv_str : "(empty)");
        OTT_REENC_TEST_MSG("------------------------------------------------------------\n");
    }
}

// ´òÓ¡¼ÓÃÜ»º³åÇøµÚÒ»ÐÐÊý¾ÝÐÅÏ¢
void print_buffer_first_lines(EncryptedBuffer* buffers, int buffer_count) {
	int i = 0;
    OTT_REENC_TEST_MSG("\nfirst lin data¢:\n");
    OTT_REENC_TEST_MSG("============================================================\n");
    for (i = 0; i < buffer_count && i < 2; i++) {
        if (buffers[i].parsed) {
            OTT_REENC_TEST_MSG("%d Encrypted Buffer:\n", i+1);
            OTT_REENC_TEST_MSG("  line number: %d\n", buffers[i].first_line_number);
            OTT_REENC_TEST_MSG("  line data: %s\n", buffers[i].first_line_content);
            OTT_REENC_TEST_MSG("------------------------------------------------------------\n");
        }
    }
}

// ¸ù¾Ýchunks½âÎö²¢´òÓ¡¼ÓÃÜÊý¾Ý
void parse_and_print_encrypted_data(DataChunk* chunks, int chunk_count, EncryptedBuffer* buffers) {
	int i = 0, j = 0;
    OTT_REENC_TEST_MSG("\nAccording to Chunks to get data:\n");
    OTT_REENC_TEST_MSG("============================================================\n");

    int offset1 = 0; // µÚÒ»¸ö»º³åÇøÆ«ÒÆ
    int offset2 = 0; // µÚ¶þ¸ö»º³åÇøÆ«ÒÆ

    for (i = 0; i < chunk_count; i++) {
        OTT_REENC_TEST_MSG("\nChunk %d result:\n", chunks[i].chunk_id);

        // ´¦ÀíµÚÒ»¸ö»º³åÇøµÄÊý¾Ý
        if (buffers[0].parsed && offset1 < buffers[0].buffer_size) {
            OTT_REENC_TEST_MSG("  µÚÒ»¸öEncrypted BufferÖÐµÄClearÊý¾Ý (%d×Ö½Ú):\n  ", chunks[i].clear_size);
            int clear_end = offset1 + chunks[i].clear_size;
            if (clear_end > buffers[0].buffer_size) clear_end = buffers[0].buffer_size;

            for (j = offset1; j < clear_end && j < offset1 + 128; j++) {
                if ((j - offset1) % 16 == 0 && j > offset1) OTT_REENC_TEST_MSG("\n  ");
                printf("%02X ", buffers[0].data[j]);
            }
            if (clear_end > offset1 + 128)
            OTT_REENC_TEST_MSG("\n");

            int protected_start = offset1 + chunks[i].clear_size;
            int protected_end = protected_start + chunks[i].protected_size;
            if (protected_end > buffers[0].buffer_size) protected_end = buffers[0].buffer_size;

            OTT_REENC_TEST_MSG("  First Encrypted Buffer Protected data(%d bytes:\n  ",
                   protected_end > protected_start ? protected_end - protected_start : 0);
            for (j = protected_start; j < protected_end && j < protected_start + 128; j++) {
                if ((j - protected_start) % 16 == 0 && j > protected_start) OTT_REENC_TEST_MSG("\n  ");
                printf("%02X ", buffers[0].data[j]);
            }
            if (protected_end > protected_start + 128)
            OTT_REENC_TEST_MSG("\n");

            offset1 = protected_start + chunks[i].protected_size;
        }

        // ´¦ÀíµÚ¶þ¸ö»º³åÇøµÄÊý¾Ý
        if (buffers[1].parsed && offset2 < buffers[1].buffer_size) {
            OTT_REENC_TEST_MSG("  Second Encrypted Buffer Clear data(%d bytes)\n  ", chunks[i].clear_size);
            int clear_end = offset2 + chunks[i].clear_size;
            if (clear_end > buffers[1].buffer_size) clear_end = buffers[1].buffer_size;

            for (j = offset2; j < clear_end && j < offset2 + 128; j++) {
                if ((j - offset2) % 16 == 0 && j > offset2) OTT_REENC_TEST_MSG("\n  ");
                printf("%02X ", buffers[1].data[j]);
            }
            if (clear_end > offset2 + 128)
            OTT_REENC_TEST_MSG("\n");

            int protected_start = offset2 + chunks[i].clear_size;
            int protected_end = protected_start + chunks[i].protected_size;
            if (protected_end > buffers[1].buffer_size) protected_end = buffers[1].buffer_size;

            OTT_REENC_TEST_MSG("  Second Encrypted Buffer Protected data(%d bytes):\n  ",
                   protected_end > protected_start ? protected_end - protected_start : 0);
            for (j = protected_start; j < protected_end && j < protected_start + 128; j++) {
                if ((j - protected_start) % 16 == 0 && j > protected_start) OTT_REENC_TEST_MSG("\n  ");
                printf("%02X ", buffers[1].data[j]);
            }
            if (protected_end > protected_start + 128)
                OTT_REENC_TEST_MSG("\n");

            offset2 = protected_start + chunks[i].protected_size;
        }

        OTT_REENC_TEST_MSG("------------------------------------------------------------\n");
    }
}

// ÊÍ·Å»º³åÇøÄÚ´æ
void free_buffers(EncryptedBuffer* buffers) {
	int i = 0;
    for (i = 0; i < 2; i++) {
        if (buffers[i].data) {
            mt_unf_cipher_free(buffers[i].data);
            buffers[i].data = NULL;
        }
    }
}

int write_tohex_file(unsigned char *file_name, unsigned char *p_buff, unsigned int len) {

    FILE *file = fopen(file_name, "wb");
    if (file == NULL) {
        OTT_REENC_TEST_MSG("error: creat failed: %s \n", file_name);
        return 1;
    }

    // ½«Êý×éÐ´Èë¶þ½øÖÆÎÄ¼þ
    size_t written = fwrite(p_buff, 1, len, file);
    if (written != len) {
        OTT_REENC_TEST_MSG("error: write not all data\n");
        fclose(file);
        return 1;
    }

    // ¹Ø±ÕÎÄ¼þ
    fclose(file);
    OTT_REENC_TEST_MSG("write data success %s\n", file_name);
    OTT_REENC_TEST_MSG("write count: %d\n", (int)written);

    return 0;
}

TSecChunkInfo testChunkInfo[32] = {0,};
TSecChunkInfo testOutputChunkInfo[32] = {0,};
TUnsignedInt8 testOutputChunk_IV[32][16] = {0,};

extern TSecFunctionTable *grant_secGetFunctionTable;
extern TSecStreamSession grantTSecStreamSession_0;
extern TSecStreamSession grantTSecStreamSession_1;
extern TTransportSessionId grantTransportSessionId[2];
extern TUnsignedInt8 grantpxKeyId[4][16];
extern TUnsignedInt16 grantKeySlotIndex[4];
extern void test_dump_data(unsigned char *str , unsigned char *data, unsigned int len);

int ott_rec_test_main() {
    DataChunk chunks[MAX_CHUNKS];
    EncryptedBuffer buffers[2];
    int chunk_count = 0;
    char filename[128] = {0,};
    char filename_in_enc[128] = {0,};
    char filename_out_enc[128] = {0,};
    TSecOpaqueInputBufferV1 initOpaqueInput = {0,};
    TSecOpaqueInputBufferV1 initOpaqueOutput = {0,};
    int i = 0, j = 0, ret;


	for (i = 2; i <= 50; i++) {  // ²½³¤Îª2£¬ÒòÎªÊ¾ÀýÖÐÊÇ0,2,4..
#if 0
		sprintf(filename, "/media/sda1/4024_Test_vector_new_20260121/TestVector_4024/402X_testVector_%d.log", i);
             printf("read file: %s\n", filename);
		sprintf(filename_in_enc, "/media/sda1/4024_Test_vector_new_20260121/4024_out/4024_in_encVector_%d.bin", i);
		sprintf(filename_out_enc, "/media/sda1/4024_Test_vector_new_20260121/4024_out/4024_out_encVector_%d.bin", i);
#else
             sprintf(filename, "/media/sda1/402A_Test_vector_new_20260130/TestVector_402A/402X_testVector_%d.log", i);
             printf("read file: %s\n", filename);
		sprintf(filename_in_enc, "/media/sda1/402A_Test_vector_new_20260130/402A_out/402A_in_encVector_%d.bin", i);
		sprintf(filename_out_enc, "/media/sda1/402A_Test_vector_new_20260130/402A_out/402A_out_encVector_%d.bin", i);
#endif
             chunk_count = 0;
		chunk_count = parse_all_chunks(chunks, filename);
             OTT_REENC_TEST_MSG("\n");
		if (chunk_count < 0) {
			OTT_REENC_TEST_MSG("Get Chunk failed\n");
			return 1;
		}
             OTT_REENC_TEST_MSG("\n");
		if (chunk_count == 0) {
			OTT_REENC_TEST_MSG("There is no ChunkCount data \n");
			return 0;
		}
             OTT_REENC_TEST_MSG("\n");
		int buffer_count = parse_encrypted_buffers(filename, buffers);
		if (buffer_count < 0) {
			OTT_REENC_TEST_MSG("get failed\n");
			return 1;
		}
             OTT_REENC_TEST_MSG("\n");
		//print_chunks(chunks, chunk_count);
		OTT_REENC_TEST_MSG("\n thera are %d encryption buffers \n", buffer_count);

/*
		print_buffer_first_lines(buffers, buffer_count);

		parse_and_print_encrypted_data(chunks, chunk_count, buffers);
*/
             memset(buffers[1].data, 0x00, buffers[1].buffer_size);
            for (j = 0; j < chunk_count; j++) {
                OTT_REENC_TEST_MSG("chunk j = %d", j);
                testChunkInfo[j].clearSize = chunks[j].clear_size;
                testChunkInfo[j].protSize = chunks[j].protected_size;
                if (strlen(chunks[j].iv_str) > 0) {
                    testChunkInfo[j].initVector = chunks[j].iv;
                    testChunkInfo[j].initVectorSize = 16;
                    OTT_REENC_TEST_MSG("set IV");
                } else {
                    testChunkInfo[j].initVector = NULL;
                    testChunkInfo[j].initVectorSize = 0;
                    OTT_REENC_TEST_MSG("IV is NULL");
                }
            }

            initOpaqueInput.version = 1;
            initOpaqueInput.data = buffers[0].data;
            initOpaqueInput.size = buffers[0].buffer_size;
            initOpaqueInput.chunks = &testChunkInfo[0];
            initOpaqueInput.chunkCount = chunk_count;
            initOpaqueInput.skipBlocks = chunks[0].skip_blocks;
            initOpaqueInput.cryptBlocks = chunks[0].crypto_blocks;

            testOutputChunkInfo[0].initVector = testOutputChunk_IV[0];
            initOpaqueOutput.data = buffers[1].data;
            memset(initOpaqueOutput.data, 0x00, buffers[1].buffer_size);
            initOpaqueOutput.chunks = &testOutputChunkInfo[0];

            //test_dump_data("input", initOpaqueInput.data, 333);
#if 1  //0x402A
        #if 0

        #else
            ret = grant_secGetFunctionTable->secStreamReEncryptSession->processOpaqueData(grantTSecStreamSession_0, grantKeySlotIndex[0], &initOpaqueInput, &initOpaqueInput,
                NULL, 0, TRUE);
            OTT_REENC_TEST_MSG("ver=%d : addr=%p, size=%d, chk=%p, coun=%d, skip=%d, cryp=%d\n", initOpaqueInput.version, initOpaqueInput.data,
                    initOpaqueInput.size, initOpaqueInput.chunks, initOpaqueInput.chunkCount, initOpaqueInput.skipBlocks, initOpaqueInput.cryptBlocks);
            OTT_REENC_TEST_MSG("clear = %d, protSize=%d, iv_size=%d", initOpaqueInput.chunks->clearSize, initOpaqueInput.chunks->protSize, initOpaqueInput.chunks->initVectorSize);
            test_dump_data("IV", initOpaqueInput.chunks->initVector, 16);
            OTT_REENC_TEST_MSG("write file 0: %s, size = %d\n", filename_in_enc, buffers[0].buffer_size);
		write_tohex_file(filename_in_enc, buffers[0].data, buffers[0].buffer_size);
        #endif
            OTT_REENC_TEST_MSG("Grant test end ret = 0x%x\n", ret);
#else
            #if 0
            //0x4024
            ret = grant_secGetFunctionTable->secStreamReEncryptSession->processOpaqueData(grantTSecStreamSession_1, grantKeySlotIndex[1], &initOpaqueInput, &initOpaqueOutput,
                NULL, 0, TRUE);
            OTT_REENC_TEST_MSG("ver=%d : addr=%p, size=%d, chk=%p, coun=%d, skip=%d, cryp=%d\n", initOpaqueOutput.version, initOpaqueOutput.data,
                    initOpaqueOutput.size, initOpaqueOutput.chunks, initOpaqueOutput.chunkCount, initOpaqueOutput.skipBlocks, initOpaqueOutput.cryptBlocks);
            OTT_REENC_TEST_MSG("clear = %d, protSize=%d, iv_size=%d", initOpaqueInput.chunks->clearSize, initOpaqueInput.chunks->protSize, initOpaqueInput.chunks->initVectorSize);
            test_dump_data("IV", initOpaqueInput.chunks->initVector, 16);
            OTT_REENC_TEST_MSG("write file 0: %s, size = %d\n", filename_in_enc, buffers[0].buffer_size);
	        write_tohex_file(filename_in_enc, buffers[0].data, buffers[0].buffer_size);

		OTT_REENC_TEST_MSG("write file 1: %s, size = %d\n", filename_out_enc, buffers[1].buffer_size);
		write_tohex_file(filename_out_enc, buffers[1].data, buffers[1].buffer_size);
        #else
            ret = grant_secGetFunctionTable->secStreamReEncryptSession->processOpaqueData(grantTSecStreamSession_1, grantKeySlotIndex[1], &initOpaqueInput, &initOpaqueInput,
                NULL, 0, TRUE);
            OTT_REENC_TEST_MSG("ver=%d : addr=%p, size=%d, chk=%p, coun=%d, skip=%d, cryp=%d\n", initOpaqueInput.version, initOpaqueInput.data,
                    initOpaqueInput.size, initOpaqueInput.chunks, initOpaqueInput.chunkCount, initOpaqueInput.skipBlocks, initOpaqueInput.cryptBlocks);
            OTT_REENC_TEST_MSG("clear = %d, protSize=%d, iv_size=%d", initOpaqueInput.chunks->clearSize, initOpaqueInput.chunks->protSize, initOpaqueInput.chunks->initVectorSize);
            test_dump_data("IV", initOpaqueInput.chunks->initVector, 16);
            OTT_REENC_TEST_MSG("write file 0: %s, size = %d\n", filename_in_enc, buffers[0].buffer_size);
		write_tohex_file(filename_in_enc, buffers[0].data, buffers[0].buffer_size);
        #endif

#endif





		free_buffers(buffers);
	}
    return 0;
}
#endif
