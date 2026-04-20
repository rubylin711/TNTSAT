/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <string.h>
#include "mt_type.h"
#include "mtos_msg.h"
#include "mtos_sem.h"
#include "mtos_printk.h"
#include "mtos_mem.h"

#include "drv_os.h"

#define MSGQ_MAX_NAME_LEN (16)

typedef enum {
    EXT_IN = 0,
    EXT_OUT,
} msgq_ext_buf_drct_t;

typedef struct
{
    u8 used;       /* flag, indicate if a buffer is used */
    s8 *p_ref;     /* refernce array, indicate the number of current user */
    u16 atom_size; /* size of a single atom buffer section */
    u16 depth;     /* the max atom count */
    u16 in;        /* pointer to current input atom buffer index */
    u16 out;       /* pointer to current output atom buffer index */
    ulong addr;      /* base address of current attached buffer */
} msgq_ext_buf_ctrl_t;

typedef struct
{
    u8 used;
    u8 name[MSGQ_MAX_NAME_LEN];
    u32 msgq_id;
    os_sem_t sync_sem;
    msgq_ext_buf_ctrl_t ext_buf[MTOS_MSGQ_MAX_BUF];
} msgq_ctrl_t;

static msgq_ctrl_t msgq_ctrl[MAX_SYSMSGQNUM];

static void _msgq_str_copy(u8 *p_dest, u16 max_len, const u8 *p_src)
{
    u16 len = 0;

    if ((p_dest == NULL) || (max_len == 0) || (p_src == NULL)) {
	return;
    }

    len = (u16)strlen((const char *)p_src);
    len ++;
    len = ((len < max_len) ? len : max_len);
    memcpy(p_dest, p_src, (len * sizeof(u8)));
    p_dest[len-1] = 0;

    return;
}

static u32 _msgq_find_idle(void)
{
    u32 i = 0;

    for (i = 0; i < MAX_SYSMSGQNUM; i++) {
	if (msgq_ctrl[i].used == 0) {
	    break;
	}
    }

    return i;
}

static s8 *_msgq_ext_find_ref_id(msgq_ext_buf_ctrl_t *p_ext, void *p_addr)
{
    u16 i = 0;

    //check param
    if ((p_ext == NULL) || (p_addr == NULL)) {
	mtos_printk("[_msgq_ext_find_ref_id] param is ERROR!\n");
	return NULL;
    }

    for (i = 0; i < p_ext->depth; i++) {
	if ((p_ext->addr + (ulong)(i * p_ext->atom_size)) == (ulong)p_addr)
	    return &(p_ext->p_ref[i]);
    }
    return NULL;
}

static u8 _msgq_ext_find_id(msgq_ext_buf_ctrl_t *p_exts, u16 atom_size)
{
    u8 i = 0;
    u8 id = MTOS_MSGQ_MAX_BUF;

    //check param
    if ((p_exts == NULL) || (atom_size == 0)) {
	mtos_printk("[_msgq_ext_find_id] param is ERROR!\n");
	return id;
    }

    for (i = 0; i < MTOS_MSGQ_MAX_BUF; i++) {
	if ((atom_size == p_exts[i].atom_size) && p_exts[i].used) {
	    id = i;
	    break;
	}
    }

    return id;
}

static ulong _msgq_ext_find_addr(msgq_ext_buf_ctrl_t *p_ext, msgq_ext_buf_drct_t drct)
{
    u16 index = 0;

    //check param
    if (p_ext == NULL) {
	mtos_printk("[_msgq_ext_find_addr] param is ERROR!\n");
	return 0;
    }

    if (drct == EXT_IN) {
	index = p_ext->in;
    } else if (drct == EXT_OUT) {
	index = p_ext->out;
    } else {
	mtos_printk("[_msgq_ext_find_addr] drct is ERROR!\n");
	return 0;
    }

    return (ulong)(p_ext->addr + (ulong)(index * p_ext->atom_size));
}

static void _msgq_ext_use(msgq_ext_buf_ctrl_t *p_exts,
                          void *p_buf,
                          u16 atom_size,
                          u16 depth)
{
    u8 i = 0;

    //check param
    if ((p_exts == NULL) || (p_buf == NULL) || (atom_size == 0) || (depth == 0)) {
	mtos_printk("[_msgq_ext_use] param is ERROR!\n");
	return;
    }

    for (i = 0; i < MTOS_MSGQ_MAX_BUF; i++) {
	if (1 == p_exts[i].used) {
	    if (p_exts[i].atom_size == atom_size) {
		mtos_printk("[_msgq_ext_use] ext buffer has already used!\n");
		return;
	    }
	}
    }

    for (i = 0; i < MTOS_MSGQ_MAX_BUF; i++) {
	if (0 == p_exts[i].used) {
	    u16 ref_depth = 0;

	    //mtos_task_lock();
	    p_exts[i].used = 1;
	    p_exts[i].addr = (ulong)p_buf;
	    p_exts[i].atom_size = atom_size;
	    p_exts[i].depth = depth;
	    //mtos_task_unlock();
	    ref_depth = (u16)((depth + 7) / 8 * 8);
	    p_exts[i].p_ref = (s8 *)mtos_malloc((u32)ref_depth);
	    memset(p_exts[i].p_ref, 0, ref_depth);
	    break;
	}
    }

    if (i >= MTOS_MSGQ_MAX_BUF) {
	mtos_printk("[_msgq_ext_use] no free ext buffer!\n");
    }

    return;
}

static void _msgq_ext_unuse(msgq_ext_buf_ctrl_t *p_exts, u16 atom_size)
{
    u8 id = MTOS_MSGQ_MAX_BUF;

    //check param
    if ((p_exts == NULL) || (atom_size == 0)) {
	mtos_printk("[_msgq_ext_discard] param is ERROR!\n");
	return;
    }

    id = _msgq_ext_find_id(p_exts, atom_size);
    if ((id >= MTOS_MSGQ_MAX_BUF) || (p_exts[id].in != p_exts[id].out)) {
	mtos_printk("[_msgq_ext_discard] ext buffer unuse ERROR!\n");
	return;
    }

    //mtos_task_lock();
    p_exts[id].used = 0;
    p_exts[id].addr = 0;
    p_exts[id].atom_size = 0;
    p_exts[id].depth = 0;
    p_exts[id].in = 0;
    p_exts[id].out = 0;
    //mtos_task_unlock();
    if (p_exts[id].p_ref != NULL) {
	mtos_free(p_exts[id].p_ref);
	p_exts[id].p_ref = NULL;
    }

    return;
}

static MT_BOOL _msgq_ext_enter(msgq_ext_buf_ctrl_t *p_exts, os_msg_t *p_msg)
{
    u8 id = 0;
    ulong addr = 0;
    s8 *p_ref = NULL;
    msgq_ext_buf_ctrl_t *p_ext = NULL;

    //check param
    if ((p_exts == NULL) || (p_msg == NULL)) {
	mtos_printk("[_msgq_ext_enter] param is ERROR!\n");
	return FALSE;
    }

    id = _msgq_ext_find_id(p_exts, (u16)p_msg->para2);
    if (id >= MTOS_MSGQ_MAX_BUF) {
	mtos_printk("[_msgq_ext_enter] find ext buffer id ERROR!\n");
	return FALSE;
    }

    p_ext = &(p_exts[id]);
    addr = _msgq_ext_find_addr(p_ext, EXT_IN);
    p_ref = _msgq_ext_find_ref_id(p_ext, (void *)addr);
    if ((p_ref == NULL) || (*p_ref) > 0) {
	mtos_printk("[_msgq_ext_enter] ref is ERROR!\n");
	return FALSE;
    }

    memcpy((void *)addr, (void *)p_msg->para1, p_msg->para2);
    p_ext->in = (u16)((p_ext->in + 1) % p_ext->depth);
    p_msg->para1 = addr;

    return TRUE;
}

static MT_BOOL _msgq_ext_exit(msgq_ext_buf_ctrl_t *p_exts, os_msg_t *p_msg)
{
    u8 id = 0;
    msgq_ext_buf_ctrl_t *p_ext = NULL;

    //check param
    if ((p_exts == NULL) || (p_msg == NULL)) {
	mtos_printk("[_msgq_ext_exit] param is ERROR!\n");
	return FALSE;
    }

    id = _msgq_ext_find_id(p_exts, (u16)p_msg->para2);
    if (id >= MTOS_MSGQ_MAX_BUF) {
	mtos_printk("[_msgq_ext_exit] find ext buffer id ERROR!\n");
	return FALSE;
    }

    p_ext = &(p_exts[id]);
    if (p_msg->para1 != _msgq_ext_find_addr(p_ext, EXT_OUT)) {
	//MT_ASSERT(0);
	mtos_printk("[_msgq_ext_exit] addr != out!\n");
    }

    p_ext->out = (u16)((p_ext->out + 1) % p_ext->depth);

    return TRUE;
}

MT_BOOL mtos_message_set_parser(u32 msgq_id, msg_parser_t p_parser)
{
    return TRUE;
}

MT_BOOL mtos_message_init(void)
{
    (void)SYS_MsgQInit();
    memset(msgq_ctrl, 0, (MAX_SYSMSGQNUM * sizeof(msgq_ctrl_t)));
    return TRUE;
}

u32 mtos_messageq_create(u32 depth, u8 *p_name)
{
    u32 index = 0;
    MT_BOOL ret = FALSE;
    ErrorCode_t err = ERROR_CODE_ERROR_RESULT;
#ifdef DRV_SEM_DEBUG
    u8 name[64];
#endif

    //check param
    if (depth == 0) {
	mtos_printk("[mtos_messageq_create] param is ERROR!\n");
	return (u32)INVALID;
    }

    //find idle msgq
    index = _msgq_find_idle();
    if (index == MAX_SYSMSGQNUM) {
	mtos_printk("[mtos_messageq_create] messageq is FULL!\n");
	return (u32)INVALID;
    }

//create sync sem
#ifdef DRV_SEM_DEBUG
    memset(name, 0, (sizeof(u8) * 64));
    strcpy((char *)name, (const char *)"mtos_messageq_create(");
    //FIXME
    strcat((char *)name, (const char *)p_name);
    strcat((char *)name, "):sync_sem");
    ret = mtos_sem_create(&(msgq_ctrl[index].sync_sem), FALSE, (const u8 *)name);
#else
    ret = mtos_sem_create(&(msgq_ctrl[index].sync_sem), FALSE);
#endif
    if (ret != TRUE) {
	mtos_printk("[mtos_messageq_create] create sem ERROR!\n");
	return (u32)INVALID;
    }

    //create msgq
    err = SYS_MsgQCreate(sizeof(os_msg_t), depth, &(msgq_ctrl[index].msgq_id));
    if (err != ERROR_CODE_NO_ERROR) {
	mtos_printk("[mtos_messageq_create] create messageq ERROR!\n");
	return (u32)INVALID;
    }

    //set msgq name
    if (p_name != NULL) {
	_msgq_str_copy(msgq_ctrl[index].name, MSGQ_MAX_NAME_LEN, p_name);
    }

    msgq_ctrl[index].used = 1;

    return index;
}

MT_BOOL mtos_messageq_send(u32 msgq_id, os_msg_t *p_msg)
{
    ErrorCode_t err = ERROR_CODE_ERROR_RESULT;

    //check param
    if ((msgq_id == INVALID) || (msgq_id > MAX_SYSMSGQNUM) || (p_msg == NULL)) {
	mtos_printk("[mtos_messageq_send] param is ERROR!\n");
	return FALSE;
    }

    if ((p_msg->is_ext) && (!p_msg->is_sync)) {
	MT_BOOL ret = FALSE;

	ret = _msgq_ext_enter(msgq_ctrl[msgq_id].ext_buf, p_msg);
	if (ret != TRUE) {
	    mtos_printk("[mtos_messageq_send] ext buffer enter ERROR!\n");
	    return FALSE;
	}
    }

    err = SYS_MsgQSend((const void *)p_msg, msgq_ctrl[msgq_id].msgq_id, 0);
    if (err != ERROR_CODE_NO_ERROR) {
	mtos_printk("[mtos_messageq_send] send message(%s) ERROR!\n", msgq_ctrl[msgq_id].name);
	if (p_msg->is_ext) {
	    _msgq_ext_exit(msgq_ctrl[msgq_id].ext_buf, p_msg);
	}
	return FALSE;
    }

    if (p_msg->is_sync) {
        //mtos_printk("\n##send[%d]sync msg[0x%x, 0x%x, 0x%x]\n", msgq_id, p_msg->content, p_msg->para1, p_msg->para2);
    	if (!mtos_sem_take(&(msgq_ctrl[msgq_id].sync_sem), 0)){
          mtos_printk("\n##mtos_messageq_send[%d]sync msg mtos_sem_take fail\n", msgq_id);
          //MT_USLEEP(3000000);
    	}
      //mtos_printk("\n##get[%d]sync msg ack[0x%x, 0x%x, 0x%x]\n", msgq_id, p_msg->content, p_msg->para1, p_msg->para2);
    }

    return TRUE;
}

MT_BOOL mtos_messageq_receive(u32 msgq_id, os_msg_t *p_msg, u32 ms)
{
    ErrorCode_t err = ERROR_CODE_ERROR_RESULT;

    //check param
    if ((msgq_id >= MAX_SYSMSGQNUM) || (p_msg == NULL)) {
	mtos_printk("[mtos_messageq_receive] param is ERROR!\n");
	return FALSE;
    }
    memset(p_msg, 0, sizeof(os_msg_t));
    err = SYS_MsgQWait((void *)p_msg, msgq_ctrl[msgq_id].msgq_id, ms);
    if (err != ERROR_CODE_NO_ERROR) {
	//mtos_printk("[mtos_messageq_receive] receive message ERROR!\n");
	return FALSE;
    }

    if ((p_msg->is_ext) && (!p_msg->is_sync)) {
	MT_BOOL ret = FALSE;

	ret = _msgq_ext_exit(msgq_ctrl[msgq_id].ext_buf, p_msg);
	if (ret != TRUE) {
	    mtos_printk("[mtos_messageq_receive] ext buffer exit ERROR!\n");
	    return FALSE;
	}
    }

    return TRUE;
}

void mtos_messageq_ack(u32 msgq_id)
{
    //check param
    if (msgq_id >= MAX_SYSMSGQNUM) {
	mtos_printk("[mtos_messageq_ack] param is ERROR!\n");
	return;
    }

    (void)mtos_sem_give(&(msgq_ctrl[msgq_id].sync_sem));

    return;
}

MT_BOOL mtos_messageq_clr(u32 msgq_id)
{
    u32 count = 0;
    os_msg_t msg;

    //check param
    if (msgq_id >= MAX_SYSMSGQNUM) {
	mtos_printk("[mtos_messageq_clr] param is ERROR!\n");
	return FALSE;
    }

    (void)SYS_MsgQQuery(msgq_ctrl[msgq_id].msgq_id, (U32 *)&count);
    while (count > 0) {
	memset(&msg, 0, sizeof(os_msg_t));
	(void)SYS_MsgQWait((void *)&msg, msgq_ctrl[msgq_id].msgq_id, SYS_TIMEOUT_IMMEDIATE);

	if (msg.is_ext) {
	    (void)_msgq_ext_exit(msgq_ctrl[msgq_id].ext_buf, &msg);
	}

  if(msg.is_sync) /**ack send sync msg***/
   {
    (void)mtos_sem_give(&(msgq_ctrl[msgq_id].sync_sem));
   }

	(void)SYS_MsgQQuery(msgq_ctrl[msgq_id].msgq_id, (U32 *)&count);
    }

    return TRUE;
}

void mtos_messageq_release(u32 msgq_id)
{
    //check param
    if (msgq_id >= MAX_SYSMSGQNUM) {
	mtos_printk("[mtos_messageq_release] param is ERROR!\n");
	return;
    }

    (void)mtos_messageq_clr(msgq_id);

    (void)SYS_MsgQDel(msgq_ctrl[msgq_id].msgq_id);
    mtos_sem_destroy(&(msgq_ctrl[msgq_id].sync_sem), 0);

    memset(&msgq_ctrl[msgq_id], 0, sizeof(msgq_ctrl_t));
    msgq_ctrl[msgq_id].used = 0;

    return;
}

MT_BOOL mtos_messageq_attach(u32 msgq_id, void *p_buf, u32 atom_size, u32 depth)
{
    if ((msgq_id >= MTOS_MAX_MSGQ) || (p_buf == NULL) || (atom_size == 0) || (depth == 0)) {
	mtos_printk("[mtos_messageq_attach] param is ERROR!\n");
	return FALSE;
    }

    _msgq_ext_use(msgq_ctrl[msgq_id].ext_buf, p_buf, (u16)atom_size, (u16)depth);

    return TRUE;
}

void mtos_messageq_detach(u32 msgq_id, u32 atom_size)
{
    if ((msgq_id >= MTOS_MAX_MSGQ) || (atom_size == 0)) {
	mtos_printk("[mtos_messageq_detach] param is ERROR!\n");
	return;
    }

    _msgq_ext_unuse(msgq_ctrl[msgq_id].ext_buf, (u16)atom_size);

    return;
}

void mtos_message_increase_ref(u32 msgq_id, void *p_addr, u32 atom_size)
{
    u8 id = 0;
    s8 *p_ref = NULL;
    msgq_ext_buf_ctrl_t *p_ext = NULL;

    if ((msgq_id >= MTOS_MAX_MSGQ) || (p_addr == NULL) || (atom_size == 0)) {
	mtos_printk("[mtos_message_increase_ref] param is ERROR!\n");
	return;
    }

    id = _msgq_ext_find_id(msgq_ctrl[msgq_id].ext_buf, (u16)atom_size);
    if (id >= MTOS_MSGQ_MAX_BUF) {
	mtos_printk("[mtos_message_increase_ref] find ext buffer id ERROR!\n");
	return;
    }

    p_ext = &(msgq_ctrl[msgq_id].ext_buf[id]);
    p_ref = _msgq_ext_find_ref_id(p_ext, p_addr);
    if (p_ref == NULL) {
	mtos_printk("[mtos_message_increase_ref] ref is ERROR!\n");
	return;
    }
    //mtos_task_lock();
    (*p_ref)++;
    //mtos_task_unlock();

    return;
}

void mtos_message_decrease_ref(u32 msgq_id, void *p_addr, u32 atom_size)
{
    u8 id = 0;
    s8 *p_ref = NULL;
    msgq_ext_buf_ctrl_t *p_ext = NULL;

    if ((msgq_id >= MTOS_MAX_MSGQ) || (p_addr == NULL) || (atom_size == 0)) {
	mtos_printk("[mtos_message_decrease_ref] param is ERROR!\n");
	return;
    }

    id = _msgq_ext_find_id(msgq_ctrl[msgq_id].ext_buf, (u16)atom_size);
    if (id >= MTOS_MSGQ_MAX_BUF) {
	mtos_printk("[mtos_message_decrease_ref] find ext buffer id ERROR!\n");
	return;
    }

    p_ext = &(msgq_ctrl[msgq_id].ext_buf[id]);
    p_ref = _msgq_ext_find_ref_id(p_ext, p_addr);
    if ((p_ref == NULL) || (*p_ref) == 0) {
	mtos_printk("[mtos_message_decrease_ref] ref is ERROR!\n");
	return;
    }

    //mtos_task_lock();
    (*p_ref)--;
    //mtos_task_unlock();

    return;
}

void mtos_messageq_query(u32 msgq_id, u32 *p_left)
{
    //check param
    if ((msgq_id >= MAX_SYSMSGQNUM) || (p_left == NULL)) {
	return;
    }

    (void)SYS_MsgQQuery(msgq_ctrl[msgq_id].msgq_id, (U32 *)p_left);

    return;
}

void mtos_message_dump(u32 msgq_id)
{
#if 0
  u8 i = 0;

  if(msgq_id >= MTOS_MAX_MSGQ)
  {
    mtos_printk("[mtos_message_dump] param is ERROR!\n");
    return;
  }

  mtos_printk("----- MTOS MSGQ EXT BUF(msgq_id: %d): -----\n", msgq_id);
  for(i=0; i<MTOS_MSGQ_MAX_BUF; i++)
  {
    mtos_printk("No[%d]\n", i);
    mtos_printk("used: %d\n", msgq_ctrl[msgq_id].ext_buf[i].used);
    mtos_printk("atom_size: %d\n", msgq_ctrl[msgq_id].ext_buf[i].atom_size);
    mtos_printk("depth: %d\n", msgq_ctrl[msgq_id].ext_buf[i].depth);
    mtos_printk("in: %d\n", msgq_ctrl[msgq_id].ext_buf[i].in);
    mtos_printk("out: %d\n", msgq_ctrl[msgq_id].ext_buf[i].out);
    mtos_printk("addr: 0x%x\n", msgq_ctrl[msgq_id].ext_buf[i].addr);
  }
#endif
    return;
}
