/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_type.h"
#include "mtos_debug.h"

//#ifndef debug_data_info_t
//#define debug_data_info_t void
//#endif

#if 0
void os_debug_init(debug_mem_t *p_mem_info)
{
    return;
}
#else

void os_debug_init(void *p_mem_info)
{
    return;
}

#endif

int debug_put_stack_info(u32 prio, u32 stack_ptr, u32 stack_size)
{
    return 0;
}

int debug_get_stack_info(debug_data_info_t *p_info)
{
    return 0;
}

int debug_get_mem_info(debug_data_info_t *p_info)
{
    return 0;
}

int debug_put_common_print(u8 *p_buff, u32 len)
{
    return 0;
}
#if 0
int debug_get_common_print(debug_data_info_t *p_info)
{
    return 0;
}
#else
int debug_get_common_print(void *p_info)
{
    return 0;
}
#endif

int debug_print_msgq_info(u8 msg_id, void *p_msg, u8 send, u8 msg_left,
                          u8 msg_qin, u8 msg_qout, u8 buf_in, u8 buf_out)
{
    return 0;
}

#if 0
int debug_get_msgq_print(debug_data_info_t *p_info)
{
    return 0;
}
#else
int debug_get_msgq_print(void *p_info)
{
    return 0;
}
#endif

int debug_put_task_name(u32 prio, u8 *p_task_name)
{
    return 0;
}

int debug_put_task_info(u32 prio, u32 cpu_usage, u32 switch_cnt)
{
    return 0;
}

#if 0
int debug_get_os_info(debug_data_info_t *p_info)
{
    return 0;
}
#else
int debug_get_os_info(void *p_info)
{
    return 0;
}
#endif

void os_dead_loop(void)
{
    return;
}
