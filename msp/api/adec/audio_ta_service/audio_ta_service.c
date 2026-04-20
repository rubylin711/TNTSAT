/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/


#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <pthread.h>

#include <tee_client_api.h>
#include <tee_client_api_extensions.h>

#include "mt_module.h"
#include "mt_mpi_adec.h"
#include "mt_error_mpi.h"
#include "mt_module_debug.h"

#include "adec/adec_api.h"

#include <linux/dma-heap.h>
#include <linux/dma-buf.h>

#define TA_ADEC_UUID \
	{ 0xea38a79f, 0xb962, 0x487c, \
	{ 0x86, 0xd3, 0xb3, 0xd1, 0xd9, 0x43, 0x83, 0xf9} }

int fd_adec;
int fd_mem_sec = -1;
int fd_sdp = -1;

TEEC_SharedMemory aud_shm;
TEEC_SharedMemory *adec_sdp_buf = NULL;
TEEC_Context ctx;
TEEC_Session sess;
pthread_t g_adec_ta_open_tid;

aud_share_buf_t * g_aud_share_buf;
unsigned long g_aud_share_phy_addr;
aud_buf_param_t aud_buf_param;

static uint32_t es_buf_size = 0;
static uint32_t sw_buf_size = 0;

static int allocate_dma_buffer(size_t size, const char *heap_name)
{
	const char *default_dev = "/dev/dma_heap/sdp";
	char *mem_sec_dev = (char *)default_dev;
	struct dma_heap_allocation_data data = { 0 };

	if (heap_name != NULL)
		mem_sec_dev = (char *)heap_name;

	fd_mem_sec = open(mem_sec_dev, O_RDWR | O_SYNC);
	if (fd_mem_sec == -1) {
		fprintf(stderr, "Error: failed to open %s\n", mem_sec_dev);
		printf("Seems no DMA buf heap is available.\n");
		return -1;
	}

	data.len = size;
	data.fd_flags = O_RDWR | O_CLOEXEC;
	data.heap_flags = 0;

	if (ioctl(fd_mem_sec, DMA_HEAP_IOCTL_ALLOC, &data) == -1) {
		fprintf(stderr, "Error: DMA buf allocate API failed\n");
		goto out;
	}

	return data.fd;
out:
	close(fd_mem_sec);
	fd_mem_sec = -1;
	return -1;
}

TEEC_SharedMemory g_tmpshm;

static int tee_register_buffer(TEEC_Context *context, void **shm_ref)
{
	TEEC_Result teerc = TEEC_ERROR_GENERIC;
	memset((void*) &g_tmpshm , 0, sizeof(g_tmpshm));

	g_tmpshm.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;

	printf("<%s:%d>TEEC_RegisterSharedMemory IN context %p g_tmpshm %p , fd %d \n", 
		__func__, __LINE__, context, &g_tmpshm, fd_sdp );

	teerc = TEEC_RegisterSharedMemoryFileDescriptor(context, &g_tmpshm, fd_sdp);
	if (teerc != TEEC_SUCCESS) {
		fprintf(stderr, "Error: TEEC_RegisterMemoryFileDescriptor() failed %x\n",
				teerc);
		return 1;
	}
	*shm_ref = &g_tmpshm;
	return 0;
}

static int service_init_ta(void)
{
	TEEC_Result res;
	TEEC_UUID uuid = TA_ADEC_UUID;
	uint32_t err_origin;
	
	if(-1 == fd_sdp) {
		/* Initialize a context connecting us to the TEE */
		res = TEEC_InitializeContext(NULL, &ctx);
		if (res != TEEC_SUCCESS)
		{
			errx(1, "TEEC_InitializeContext failed with code 0x%x", res);
		}
	}
	/*
	 * Open a session to the "hello world" TA, the TA will print "hello
	 * world!" in the log when the session is created.
	 */
	res = TEEC_OpenSession(&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
	{
		if (0 != ioctl(fd_adec, IO_ADEC_TEE_DISABLE, NULL))
		{
			printf("IO_ADEC_TEE_DISABLE error!");
		}
		else
		{
			printf("NO audio TA IO_ADEC_TEE_DISABLE Done!");
		}
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x", res, err_origin);
	}

	return 0;
}

static int service_init_share_buf(void)
{
	uint32_t alloc_size = 0;
	int res;
	
	if(-1 != fd_sdp) {
		return 0;
	}
	
	g_aud_share_buf = (aud_share_buf_t *)mt_mem_map_cache((phys_addr_t)aud_buf_param.share_buf_phy, sizeof(aud_share_buf_t));
	
	g_aud_share_buf->es_buf.data_usr_vir = (u64)(ulong)mt_mem_map_cache((phys_addr_t)aud_buf_param.es_buf_phy, es_buf_size);
	g_aud_share_buf->ad_es_buf.data_usr_vir = g_aud_share_buf->es_buf.data_usr_vir + aud_buf_param.aud_buf_size_map.es_buf_size;
	
	g_aud_share_buf->snd_sw_buf[0].base_usr_vir = (u64)(ulong)mt_mem_map_cache((phys_addr_t)aud_buf_param.snd_buf_phy, sw_buf_size);
	g_aud_share_buf->snd_sw_buf[1].base_usr_vir = g_aud_share_buf->snd_sw_buf[0].base_usr_vir + aud_buf_param.aud_buf_size_map.snd_sw_buf0_size;
	g_aud_share_buf->snd_sw_buf[2].base_usr_vir = g_aud_share_buf->snd_sw_buf[0].base_usr_vir + aud_buf_param.aud_buf_size_map.snd_sw_buf1_size;

	aud_shm.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
	aud_shm.buffer = (void *)g_aud_share_buf;
	aud_shm.size = sizeof(aud_share_buf_t);
	g_aud_share_buf = (aud_share_buf_t *)aud_shm.buffer;
	if (NULL == aud_shm.buffer)
	{
		printf("<ca> mt_mem_map share_buf error!\n");
		return -1;
	}
	res = TEEC_RegisterSharedMemory(&ctx, &aud_shm);
	if (0 != res)
	{
		printf("<%s:%d>TEEC_RegisterSharedMemory error!\n", __func__, __LINE__);
		return -1;
	}

	/*
	 * Execute a function in the TA by invoking it, in this case
	 * we're setting the allocated dma-buf with some magic value,
	 * and expecting another value modified by TA.
	 *
	 * The value of command ID part and how the parameters are
	 * interpreted is part of the interface provided by the TA.
	 */
	alloc_size = es_buf_size + sw_buf_size;
	
	if(-1 == fd_sdp) {
		fd_sdp = allocate_dma_buffer(alloc_size, NULL);
		if (fd_sdp < 0) {
			printf("SDP alloc failed (%d bytes) in /dev/dma_heap/sdp: %d\n",
				   alloc_size, fd_sdp);
		}
		
		/* register this dma-buf to TEE by fd */
		if (tee_register_buffer(&ctx, (void **)&adec_sdp_buf)) {
			printf("%s - %d : Register share memory dma-buf file handler issue!\n",
				   __func__, __LINE__);
		}
	}


	return 0;
}

static void* service_adec_ta_open_thread(void *args)
{
	TEEC_Result ret;
	TEEC_Operation op = {0};
	uint32_t err_origin;

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_PARTIAL_INOUT, TEEC_MEMREF_PARTIAL_INOUT, TEEC_MEMREF_PARTIAL_INOUT, TEEC_MEMREF_TEMP_INPUT);
	op.params[0].memref.parent = &aud_shm;
	op.params[0].memref.offset = 0;
	op.params[0].memref.size = aud_shm.size;

	op.params[1].memref.parent = adec_sdp_buf;
	op.params[1].memref.offset = 0;
	op.params[1].memref.size = es_buf_size;

	//printf("es size:0x%x, ad es size:0x%x!!!\n",aud_buf_param.aud_buf_size_map.es_buf_size, aud_buf_param.aud_buf_size_map.ad_es_buf_size);
	op.params[2].memref.parent = adec_sdp_buf;
	op.params[2].memref.offset = es_buf_size;
	op.params[2].memref.size = sw_buf_size;

	op.params[3].tmpref.buffer = (void *)&aud_buf_param.aud_buf_size_map;
	op.params[3].tmpref.size = sizeof(aud_buf_size_map_t);

	//printf("<ca>%d %d\n", sizeof(aud_share_buf_t), sizeof(adec_info_t));
	ret = TEEC_InvokeCommand(&sess, TA_ADEC_OPEN, &op, &err_origin);
	if (ret != TEEC_SUCCESS)
	{
		printf("<%s:%d>TEEC_InvokeCommand failed with code 0x%x origin 0x%x\n", __func__, __LINE__, ret, err_origin);
		
	}

	return NULL;
}

static int start_audio_ta(void)
{
	int wait_count = 0;

	fd_adec = open("/dev/adec", O_RDWR | O_CLOEXEC);
	if (fd_adec < 0)
	{
		printf("<%s:%d> open adec error!\n", __func__, __LINE__);
		return -1;
	}
	else
	{
		printf("<ah><%s:%d> open /dev/adec success!\n", __func__, __LINE__);
	}
	
	if (0 != ioctl(fd_adec, IO_ADEC_GET_AUD_MEM, &aud_buf_param))
	{
		printf("IO_ADEC_GET_AUD_MEM error!");
		return -1;
	}

	es_buf_size = aud_buf_param.aud_buf_size_map.es_buf_size + aud_buf_param.aud_buf_size_map.ad_es_buf_size;
	sw_buf_size = aud_buf_param.aud_buf_size_map.snd_sw_buf0_size +
							aud_buf_param.aud_buf_size_map.snd_sw_buf1_size + 
							aud_buf_param.aud_buf_size_map.snd_sw_buf2_size;

	printf("ta_enable %d !\n", aud_buf_param.ta_enabled);

	if(aud_buf_param.ta_enabled >= 1)
	{
		errx(1, "audio ta allready started exit\n");
	}

	service_init_ta();
	service_init_share_buf();

	if (0 != pthread_create(&g_adec_ta_open_tid, NULL, service_adec_ta_open_thread, (void *)NULL))
	{
		printf("<%s:%d>create adec_ta_open_thread error!\n", __func__, __LINE__);
		return -1;
	}

	while (g_aud_share_buf->adec_info.status <  1)
	{
		usleep(1000);
		wait_count++;
		if(wait_count > 400)
		{
			break;
		}
	}
	printf("<ca> wait ta ready! sta %d  wait_count %d \n", g_aud_share_buf->adec_info.status, wait_count);
	if (0 != ioctl(fd_adec, IO_ADEC_TEE_ENABLE, NULL))
	{
		printf("IO_ADEC_TEE_ENABLE error! \n");
		return -1;
	}

	printf("IO_ADEC_TEE_ENABLE Done! \n");
    return 0;
}
static void sig_fun(int sig)
{
	if (SIGINT == sig || SIGTERM == sig)
	{
		printf("signal SIGINT\n");
		if (fd_mem_sec != -1) {
			close(fd_mem_sec);
			fd_mem_sec = -1;
		}
		
		close(fd_adec);
		TEEC_ReleaseSharedMemory(&aud_shm);

		TEEC_CloseSession(&sess);

		TEEC_FinalizeContext(&ctx);
		pthread_join(g_adec_ta_open_tid, NULL);
		fd_adec = -1;
		exit(0);
	}

	if(SIGUSR1 == sig)
	{
		printf("signal SIGUSR1 fd_adec %d \n", fd_adec);
		if(fd_adec < 0)
		{	
			return;
		}
		if (fd_mem_sec != -1) {
			close(fd_mem_sec);
			fd_mem_sec = -1;
		}
		close(fd_adec);

	//	TEEC_ReleaseSharedMemory(&aud_shm);  // casue memrory leak  bug: 29428
		TEEC_CloseSession(&sess);

	//	TEEC_FinalizeContext(&ctx);   // casue memrory leak  bug: 29428
		pthread_join(g_adec_ta_open_tid, NULL);
		printf("close audio ta done \n");
		fd_adec = -1;
	}
	else if(SIGUSR2 == sig)
	{
		printf("signal SIGUSR2 fd_adec %d \n", fd_adec);
		if(fd_adec < 0)
		{	
			start_audio_ta();
		}
	}
}
mt_s32 mt_module_init(mt_void);

int main(int argc, char *argv[])
{
	signal(SIGINT, sig_fun);	// ctrl-c
	signal(SIGTERM, sig_fun);
	signal(SIGUSR1, sig_fun);
	signal(SIGUSR2, sig_fun);
	
	(void)mt_module_init();
	start_audio_ta();
	while (1 )
	{
		sleep(1);
	}
 
	return 0;
}
