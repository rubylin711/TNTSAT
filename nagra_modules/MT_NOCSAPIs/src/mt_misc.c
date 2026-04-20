#include <pthread.h>
#include "ca_cert_impl.h"

#include "mt_unf_otp.h"

#include "mt_sec_ext.h"
#include "mt_unf_otp.h"

static pthread_mutex_t g_otp_op_mutex = PTHREAD_MUTEX_INITIALIZER;
#define otp_lock() (void) pthread_mutex_lock(&g_otp_op_mutex);
#define otp_unlock() (void) pthread_mutex_unlock(&g_otp_op_mutex);

MT_S32 mt_otp_read_byte(MT_U32 addr, MT_U32 len, MT_U8 * data)
{
	MT_S32 ret = MT_SUCCESS;
	MT_U32 val = 0;
	MT_U32 i;

	otp_lock();

	MT_UNF_OTP_Init();

	if (MT_UNF_OTP_read(addr << 3, len << 3, &val) != MT_SUCCESS) {
		ret = MT_FAILURE;
		goto OUT;
	}

	for (i = 0; i < len; i++) {
		data[i] = (val >> (8 * i)) & 0xff;
	}

      OUT:
	MT_UNF_OTP_Deinit();

	otp_unlock();

	return ret;
}

MT_S32 mt_otp_write_byte(MT_U32 addr, MT_U32 len, MT_U8 * data)
{
	MT_S32 ret = MT_SUCCESS;
	MT_U32 val = 0;
	MT_U32 i;

	otp_lock();

	MT_UNF_OTP_Init();

	for (i = 0; i < len; i++) {
		val |= data[i] << (8 * i);
	}

	if (MT_UNF_OTP_write(addr << 3, len << 3, val) != MT_SUCCESS) {
		ret = MT_FAILURE;
		goto OUT;
	}

      OUT:
	MT_UNF_OTP_Deinit();

	otp_unlock();

	return ret;
}
