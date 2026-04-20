/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies   */
/************************************************************************
  File Name     : scan.c
  Version        : Initial Draft
  Author         : Montage software group
  Created        : 2023/04/06
  Description   :  bluetooth  api head file
  				

  History         :
  1.Date          : 2023/04/06
    Author        :
    Modification: Created file

************************************************************************/
#ifndef __MT_BLUEZ_API_H
#define __MT_BLUEZ_API_H

#include <glib.h>
#include <gio/gio.h>
#include <pthread.h>
#ifdef __cplusplus
extern "C" {
#endif
enum __BLUEZ_MSG_ID {
	MSG_UNKNOWN = 0,
	SCAN_DEV,
	REMOVE_DEV,
	LIST_DEV,
	PAIRED_DEV,
	CONTROLLER_OUT,
	CONTROLLER_IN,
	DEV_CONNECTED,
	DEV_DISCONNECT,
    DEV_AUTO_TRUSTED,  /* set auto trust message */
    AGENT_CONFIRM_REQ, /* Agent RequestConfirmation (payload: agent_confirm_info) */
    AGENT_AUTH_REQ     /* Agent RequestAuthorization (payload: agent_auth_info) */
};

typedef struct __bt_dev_info {
	char alias[32];
	char mac[18];
}bt_dev_info;

typedef struct __agent_confirm_info {
	char mac[18];
	unsigned int passkey; /* 6-digit */
} agent_confirm_info;

typedef struct __agent_auth_info {
	char mac[18];
} agent_auth_info;

typedef struct __bluez_con_info {
	void *con;
	void *loop;
	unsigned char prop_changed;
	unsigned char iface_added;
	unsigned char iface_removed;
	char hci_path[32];
	unsigned long threadid;
}bluez_con_info;

typedef int (*bluez_callback)(int msgid,void *msg,int len);

/**
\brief	set callback to montage bluez adaptor module
\attention \n
set message callback function
CNcomment: 设置消息回调函数。CNend
N/A.CNcomment:无。CNend
*/
void bluez_set_callback(bluez_callback callback);

/**
\brief	Init the montage bluez adaptor module
\attention \n
Before using the module,call the api first.
The NULL will be returned if some error happened
CNcomment: 在进行相关操作前，请先调用
			这个api初始化。CNend
\param[in]  hci bluetooth controller name.CNcomment:蓝牙控制器设备名字。CNend
\retval ::如果失败会返回NULL。CNend
\see \n
N/A.CNcomment:无。CNend
*/
bluez_con_info* bluez_con_init(char *hci);

/**
\brief	deinit the montage bluez adaptor module
\attention \n
call this api for free resource when don't use the module
CNcomment: 退出模块是调用该API释放资源CNend
\param[in] pcon  moudle handle.CNcomment:模块句柄。CNend
\retval :: voidNend
\see \n
N/A.CNcomment:无。CNend
*/
void bluez_con_deinit(bluez_con_info *pcon);

/**
\brief	scan bt devices nearby
\attention \n
start to scan bt devices nearby
CNcomment:  扫描附近的蓝牙设备CNend
\param[in] pcon  moudle handle.CNcomment:模块句柄。CNend
\retval ::0  Success.CNcomment:成功。CNend
\retval ::-1  Success.CNcomment:失败。CNend

\see \n
N/A.CNcomment:无。CNend
*/
int bluez_start_discovery(bluez_con_info *pcon);

/**
\brief	stop to scan bt devices
\attention \n
stop to scan bt devices 
CNcomment: 停止扫描蓝牙设备CNend
\param[in] pcon  moudle handle.CNcomment:模块句柄。CNend
\retval :: 0  Success.CNcomment:成功。CNend
\retval ::-1  Success.CNcomment:失败。CNend

\see \n
N/A.CNcomment:无。CNend
*/
int bluez_stop_discovery(bluez_con_info *pcon);

/**
\brief	connect the bt device
\attention \n
connect the bt devices 
CNcomment: 连接蓝牙设备CNend
\param[in] pcon  moudle handle.CNcomment:模块句柄。CNend
\param[in] dev_mac  the mac of bt device:蓝牙设备地址。CNend

\retval :: 0  Success.CNcomment:成功。CNend
\retval ::-1  Success.CNcomment:失败。CNend

\see \n
N/A.CNcomment:无。CNend
*/
int bluez_connect_device(bluez_con_info *pcon,char *dev_mac);

/**
\brief	pair with the bt device
\attention \n
Initiate pairing with the bt device via org.bluez.Device1.Pair. 
CNcomment: 配对蓝牙设备CNend
\param[in] pcon  moudle handle.CNcomment:模块句柄。CNend
\param[in] dev_mac  the mac of bt device:蓝牙设备地址。CNend

\retval :: 0  Success.CNcomment:成功。CNend
\retval ::-1  Success.CNcomment:失败。CNend

\see \n
N/A.CNcomment:无。CNend
*/
int bluez_pair_device(bluez_con_info *pcon, char *dev_mac);

/**
\brief	disconnect the bt device
\attention \n
disconnect the bt devices 
CNcomment: 断开蓝牙设备连接CNend
\param[in] pcon  moudle handle.CNcomment:模块句柄。CNend
\param[in] dev_mac  the mac of bt device:蓝牙设备地址。CNend

\retval :: 0  Success.CNcomment:成功。CNend
\retval ::-1  Success.CNcomment:失败。CNend

\see \n
N/A.CNcomment:无。CNend
*/
int bluez_disconnect_device(bluez_con_info *pcon,char *dev_mac);

/**
\brief	remove the bt device
\attention \n
remove the bt devices .\n
the host will store the paired device's info. if the device paired with another adaptor.\n
in this case,connect the device will be failed. We should remove the device,then \n
scan,connect the device
CNcomment: 移除蓝牙设备\n
蓝牙host端会保存已配对的设备信息，假如设备又和\n
另外一个host进行过配对.这种情况下连接设备会失败。
建议先移除设备，然后进行扫描，连接这个设备\n
CNend
\param[in] pcon  moudle handle.CNcomment:模块句柄。CNend
\param[in] dev_mac  the mac of bt device:蓝牙设备地址。CNend

\retval :: 0  Success.CNcomment:成功。CNend
\retval ::-1  Success.CNcomment:失败。CNend

\see \n
N/A.CNcomment:无。CNend
*/
int bluez_remove_device(bluez_con_info *pcon,char *dev_mac);

/**
\brief	get available  bt devices。
\attention \n
get available bt devices \n
call this api will return the available bt devices across the message callback function 
CNcomment:  获取有效的蓝牙设备，通过回调函数返回\n
设备信息
CNend
\param[in] pcon  moudle handle.CNcomment:模块句柄。CNend

\retval :: 0  Success.CNcomment:成功。CNend
\retval ::-1  Success.CNcomment:失败。CNend

\see \n
N/A.CNcomment:无。CNend
*/

int bluez_get_available_devices(bluez_con_info *pcon);

/**
\brief	get the bt device pair status
\attention \n
get the bt device pair status
CNcomment: 获取蓝牙设备配对状态CNend
\param[in] pcon  moudle handle.CNcomment:模块句柄。CNend
\param[in] dev_mac  the mac of bt device:蓝牙设备地址。CNend

\retval :: 0  no paire.CNcomment:没有配对。CNend
\retval ::1  paired.CNcomment:已经配对。CNend

\see \n
N/A.CNcomment:无。CNend
*/
int bluez_device_is_paired(bluez_con_info *pcon,char *dev_mac);

/**
\brief	get the bt device connect status
\attention \n
get the bt device connect status
CNcomment: 获取蓝牙设备连接状态CNend
\param[in] pcon  moudle handle.CNcomment:模块句柄。CNend
\param[in] dev_mac  the mac of bt device:蓝牙设备地址。CNend

\retval :: 0  no connect.CNcomment:没有连接。CNend
\retval ::1  connected.CNcomment:已经连接。CNend

\see \n
N/A.CNcomment:无。CNend
*/
int bluez_device_is_connected(bluez_con_info *pcon,char *dev_mac);

/**
\brief	power on/off controller
\attention \n
power on/off controller
CNcomment: 打开/关闭控制器电源 CNend
\param[in] pcon  moudle handle.CNcomment:模块句柄。CNend
\param[in] power  switch value:开关值。CNend

\retval :: 0  control power on/off successful:控制电源操作成功。CNend

\see \n
N/A.CNcomment:无。CNend
*/
int bluez_controller_power(bluez_con_info *pcon,int power);

/**
\brief	get paired  bt devices。
\attention \n
get paired bt devices \n
call this api will return the paired bt devices across the message callback function 
CNcomment:  获取已配对的蓝牙设备，通过回调函数返回\n
设备信息
CNend
\param[in] pcon  moudle handle.CNcomment:模块句柄。CNend

\retval :: 0  Success.CNcomment:成功。CNend
\retval ::-1  Success.CNcomment:失败。CNend

\see \n
N/A.CNcomment:无。CNend
*/
int bluez_get_paired_devices(bluez_con_info *pcon);
/**
 * @description: Set bluetooth adapter pairable status
 * @param pcon - [in] bluetooth connection info pointer
 * @param pairable - [in] pairable status, 1 for pairable, 0 for not pairable
 * @return: 0 on success, -1 on failure
 */
int bluez_set_pairable(bluez_con_info *pcon, int pairable);

/**
 * @description: Set bluetooth adapter discoverable status
 * @param pcon - [in] bluetooth connection info pointer
 * @param discoverable - [in] discoverable status, 1 for discoverable, 0 for not discoverable
 * @return: 0 on success, -1 on failure
 */
int bluez_set_discoverable(bluez_con_info *pcon, int discoverable);

/**
 * @description: Trust/untrust a specific bluetooth device
 * @param pcon - [in] bluetooth connection info pointer
 * @param dev_mac - [in] device MAC address, format: xx:xx:xx:xx:xx:xx
 * @param trust - [in] trust status, 1 for trust, 0 for untrust
 * @return: 0 on success, -1 on failure
 */
int bluez_trust_device(bluez_con_info *pcon, char *dev_mac, int trust);

/**
 * @description: set auto trust remote device
 * @param enable - [in] 1:auto trust 0:no auto trust
 * @return: void
 */
void bluez_set_auto_trust(int enable);

/**
 * @description: get auto trust status
 * @return: 1:enable auto trust,0:disable auto trust
 */
int bluez_get_auto_trust_status(void);
/**
 * @description: Set bluetooth adapter system alias
 * @param pcon - [in] bluetooth connection info structure pointer
 * @param alias - [in] new system alias string
 * @return: 0 success, -1 failure
 */
int bluez_set_system_alias(bluez_con_info *pcon, const char *alias);

/**
 * @description: Get current system alias of bluetooth adapter
 * @param pcon - [in] bluetooth connection info structure pointer
 * @param alias - [out] buffer to store alias
 * @param alias_size - [in] buffer size
 * @return: 0 success, -1 failure
 */
int bluez_get_system_alias(bluez_con_info *pcon, char *alias, size_t alias_size);

/**
 * @description: 设置Agent自动接受模式
 * @param enable - [in] 1: 自动接受配对确认/授权；0: 由上层决定（需调用回复API）
 */
void bluez_agent_set_auto_accept(int enable);

/**
 * @description: 上层对确认请求进行应答（yes/no）
 * @param mac    - [in] 目标设备MAC地址（xx:xx:xx:xx:xx:xx）
 * @param accept - [in] 1: 接受；0: 拒绝
 * @return 0: 成功；-1: 失败（未找到待处理请求等）
 */
int bluez_agent_reply_confirmation(const char *mac, int accept);

/**
 * @description: 上层对授权请求进行应答（yes/no）
 * @param mac    - [in] 目标设备MAC地址（xx:xx:xx:xx:xx:xx）
 * @param accept - [in] 1: 接受；0: 拒绝
 * @return 0: 成功；-1: 失败
 */
int bluez_agent_reply_authorization(const char *mac, int accept);
#ifdef __cplusplus
}
#endif

#endif /* __MT_BLUEZ_API_H__ */