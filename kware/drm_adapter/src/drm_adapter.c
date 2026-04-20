/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "playready_adapter.h"
#include "drm_adapter.h"
#include "curl/curl.h"
#include "b64/cdecode.h"

#include<libxml/parser.h>
#include<libxml/tree.h>
#include  "mt_debug.h"

static void *p_challenge = NULL;
static uint32_t challenge_size = 0;

typedef struct __tagCurlResponse
{
    uint32_t cbRead;
    uint32_t cbResp;
    uint8_t *pbResp;
} CurlResponse;

#define CURL_MAXIMUM_HTTP_RESPONSE (512*1024)
#define CURL_HTTP_BUFFER_SIZE      ( 64*1024)

#define CURL_EOS_CHAR             '\0'
#define CURL_HTTP_HEADER_END_CHAR '\r'
#define CURL_NEW_LINE_CHAR        '\n'

#define TEST_MIN(x, y)  ((x) >(y) ? (y) : (x))
#define ChkCURL(x)                                                                                                                     \
    do {                                          \
            CURLcode _curlErr = (x);               \
            if( _curlErr != CURLE_OK )             \
            {                                      \
                printf( "\nCURL FAILURE: %s (%d)\nfile: %s\nline: %d\n", \
                        curl_easy_strerror(_curlErr), _curlErr, __FILE__, __LINE__ ); \
                goto ErrorExit;                    \
            }                                      \
    } while(0)


static uint32_t _CurlProcessHttpResponse( void *ptr, uint32_t size, uint32_t nmemb, void *stream )
{
    if( ptr != NULL && stream != NULL )
    {
        CurlResponse *pResp = ( CurlResponse * )stream;
        uint32_t cbBytesToCopy = TEST_MIN( size * nmemb, pResp->cbResp - pResp->cbRead );

        if( cbBytesToCopy > 0 )
        {
            memcpy( &pResp->pbResp[pResp->cbRead], ptr, cbBytesToCopy );
            pResp->cbRead += cbBytesToCopy;

            return cbBytesToCopy;
        }
    }

    return 0;
}


static int  _SetInternetOptions(CURL *f_phInetHandle )
{
    int dr          = 0;
    uint32_t   lTimeOutSec = 30;
    uint32_t   lNoDelay    = 1;

    ChkCURL( curl_easy_setopt( f_phInetHandle, CURLOPT_TCP_NODELAY   , lNoDelay              ) );
    ChkCURL( curl_easy_setopt( f_phInetHandle, CURLOPT_TIMEOUT       , lTimeOutSec           ) );
    ChkCURL( curl_easy_setopt( f_phInetHandle, CURLOPT_CONNECTTIMEOUT, lTimeOutSec           ) );
    ChkCURL( curl_easy_setopt( f_phInetHandle, CURLOPT_FOLLOWLOCATION, 1                     ) );
    ChkCURL( curl_easy_setopt( f_phInetHandle, CURLOPT_BUFFERSIZE    , 64*1024 ) );
    ChkCURL( curl_easy_setopt( f_phInetHandle, CURLOPT_USERAGENT     , "MSPR_PK_UTILITY"     ) );

ErrorExit:
    return dr;
}

static int _SendRequestRedirectable(
    CURL              *f_phInet,
    const char          *f_pszUrl,
    const uint8_t          *f_pbChallenge,
    uint32_t          f_cbChallenge,
    const char          *f_pszHeaders,
    MT_BOOL           f_fPost,
    uint8_t         **f_ppbResponse,
    uint32_t         *f_pcbResponse)
{
    int         dr               = 0;
    //  uint32_t          retries          = 0;
    struct curl_slist *poHeaders        = NULL;
    const char    *pCur             = f_pszHeaders;
    const char    *pStart           = pCur;
    char           rgchHeader[4096] = {0};
//    CURLcode           result           = CURLE_OK;
    CurlResponse       oResp            = {0};

    /*
    ChkArg( f_phInet      != NULL );
    ChkArg( f_pszUrl      != NULL );
    ChkArg( f_ppbResponse != NULL );
    ChkArg( f_pcbResponse != NULL );
    */
    if(f_pszUrl      == NULL)
    {
        goto ErrorExit;
    }
    poHeaders = curl_slist_append( poHeaders, "Accept:" );
    poHeaders = curl_slist_append( poHeaders, "Expect:" );
    poHeaders = curl_slist_append( poHeaders, "Proxy-Connection:" );
    poHeaders = curl_slist_append( poHeaders, "Proxy-Authorization:" );
    poHeaders = curl_slist_append( poHeaders, "Pragma: no-cache" );

    while( pCur != NULL )
    {
        if( (*pCur == CURL_EOS_CHAR || *pCur == CURL_HTTP_HEADER_END_CHAR) && pStart != pCur )
        {
            uint32_t len = TEST_MIN( sizeof(rgchHeader) - 1, (uint32_t)(pCur - pStart) );

            if( len > 0 )
            {
                memcpy( rgchHeader, pStart, len );
                rgchHeader[len] = CURL_EOS_CHAR;

                poHeaders = curl_slist_append( poHeaders, rgchHeader );
            }

            if( *pCur != CURL_EOS_CHAR )
            {
                pStart = pCur + 1;
            }
        }
        else if( *pCur == CURL_NEW_LINE_CHAR )
        {
            pStart++;
        }

        if( *pCur == CURL_EOS_CHAR )
        {
            break;
        }

        pCur++;
    }

    ChkCURL( curl_easy_setopt( f_phInet, CURLOPT_HTTPHEADER, poHeaders ) );

    if( f_fPost )
    {
        ChkCURL( curl_easy_setopt( f_phInet, CURLOPT_POST, 1 ) );
        ChkCURL( curl_easy_setopt( f_phInet, CURLOPT_POSTFIELDSIZE, f_cbChallenge ) );
        ChkCURL( curl_easy_setopt( f_phInet, CURLOPT_POSTFIELDS, (const char *)f_pbChallenge ) );
    }
    oResp.pbResp = malloc( CURL_MAXIMUM_HTTP_RESPONSE );
        //ChkMem( oResp.pbResp = malloc( CURL_MAXIMUM_HTTP_RESPONSE ) );
    oResp.cbResp = CURL_MAXIMUM_HTTP_RESPONSE;

    ChkCURL( curl_easy_setopt( f_phInet, CURLOPT_WRITEFUNCTION, _CurlProcessHttpResponse ) );
    ChkCURL( curl_easy_setopt( f_phInet, CURLOPT_WRITEDATA    , (void *)&oResp ) );
    ChkCURL( curl_easy_setopt( f_phInet, CURLOPT_VERBOSE , 1));
//    ChkCURL( curl_easy_setopt( f_phInet, CURLOPT_SSL_VERIFYHOST , 0L));
//    ChkCURL( curl_easy_setopt( f_phInet, CURLOPT_SSL_VERIFYPEER , 0L));
    ChkCURL( curl_easy_perform( f_phInet ) );

    *f_ppbResponse = oResp.pbResp;
    *f_pcbResponse = oResp.cbRead;
    oResp.pbResp = NULL;

ErrorExit:

    curl_slist_free_all( poHeaders );

    if(oResp.pbResp)
       free( oResp.pbResp );

    return dr;
}



static int DRM_TOOLS_OEM_DoHttpTransaction(
        char             *f_pszUrl,
        uint32_t             f_cchUrl,
        const char             *f_pszHttpHeader,
        uint32_t             f_cchHttpHeader,
        const uint8_t             *f_pbChallenge,
        uint32_t             f_cbChallenge,
        MT_BOOL              f_fPost,
        uint8_t            **f_ppbResponse,
        uint32_t            *f_pcbResponse)
{
    int  dr       = 0;
    CURL       *phCurl   = NULL;
//    uint64_t    lRequest = 0;

    /*ChkArg( f_pszUrl        != NULL );
    ChkArg( f_ppbResponse   != NULL );
    ChkArg( f_pcbResponse   != NULL );
    ChkArg( ( f_fPost && f_pbChallenge != NULL ) || !f_fPost );
    */
    if(f_cchUrl == 0 || f_cchHttpHeader == 0)
    {
        //fix warning
    }

    /* In windows, this will init the winsock stuff */
    curl_global_init( CURL_GLOBAL_ALL );

    /* get a curl handle */
    phCurl = curl_easy_init();
    //ChkBOOL( phCurl != NULL, DRM_E_FAIL );

    ChkCURL( curl_easy_setopt( phCurl, CURLOPT_URL, f_pszUrl ) );
    if( _SetInternetOptions( phCurl ) < 0)
     {
         printf("error SetInternetOptions\n");
         goto ErrorExit;
     }

    /*
    ** Send the challenge to the server and handle possible
    ** HTTP redirection(s).
    */
    if( _SendRequestRedirectable(
        phCurl,
        f_pszUrl,
        f_pbChallenge,
        f_cbChallenge,
        f_pszHttpHeader,
        f_fPost,
        f_ppbResponse,
        f_pcbResponse ) )
    {
        printf("error SendRequestRedirectable\n");
        goto ErrorExit;
    }

ErrorExit:

    if( phCurl != NULL )
    {
        curl_easy_cleanup( phCurl );
    }

    curl_global_cleanup();

    return dr;
}



static void test_OnCloseCallback (
                                        void    *f_pMediaKeySession,
                                    void                      *f_pvCallbackContext )
{
	printf("ADP_DRM_CDMI_OnCloseCallback  %lx %lx\n", (ulong)f_pMediaKeySession, (ulong)f_pvCallbackContext);	
}

static void test_OnKeyStatusChangeCallback (
                                       void    *f_pMediaKeySession,
                                    void                      *f_pvCallbackContext )
{
	printf("ADP_DRM_CDMI_OnKeyStatusChangeCallback  %lx %lx\n", (ulong)f_pMediaKeySession, (ulong)f_pvCallbackContext);	
}

static void test_OnKeyMessageCallback (
                                        void    *f_pMediaKeySession,
                                    void                      *f_pvCallbackContext,
                                 const char                      *f_pszMediaKeyMessageType,
                                          uint32_t                      f_cbMediaKeyMessage,
          uint8_t                      *f_pbMediaKeyMessage )
{
		printf("ADP_DRM_CDMI_OnKeyMessageCallback  %lx %lx\n", (ulong)f_pMediaKeySession, (ulong)f_pvCallbackContext);	
        printf("key message received (%u bytes) \n", f_cbMediaKeyMessage);
        printf("key message received (%s type) \n", f_pszMediaKeyMessageType);
        challenge_size = f_cbMediaKeyMessage;
        if(p_challenge == NULL)
        {
            p_challenge = malloc(challenge_size +1);
            
        }
        memcpy(p_challenge, f_pbMediaKeyMessage, challenge_size);
        //printf("%s  \n", f_pbMediaKeyMessage);
        
}

static int UnicodeToUtf8(char* pInput, char *pOutput)
{
    int len = 0;
    while (*pInput)
    {
        char low = *pInput;
        pInput++;
        char high = *pInput;
        //    int w=high<<8;
        int  wchar = (high<<8)+low;
        if (wchar <= 0x7F )
        {
            pOutput[len] = (char)wchar;
            len++;
        }
        else if (wchar >=0x80 && wchar <= 0x7FF)
        {
            pOutput[len] = 0xc0 |((wchar >> 6)&0x1f);
            len++;
            pOutput[len] = 0x80 | (wchar & 0x3f);
            len++;
        }
        else if (wchar >=0x800 && wchar < 0xFFFF)
        {
            pOutput[len] = 0xe0 | ((wchar >> 12)&0x0f);
            len++;
            pOutput[len] = 0x80 | ((wchar >> 6) & 0x3f);
            len++;
            pOutput[len] = 0x80 | (wchar & 0x3f);
            len++;
        }
        else
        {
            return -1;
        }
        pInput ++;
    }
    
    pOutput [len]= 0;
    return len;
}

static int parse_xml_file(char * la_url, char *kid, char *aes_mode,
    char *buf,int len){
        xmlDocPtr doc;
        xmlNodePtr root,node,info, kidnode;
        xmlChar *name,*value;
        doc=xmlParseMemory(buf,len);    //parse xml in memory
        if(doc==NULL){
            printf("doc == null\n");
            return -1;
        }
        root=xmlDocGetRootElement(doc);
        for(node=root->children;node;node=node->next){
            if(xmlStrcasecmp(node->name,BAD_CAST"DATA")==0)
                        break;
        }
        if(node==NULL){
            printf("no node = content\n");
            return -1;
        }
        for(node=node->children;node;node=node->next)
        {
            if(xmlStrcasecmp(node->name,BAD_CAST"LA_URL")==0)
            {         //get pro node
                 value=xmlNodeGetContent(node);
                 // printf("la_url is:%s\n",(char*)value);
                 if(la_url != NULL)
                 {
                     strcpy(la_url, (char*)value);
                 }
                 xmlFree(value);
            }
            else if(xmlStrcasecmp(node->name,BAD_CAST"PROTECTINFO")==0)
            {       //get details nod
                for(info=node->children;info;info=info->next)
                {  //traverse detail node
                    if(xmlStrcasecmp(info->name,BAD_CAST"KIDS")==0)
                    {
                        for(kidnode=info->children;kidnode;kidnode=kidnode->next)
                        {
                            if(xmlStrcasecmp(kidnode->name,BAD_CAST"KID")==0)
                            {
                                name=xmlGetProp(kidnode,BAD_CAST"ALGID");
                                value=xmlGetProp(kidnode,BAD_CAST"VALUE");
                                if(strlen((char*)value)!=0)
                                {
                                    // printf("%s : %s\n",(char*)name,(char*)value);
                                }
                                else
                                {
                                    // printf("%s has no value\n",(char*)name);
                                }
                                if(aes_mode != NULL)
                                {
                                    strcpy(aes_mode, name);
                                    strcpy(kid,value);
                                }
                                xmlFree(name);
                                xmlFree(value);
                            }
                        }
                    }
                }
            }
        }
        xmlFreeDoc(doc);
        return 0;
}

int drm_playreay_create(char * p_xml, void **f_pMediaKeySession,
                        ADAPTER_CDMI_MEDIA_KEYS *f_pMediaKeys,
                       ADAPTER_CDMI_MEDIA_KEY_SYSTEM_ACCESS       *f_pMediaKeySystemAccess)
{
	//ADAPTER_CDMI_MEDIA_KEY_SYSTEM_ACCESS MediaKeySystemAccess;
//	ADAPTER_CDMI_MEDIA_KEYS                    MediaKeys;
	void             *pMediaKeySession = NULL;
	DRMPL_RESULT res = 0;
	const char    *rgpszSupportedInitDataTypesDefault[]  = { "cenc", "keyids" };
	//const char kids[] =  "{\"kids\":[\"AAAAEAAQABAQABAAAAAAAQ==\"],\"com.microsoft.playready.algid\":\"aes128cbc\"}";
      char kids[256] = {0};
	uint32_t    cInitDataTypes     = 2;//DRM_NO_OF( rgpszSupportedInitDataTypesDefault );
	//char   **rgpszSupportedInitDataTypes    = rgpszSupportedInitDataTypesDefault;
    //char LA_url[] = "http://test.playready.microsoft.com/service/rightsmanager.asmx?cfg=(per:false)";
    char LA_url[256] = {0};
    char aes_mode[32] = {0};
    char kid[48] = {0};
     char header[] =  "Content-Type: text/xml; charset=utf-8\r\nSOAPAction: \"http://schemas.microsoft.com/DRM/2007/03/protocols/AcquireLicense\"\r\n";
    uint8_t *p_response = NULL;
    uint32_t response_size = 0;
	char store[] = "./store.dat";
    
//        printf("utf8: %s\n", p_xml);
	parse_xml_file(LA_url,kid,aes_mode,p_xml,strlen(p_xml));
    printf("la: %s\n", LA_url);
    printf("aes_mod %s\n", aes_mode);
    printf("kid %s\n", kid);
#if 1
    if(0==strcmp(aes_mode, "AESCBC"))
    {
        printf("CBC mode!!!!\n");
        sprintf(kids, "{\"kids\":[\"%s\"],\"com.microsoft.playready.algid\":\"aes128cbc\"}\n", kid);
        printf("%s\n",kids);
    }
    else
    {
        sprintf(kids, "{\"kids\":[\"%s\"],\"com.microsoft.playready.algid\":\"aes128ctr\"}\n", kid);
    }
#endif
	//DRM_CDMI_CreateMediaKeySystemAccess(
    res = ADAPTER_CDMI_CreateMediaKeySystemAccess(
     "com.microsoft.playready.recommendation",
     &cInitDataTypes,
     (char **)&rgpszSupportedInitDataTypesDefault,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     NULL,
     f_pMediaKeySystemAccess);
	 printf("ADAPTER_CDMI_CreateMediaKeySystemAccess res %d\n", res);
      
	 res = ADAPTER_CDMI_CreateMediaKeys(f_pMediaKeySystemAccess, f_pMediaKeys );
	 printf("ADAPTER_CDMI_CreateMediaKeys res %d, key %x\n", res, f_pMediaKeys);
	 
	 res = ADAPTER_CDMI_CreateMediaKeySession(f_pMediaKeys,
			"temporary", /*I do not known which string should be used*/
			NULL, /*no context*/
			store, /*must be a w+r enable path*/
			&pMediaKeySession );
	printf("ADAPTER_CDMI_CreateMediaKeySession res %d\n", res);
	
	ADAPTER_CDMI_SetSessionCallbacks(
    pMediaKeySession,
    NULL,
    test_OnCloseCallback,
    test_OnKeyStatusChangeCallback,
    test_OnKeyMessageCallback);
	
	res = ADAPTER_CDMI_GenerateRequest(
    pMediaKeySession,
    "keyids", /*keyids, cenc*/
    strlen(kids),
    (const uint8_t*)kids );
	printf("ADAPTER_CDMI_GenerateRequest res 0x%x\n", res);
    //printf("kids:%s\n", kids);

    //send challenge to server
    //http://test.playready.microsoft.com/service/rightsmanager.asmx?cfg=(persist:false,sl:150)
    DRM_TOOLS_OEM_DoHttpTransaction(
        LA_url,
        strlen(LA_url),
        header,
        strlen(header),
        p_challenge,
        challenge_size,
        1,
        &p_response,
        &response_size);

    printf("response size is %d\n", response_size);

    res = ADAPTER_CDMI_Update(
    pMediaKeySession,
    NULL,
    NULL,
    response_size,
    p_response);

    printf("ADAPTER_CDMI_Update res 0x%x\n", res);
	*f_pMediaKeySession = pMediaKeySession;
    //*f_pMediakeys = &MediaKeys;
    return 0;
}


int drm_session_create(void **f_pMediaKeySession,
                       ADAPTER_CDMI_MEDIA_KEYS *f_pMediaKeys,
                       ADAPTER_CDMI_MEDIA_KEY_SYSTEM_ACCESS       *f_pMediaKeySystemAccess,
                       char *base64str)
{
    base64_decodestate state;
	char* plaintext_out = NULL;
    char *p_utf8 = NULL;
	int i = 0;
    int inlen = strlen(base64str);
    printf("run in cdmi test main\n");
	
	
	//base64 decode str
	plaintext_out = malloc(inlen);
    p_utf8 = malloc(inlen);
	base64_init_decodestate(&state);
	base64_decode_block(base64str, (int)strlen(base64str), plaintext_out, &state);
    UnicodeToUtf8(plaintext_out, p_utf8);
    free(plaintext_out);

    if(NULL != strstr(p_utf8, "PlayReadyHeader"))
    {
        //playready mode 
        drm_playreay_create(p_utf8, f_pMediaKeySession, f_pMediaKeys, f_pMediaKeySystemAccess);
    }
    else
    {
        //other drm
        printf("not playread drm ...\n");
    }
	return 0;
}

int drm_session_decrypt(void *f_pMediaKeySession,
                        char *kid_b64,
                        uint64_t iv,
                        uint32_t  encrypted_Len,
                        const uint8_t *p_encrypted,
                        uint32_t  *out_len,
                        uint8_t   **p_out_buf)
{
    int ret = 0;
    //  char iv[16] = {0};
    char kid[16] = {0};
    uint32_t mapping[2] = {0,0};
    base64_decodestate state;
    ADAPTER_ID s_id = {0};
    
    base64_init_decodestate(&state);
	base64_decode_block(kid_b64, (int)strlen(kid_b64), kid, &state);

    //base64_init_decodestate(&state);
	//base64_decode_block(iv_b64, (int)strlen(iv_b64), iv, &state);
    mapping[1] = encrypted_Len;

    memcpy(s_id.rgb, kid, 16);
    ret = ADAPTER_CDMI_DecryptOpaque(
        f_pMediaKeySession,
        &s_id,
        2,
        mapping,
        iv,
        encrypted_Len,
        p_encrypted,
        out_len,
        p_out_buf);

    printf("p_buf in decrypt %x\n", *p_out_buf);
    return ret;
}

int drm_session_free_decrypted(void *f_pMediaKeySession,
                        char *kid_b64,
                        uint32_t  out_len, 
                        uint8_t   *p_out_buf)
{
    int ret = 0;
    ADAPTER_ID s_id = {0};
    char kid[16] = {0};
    base64_decodestate state;

    printf("p_buf in free %x\n", p_out_buf);
    base64_init_decodestate(&state);
	base64_decode_block(kid_b64, (int)strlen(kid_b64), kid, &state);
    memcpy(s_id.rgb, kid, 16);
    
    ADAPTER_CDMI_FreeOpaqueDecryptedContent(
    f_pMediaKeySession,
    &s_id,
    out_len,
    p_out_buf);

    return ret;
}

int drm_session_destroy(void *f_pMediaKeySession,
                        void *f_pMediakeys,
                        ADAPTER_CDMI_MEDIA_KEY_SYSTEM_ACCESS       *f_pMediaKeySystemAccess)
{
    int ret = 0;

    printf("drm_session_destroy, key %x\n",  f_pMediakeys);
    //ADAPTER_CDMI_DestroyMediaKeys(f_pMediakeys);
    ADAPTER_CDMI_DestroyMediaKeySession(f_pMediaKeySession );
    printf("line %d\n", __LINE__);
    // ADAPTER_CDMI_DestroyMediaKeys(f_pMediakeys);
    printf("line %d\n", __LINE__);
    // ADAPTER_CDMI_Close(f_pMediaKeySession );
    printf("line %d\n", __LINE__);
    //  ADAPTER_CDMI_Remove(f_pMediaKeySession );
    printf("line %d\n", __LINE__);
    //ADAPTER_CDMI_DestroyMediaKeys(f_pMediakeys);
    ADAPTER_CDMI_DestroyMediaKeySystemAccess(f_pMediaKeySystemAccess );
    return 0;
}
