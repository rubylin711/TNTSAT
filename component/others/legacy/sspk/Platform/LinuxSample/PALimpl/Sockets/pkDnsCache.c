/********************************************************************************************/
/* Montage Technology (Chengdu) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <pkPAL.h>
#include <pkSockets.h>

#include <platPrivate.h>
#include <palPrint.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <pkPAL.h>
#include <pkExecutive.h>

#include <platPrivate.h>
#include <palPrint.h>

#include <stdarg.h> // valist
#include <memory.h>
#include <malloc.h>

#include <pthread.h> //Threading
#include <signal.h>  //suspend/resume
#include <errno.h>   //error return values
#include <sched.h>   //priority set/get
#include <time.h>    //clock_gettime
#include <stdlib.h>  //srand; getenv
#include <wchar.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h> // setpriority
#include <sys/syscall.h> // syscall numbers
#include <unistd.h> // syscall


typedef enum
{
  DNS_STATUS_NONE ,
  DNS_STATUS_EXIST ,
  DNS_STATUS_PROCESS ,
} dns_status_e;


typedef struct addrInfo{
   dns_status_e dns_state;
   char *strHost;
   char *strPort;    
   //struct addrinfo** ppAi;
   struct  addrinfo *ai;
   int use_cnt;
   unsigned int time;
   int not_free;
}addrInfo_t;





typedef struct dnsCache{
 	int cnt;
	char *pst_addr;
	pthread_mutex_t mutex;
}dnsCache_t; 


static  dnsCache_t *gp_stDnsCache;
static int IsIP(const char* cp)
{
    return inet_addr(cp) == INADDR_NONE;
}


int initDnsCache(int cnt)
{
    int ret = pkS_OK; 
	if(gp_stDnsCache!=NULL)
		return pkS_FALSE; 
	gp_stDnsCache = (dnsCache_t *)malloc(sizeof( dnsCache_t));
	if(gp_stDnsCache == NULL)
		return pkS_FALSE; 
	memset(gp_stDnsCache,0,sizeof( dnsCache_t));
	gp_stDnsCache->cnt = cnt;
	gp_stDnsCache->pst_addr =  (char *)malloc(cnt *sizeof( addrInfo_t));
	memset(gp_stDnsCache->pst_addr,0,cnt *sizeof( addrInfo_t));


	//printf("initDnsCache\n");

	pthread_mutex_init(&gp_stDnsCache->mutex, NULL);
	return ret; 
}


static int check_addr_exit(dnsCache_t *dns_cache, char * hostname, char * portstr,int*idx)
{
	int i = 0;
	int host_same = 0, port_same =0;
	addrInfo_t *pst_addr ;
	char *pst_add_temp = NULL;
	for(i =0 ;i< dns_cache->cnt;i++){
		pst_add_temp = (char*)dns_cache->pst_addr + i*sizeof(addrInfo_t);
		pst_addr = (addrInfo_t *)pst_add_temp;
		if(pst_addr->dns_state ==DNS_STATUS_EXIST ||pst_addr->dns_state ==DNS_STATUS_PROCESS){
			if(hostname && pst_addr->strHost && (0==strcmp(hostname ,pst_addr->strHost)))
				host_same = 1;
			else if(hostname==NULL && pst_addr->strHost ==NULL)
				host_same = 1;
			
			if(portstr && pst_addr->strPort&& (0==strcmp(portstr ,pst_addr->strPort)))
				port_same = 1;
			else if(portstr==NULL && pst_addr->strPort ==NULL)
				port_same = 1;

			if(host_same ==1 && port_same ==1){
				*idx = i;
				while(pst_addr->dns_state ==DNS_STATUS_PROCESS){	
					usleep(10000);//10ms
				}
				if(pst_addr->dns_state ==DNS_STATUS_EXIST)
					return pkS_OK;
				else
					break;
			}	
		}
	}
	return pkS_FALSE; 
}

static int get_addr_exit(dnsCache_t *dns_cache, int idx,  struct addrinfo** ai)
{
	addrInfo_t *pst_addr ;
	struct timeval tvNow;
	char *pst_addr_temp = (char*)dns_cache->pst_addr + idx*sizeof(addrInfo_t);
	pst_addr = (addrInfo_t *)pst_addr_temp;
	*ai = pst_addr->ai;
	pst_addr->use_cnt++;
	//printf("get_addr_exit use_cnt:%d\n",pst_addr->use_cnt);
	gettimeofday(&tvNow, NULL);
	int cur_time = tvNow.tv_sec*1000 + (uint32_t)tvNow.tv_usec/1000;
	pst_addr->time = cur_time;
	return pkS_OK;	
}

static int get_addr_by_idx(dnsCache_t *dns_cache, int idx,  struct addrinfo** ai)
{
	addrInfo_t *pst_addr ,*pst_addr_tmp;
	struct timeval tvNow;
	int i = 0;
	char *pst_addr_temp = (char*)dns_cache->pst_addr + idx*sizeof(addrInfo_t);
	pst_addr = (addrInfo_t *)pst_addr_temp;
	while(pst_addr->dns_state == DNS_STATUS_PROCESS){
		usleep(10000);
	}
	pthread_mutex_lock(&dns_cache->mutex);
 
	if(pst_addr->dns_state == DNS_STATUS_EXIST){
		*ai = pst_addr->ai;
		pst_addr->not_free =0;
		pst_addr->use_cnt++;		
		gettimeofday(&tvNow, NULL);
		int cur_time = tvNow.tv_sec*1000 + (uint32_t)tvNow.tv_usec/1000;
		pst_addr->time =  cur_time;
		for(i =0 ;i< dns_cache->cnt;i++){
			pst_addr_temp = (char*)dns_cache->pst_addr + i*sizeof(addrInfo_t);
			pst_addr_tmp = (addrInfo_t *)pst_addr_temp;;
			if(pst_addr_tmp->dns_state ==DNS_STATUS_EXIST &&pst_addr_tmp!=pst_addr){
				if(pst_addr->time< pst_addr_tmp->time )
					pst_addr_tmp->time = 0;
			}	
		}
   	 	pthread_mutex_unlock(&dns_cache->mutex);		
		return pkS_OK;
	}
 	pst_addr->not_free =0;
  	pthread_mutex_unlock(&dns_cache->mutex);
	return pkS_FALSE;

}


static int get_free_addr(dnsCache_t *dns_cache, char * hostname, char * portstr,int*idx)
{
	int i = 0,real_idx = -1;
	addrInfo_t *pst_addr ;
	unsigned int time = 0;	
	char *pst_addr_temp = NULL;
re_check:
	real_idx = -1;
	for(i =0 ;i< dns_cache->cnt;i++){
		pst_addr_temp = (char*)dns_cache->pst_addr + i*sizeof(addrInfo_t);
		pst_addr = (addrInfo_t *)pst_addr_temp;
		if(pst_addr->dns_state ==DNS_STATUS_NONE){

			if(pst_addr->strHost)
				free(pst_addr->strHost);
			if(pst_addr->strPort)
				free(pst_addr->strPort);
			pst_addr->strHost = pst_addr->strPort = NULL;
			
			if(hostname)
				pst_addr->strHost  =strdup(hostname);
			if(portstr)
				pst_addr->strPort  =strdup(portstr);
			pst_addr->dns_state =DNS_STATUS_PROCESS;
			pst_addr->not_free =1;
			*idx =i;
			return pkS_OK;
		}
	}
	if(i == dns_cache->cnt){
		for(i =0 ;i< dns_cache->cnt;i++){
			pst_addr_temp = (char*)dns_cache->pst_addr + i*sizeof(addrInfo_t);
			pst_addr = (addrInfo_t *)pst_addr_temp;
			if(pst_addr->dns_state ==DNS_STATUS_EXIST && !pst_addr->not_free){
				if(pst_addr->use_cnt ==0 ){
					if(time==0){
						time =pst_addr->time;
						real_idx = i;
					}else if(time>pst_addr->time){
						time = pst_addr->time;
						real_idx = i;
					}
				}				
			}			
		}		
	}

	if(real_idx>=0){
		pst_addr_temp = (char*)dns_cache->pst_addr + real_idx*sizeof(addrInfo_t);
		pst_addr = (addrInfo_t *)pst_addr_temp;		
		if (pst_addr->ai != NULL){
			freeaddrinfo(pst_addr->ai);
			pst_addr->ai = NULL;
		}
		pst_addr->dns_state =DNS_STATUS_NONE;
		goto re_check;	

	}
	
	if(real_idx ==-1){
		 pthread_mutex_lock(&dns_cache->mutex);
		 usleep(50000);
		 pthread_mutex_unlock(&dns_cache->mutex);
		 goto re_check;
	}

	return pkS_FALSE;
}


int mss_getaddrinfo(char * hostname, char * portstr,  struct addrinfo* hints,  struct addrinfo** ai)
{
	dnsCache_t *dns_cache = gp_stDnsCache;
	int idx = 0;
    int ret = pkS_OK;
	//printf("mt_getaddrinfo hostname:%s, port:%s\n",hostname, portstr);
	if(gp_stDnsCache == NULL ||hostname == NULL ||!IsIP(hostname) ){
		return getaddrinfo(hostname, portstr, hints, ai); 
	}

	pthread_mutex_lock(&dns_cache->mutex);
	if(pkS_OK == check_addr_exit(dns_cache ,hostname, portstr,&idx)){		
		//printf("mt_getaddrinfo idx:%d\n",idx);
		get_addr_exit(dns_cache,idx,ai);
	   	pthread_mutex_unlock(&dns_cache->mutex);
		return pkS_OK;
	}

	if(pkS_OK !=get_free_addr(dns_cache ,hostname, portstr,&idx)){
   	 	pthread_mutex_unlock(&dns_cache->mutex);
		return pkS_FALSE;
	}	
	//printf("mt_getaddrinfo free idx:%d\n",idx);

	addrInfo_t *pst_addr =NULL;	
	char *pst_addr_temp = NULL;
	pst_addr_temp = (char*)dns_cache->pst_addr + idx*sizeof(addrInfo_t);
	pst_addr = (addrInfo_t *)pst_addr_temp;
    ret= getaddrinfo(pst_addr->strHost, pst_addr->strPort,hints, &pst_addr->ai);   
    if(ret==0 && pst_addr->ai)  {
		pst_addr->dns_state = DNS_STATUS_EXIST;
    }else {
		pst_addr->dns_state = DNS_STATUS_NONE;
    }
	//printf("mss_getaddrinfo ai:0x%x\n",pst_addr->ai);

   	pthread_mutex_unlock(&dns_cache->mutex);
	return get_addr_by_idx(dns_cache,idx,ai);
}



static int free_addr_cnt(dnsCache_t *dns_cache, char * hostname, char * portstr,struct  addrinfo *ai)
{
	int i = 0;
	addrInfo_t *pst_addr =NULL;
	char *pst_addr_temp = NULL;
	for(i =0 ;i< dns_cache->cnt;i++){
		pst_addr_temp = (char*)dns_cache->pst_addr + i*sizeof(addrInfo_t);
		pst_addr = (addrInfo_t *)pst_addr_temp;
		if(pst_addr->dns_state ==DNS_STATUS_EXIST ){
			if(pst_addr->ai==ai){
				if(pst_addr->use_cnt>0){
					pst_addr->use_cnt --;
					//printf("free_addr_cnt use_cnt:%d\n",pst_addr->use_cnt);
				}
				break;
			}
				
		}
	}
	return pkS_OK; 
	
}

int mss_freeaddrinfo(char *hostname, char *port, struct  addrinfo *ai)
{
	//printf("mt_freeaddrinfo hostname:%s, port:%s\n",hostname, port);
	if(gp_stDnsCache==NULL ||hostname == NULL ||!IsIP(hostname) ){
        freeaddrinfo(ai);
		return pkS_OK;
	}
   	pthread_mutex_lock(&gp_stDnsCache->mutex);
	free_addr_cnt(gp_stDnsCache, hostname, port, ai);
   	pthread_mutex_unlock(&gp_stDnsCache->mutex);
	return pkS_OK;
}

static int free_all_addr(dnsCache_t *dns_cache)
{
	int i = 0;
	addrInfo_t *pst_addr ;	
	char *pst_addr_temp = NULL;
	for(i =0 ;i< dns_cache->cnt;i++){
		pst_addr_temp = (char*)dns_cache->pst_addr + i*sizeof(addrInfo_t);
		pst_addr = (addrInfo_t *)pst_addr_temp; 
		if(pst_addr->strHost)
			free(pst_addr->strHost);
		if(pst_addr->strPort)
			free(pst_addr->strPort);
		pst_addr->strHost = pst_addr->strPort = NULL;
		if (pst_addr->ai != NULL){			
			//printf("free_all_addr freeaddrinfo ai:0x%x\n",pst_addr->ai);
			freeaddrinfo(pst_addr->ai);
			pst_addr->ai = NULL;
		}
	
	}
	free(dns_cache->pst_addr);
	return pkS_OK;
	
}
int uninitDnsCache()
{
    int ret = pkS_OK; 
	if(gp_stDnsCache==NULL)
		return pkS_OK;
	free_all_addr(gp_stDnsCache);
	pthread_mutex_destroy(&gp_stDnsCache->mutex);
	free(gp_stDnsCache);
	gp_stDnsCache= NULL;	
	return ret;
}

