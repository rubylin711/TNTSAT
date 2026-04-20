/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __PTS_LIST_H__
#define __PTS_LIST_H__
#ifdef __cplusplus
extern "C" {
#endif

int list_rwlock_init(void);
void list_rwlock_lock(void);
void list_rwlock_unlock(void);
void list_rwlock_deinit(void);

typedef int Elemtype;  //注意这里，下面函数中的输入数据针对int
typedef struct myNode  //数据结构
{
	Elemtype data;
	struct myNode *next;
}Node;

Node * InitList(void);
int ListLength(Node *L);

void Show(Node *L);
int AddFromEnd(Node *L,Elemtype data);
void find_rang(Node *L,Elemtype h,Elemtype* prev,Elemtype* post);
int destroy(Node *L);

int destroy_list_vpts(Node* pts);
int get_accurate_vpts_I_frame(Node* pts,u32 input_t,u32* prev_num,
		u32* accurate_num);

int AddnFromEnd(Node *L,int n,Elemtype data);
int del(Node *L,int n,Elemtype *rec);
int motify(Node *L,int n,Elemtype *rec,Elemtype data);
Node * find(Node *L,Elemtype num);
Node * findn(Node *L,int n);

#ifdef __cplusplus
}
#endif

#endif //__PTS_LIST_H__


