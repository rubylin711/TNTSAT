/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*************
the pts list for record PTS of video I frame,
for demux_seek can finish seek quickly.
**************/

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "mtos_sem.h"
#include "mt_type.h"
#include "file_playback_sequence.h"
#include "pts_list.h"

//#include<malloc.h>//如果要测试全部函数，请修改main中的注释符，插入删除等是对链表La的操作
//
//#include <pthread.h>
//#include <stdlib.h>

static os_sem_t list_rw_lock = 0;
int list_rwlock_init(void)
{
	int ret = TRUE;
	if(list_rw_lock == 0)
		ret =mtos_sem_create(&(list_rw_lock), TRUE);
	//MT_ASSERT(ret == TRUE);

	return ret;
}
void list_rwlock_lock(void)
{
	mtos_sem_take((os_sem_t *)(&(list_rw_lock)), 0);
	return ;
}
void list_rwlock_unlock(void)
{
	mtos_sem_give((os_sem_t *)(&(list_rw_lock)));
	return ;
}
void list_rwlock_deinit(void)
{
    	mtos_sem_destroy(&(list_rw_lock), 0);
	list_rw_lock = 0;
	return ;
}

Node* InitList(void)//初始化链表，带头结点，返回头结点
{
	Node *H;
	//mtos_printk("\n========init list========\n");
	H=(Node *)malloc33(sizeof(Node));

	if(!H)
		return 0;
	memset(H,0,sizeof(Node));
	H->next =NULL;//
	return H;
}
#if 0
int AddFromHead(Node *L,Elemtype e)
//头插创建，L需要已经初始化，输入e结束，原有数据不会清空
{
	Node *s;
	int flag=1;
	Elemtype data;//暂时存输入数据
	while(flag)
	{
		printf("int your num:");
		//scanf("%d",&data);

		if(data==e)//以输入e结束
		{
			flag=0;
		}
		else
		{
			s=(Node *)malloc33(sizeof(Node));//开辟
			if(!s)
				return 0;
			s->data=data;
			s->next=L->next;//头插
			L->next=s;
		}
	}
	return 1;
}
int AddnFromHead(Node *L,int n)
//头插法创建n个元素的链表，L已经初始化，原有内容不会清空
{
	Node *s;
	int c=0;//计数
	for(c=1;c<=n;c++)
	{
		s=(Node *)malloc33(sizeof(Node));
		if(!s)
			return 0;
		printf("int your num:");
		scanf("%d",&s->data);
		s->next=L->next;
		L->next=s;
	}
	return 1;//无异常，返回1
}
static Node* get_elem_tail(Node *L)
{
	Node *p;
	p= L;
	while(p&&(p->next != NULL))
	{
		p=p->next;
	}
	return p;
}
#endif
static Node* get_elem_tail_1(Node *L,int* n,Elemtype data)
{
	Node *p;
	p= L;
	while(p&&(p->next != NULL))
	{
		*n +=1;
		if(p->data ==data || p->data > data)
		  break;

		p=p->next;
		//mtos_printk("n:%d p->data:%d\n",*n,p->data);
	}
	return p;
}

static int insert(Node *L,int n,Elemtype num)
//在第n个位置插入数值num
{
	Node *pre,*s;
	int c;
	if(!L)
		return 0;
	pre=L;
	for(c=0;c<n-1 && pre!=NULL;c++)
	{
		pre=pre->next;
	}
	//mtos_printk("n:%d pre->data:%d nextdata:%d \n",n,pre->data,pre->next->data);
	s=(Node *)malloc33(sizeof(Node));
	if(!s){
		return 0;
	}
	s->data=num;
	s->next=pre->next;
	pre->next=s;
	return 1;
}

int AddFromEnd(Node *L,Elemtype data)
//尾插法创建，输入e结束，L已经初始化，原有内容会丢失
{
	/**
		L : list head node
		r:  list tail node
	*/
	Node *s,*r;
	int flag=1;
	int n=0;
	if(!L)
		return 0;
	//mtos_printk("0 list len:%d  input:%d \n",ListLength(L),data);
	r=get_elem_tail_1(L,&n,data);if(r == NULL) return 0;

	//mtos_printk("-- r->data:%d n:%d\n",r->data,n);
	while(flag)
	{

		if((data==r->data)&&ListLength(L)>0)
		{

			//mtos_printk("--->the data:%dis exist-- \n",data);
			flag=0;
		}
		else if(data < r->data)
		{
			//mtos_printk("insert data:%d n:%d \n",data,n);
			if(insert(L,n-1,data))
			{
			//	mtos_printk("11 list len:%d \n",ListLength(L));
				break;
			}

			//mtos_printk("22 list len:%d \n",ListLength(L));
		}
		else
		{
			s=(Node *)malloc33(sizeof(Node));//动态开辟
		if(!s)
			return 0;
		s->data=data;
		r->next=s;
		r=s;
		r->next=NULL;
		//mtos_printk("--new data:0x%x   r->data:0x%x n:%d\n",data,r->data,n+1);
		}
	}
	//mtos_printk("--L:0x%x  r:0x%x \n",L,r);

	//mtos_printk("33 list len:%d \n",ListLength(L));
	return 1;//无异常，返回1
}

int AddnFromEnd(Node *L,int n,Elemtype data)
//尾插法创建n个，原有内容会丢失
{
	Node *s,*r;
	int c=0;//计数

	r=L;//尾指针指向头结点，原有内容会丢失
	for(c=1;c<=n;c++)
	{
		s=(Node *)malloc33(sizeof(Node));
		if(!s)
		{
			return 0;
		}
		//printf("input your num:");
		s->data = data;
		r->next=s;
		r=s;
	}
	r->next=NULL;
	return 1;
}
int del(Node *L,int n,Elemtype *rec)
//删除链表L中的第n个元素，rec保存被删除的数值
{
	Node *pre,*r;//pre为前驱
	int c=0;

	if(!L)
		return 0;
	pre=L;
	for(c=0;c<n-1 && pre->next != NULL;c++)
	{
		pre=pre->next;
	}
	if(pre->next == NULL)
	{
		return 0;
	}
	r=pre->next;//r为待删除的
	pre->next=r->next;
	//*rec=r->data;//保存删除的值
	free33(r);
	return 1;
}
int motify(Node *L,int n,Elemtype *rec,Elemtype data)
//写给第n个节电数值，rec取得原来的数值
{
	Node *p;//
	int c=0;
	if(!L)
		return 0;
	p=L;
	for(c=0;c<n && p!=NULL;c++)
	{
		p=p->next;
	}
	if(p == NULL){
		return 0;
	}
	//*rec=p->data;//取值
	printf("int a new number:");
	//scanf("%d",&p->data);
	p->data = data;
	return 1;
}

Node * find(Node *L,Elemtype num)
//查找数值num，返回地址
{
	Node *p;
	p=L->next;
	while(p)
	{
		if(p->data==num)
			break;
		else
			p=p->next;
	}
	return p;
}
void find_rang(Node *L,Elemtype h,Elemtype* prev,Elemtype* post)
{
    Node *p = NULL;
    Node *p_prev = NULL;
    if(L && L->next) {
        p_prev = L;
        p = L->next;
    }

    while(p) {
        if(p->data < h) {
            p_prev = p;
            p = p->next;
        }

        if(p && p->data)
            *post = p->data;
        if(p_prev && p_prev->data)
            *prev = p_prev->data;

        if(!p || (p->data >= h))
            break;
    }
    return;
}
Node * findn(Node *L,int n)
//查找第n个位置的元素，返回地址
{
	Node *p;
	int c;
	p=L;
	for(c=0;c<n && p!=NULL;c++)
	{
		p=p->next;
	}
	return p;
}
int ListLength(Node *L)//取得链表有效长度
{
	Node *p;
	int c=0;
	if(!L)
		return 0;
	p=L->next;
	while(p)
	{
		c++;
		p=p->next;
	}
	return c;
}
void Show(Node *L)//输出链表
{
	Node *p;
	int n = 0;
	if(!L)
		return ;
	//mtos_printk("\n----show list----:\n");
	for(p=L->next,n = 0;p!=NULL;p=p->next)
	{

		//mtos_printk("%d: %d\n",n,p->data);
		n++;
	}
	//mtos_printk("----list end----:\n");
}

int destroy(Node *L)//销毁链表
{
	Node *s;
	if(!L)
		return 0;
	s=L->next;
	int n = 0;
	while(s)
	{
		L->next=L->next->next;
		free33(s);
		s=L->next;
		n++;
	}
//	mtos_printk("---free n:%d \n");
	free33(L);
	return 1;
}

/*
	for recode video pts (I frame)
*/
int destroy_list_vpts(Node* pts)
{
	int ret = 0;
	if(!pts)
		return ret;

	ret = destroy(pts);
	//pts =NULL;
	return ret;
}
/**
	input_t : input param , will serach dest in the list
	prev_num: prev node of dest
	accurate_num: dest node

	prev_num -> accurate_num
	         \               /
	          \             /
	           \           /
                    \        /
	             input_t
	 you will use accurate_num directly.

*/
int get_accurate_vpts_I_frame(Node* pts,u32 input_t,u32* prev_num,u32* post_num)
{
	if(!pts)
		return 0;
	find_rang(pts,(Elemtype)input_t,(Elemtype *)prev_num,(Elemtype *)post_num);
	//mtos_printk("prev_num:%d  post_num :%d \n",*prev_num,*post_num);

	return (int)*prev_num;
}


