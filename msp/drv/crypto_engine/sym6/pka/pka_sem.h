/***************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/***************************************************************************/
#ifndef _PKA_SEM_H_
#define _PKA_SEM_H_

enum PKA_SEM_CPU_ID {
	PKA_SEM_CPU_ID_REECPU = 0x8,
	PKA_SEM_CPU_ID_TEECPU = 0xd,
	PKA_SEM_CPU_ID_VSCPU = 0xc,

	PKA_SEM_CPU_ID_MASK = 0xf
};

enum PKA_SEM_REQ_TIME {
	PKA_SEM_REQ_TIME_MAXIMUM = 0,	/*About 28 seconds. */
	PKA_SEM_REQ_TIME_IMMIDIATE = 1,
};

extern uint8_t sem_cpu_id;

mt_s32 pka_sem_wait(mt_u8 cpu_id, mt_u32 req_time);

mt_s32 pka_sem_post(void);

mt_s32 pka_sem_is_free(void);

#endif /* end of include guard: _PKA_SEM_H_ */
