/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef _DRM_ADAPTER_H__
#define _DRM_ADAPTER_H__
/*
  f_pMediaKeySession : out handle
  f_pMediakeys:  out handle
  base64str: in string
 */
int drm_session_create(void **f_pMediaKeySession,
                       ADAPTER_CDMI_MEDIA_KEYS *f_pMediaKeys,
                       ADAPTER_CDMI_MEDIA_KEY_SYSTEM_ACCESS       *f_pMediaKeySystemAccess,
                       char *base64str);


/*
  f_pMediaKeySession: in handle
  kid_b64: in
  iv: in
  encrypted_Len: in data size
  p_encrypted: in data ptr
  out_len: out
  p_out_buf: out
 */
int drm_session_decrypt(void *f_pMediaKeySession,
                        char *kid_b64,
                        uint64_t iv,
                        uint32_t  encrypted_Len,
                        const uint8_t *p_encrypted,
                        uint32_t  *out_len,
                        uint8_t   **p_out_buf);


/*
f_pMediaKeySession: in handle
  kid_b64: in
  out_len: in
  p_out_buf: in
 */
int drm_session_free_decrypted(void *f_pMediaKeySession,
                        char *kid_b64,
                        uint32_t  out_len, 
                               uint8_t   *p_out_buf);

int drm_session_destroy(void *f_pMediaKeySession,
                        void *f_pMediakeys,
                        ADAPTER_CDMI_MEDIA_KEY_SYSTEM_ACCESS       *f_pMediaKeySystemAccess);

int drm_playreay_create(char * p_xml, void **f_pMediaKeySession,
                        ADAPTER_CDMI_MEDIA_KEYS *f_pMediaKeys,
                       ADAPTER_CDMI_MEDIA_KEY_SYSTEM_ACCESS       *f_pMediaKeySystemAccess);

#endif
