/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies   */
/************************************************************************
  File Name     : mt_bluez_api.c
  Version        : Initial Draft
  Author         : Montage software group
  Created        : 2023/04/06
  Description   :  bluetooth  api for scan,connect,disconnect,remove...
  				

  History         :
  1.Date          : 2023/04/06
    Author        :
    Modification: Created file

************************************************************************/

#include <glib.h>
#include <gio/gio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <ctype.h>
#include "mt_bluez_api.h"

#define BT_ADDRESS_STRING_SIZE 18
#define AGENT_PATH "/org/bluez/my_agent"

static bluez_callback msg_cb = NULL;
static int auto_trust_enabled = 1;  /* 自动信任功能开关 */
static int agent_auto_accept = 1;   /* Agent自动接受开关：1自动，0由上层决定 */

/* Agent1 导出与注册相关的全局（进程内）状态 */
static GDBusNodeInfo *agent_node_info = NULL;
static guint agent_reg_id = 0;
static bluez_con_info *g_agent_pcon = NULL; /* 便于在 Agent 回调中访问 system bus */
/* 待处理的 Agent 请求：以MAC为key，invocation为值 */
static GHashTable *pending_confirm = NULL;     /* RequestConfirmation 队列 */
static GHashTable *pending_authorize = NULL;   /* RequestAuthorization 队列 */

/* Agent1 接口 XML 描述（简化版，涵盖常用方法） */
static const gchar agent_introspection_xml[] =
	"<node>"
	"  <interface name='org.bluez.Agent1'>"
	"    <method name='Release'/>"
	"    <method name='RequestPinCode'>"
	"      <arg type='o' name='device' direction='in'/>"
	"      <arg type='s' name='pincode' direction='out'/>"
	"    </method>"
	"    <method name='DisplayPinCode'>"
	"      <arg type='o' name='device' direction='in'/>"
	"      <arg type='s' name='pincode' direction='in'/>"
	"    </method>"
	"    <method name='RequestPasskey'>"
	"      <arg type='o' name='device' direction='in'/>"
	"      <arg type='u' name='passkey' direction='out'/>"
	"    </method>"
	"    <method name='DisplayPasskey'>"
	"      <arg type='o' name='device' direction='in'/>"
	"      <arg type='u' name='passkey' direction='in'/>"
	"      <arg type='q' name='entered' direction='in'/>"
	"    </method>"
	"    <method name='RequestConfirmation'>"
	"      <arg type='o' name='device' direction='in'/>"
	"      <arg type='u' name='passkey' direction='in'/>"
	"    </method>"
	"    <method name='RequestAuthorization'>"
	"      <arg type='o' name='device' direction='in'/>"
	"    </method>"
	"    <method name='AuthorizeService'>"
	"      <arg type='o' name='device' direction='in'/>"
	"      <arg type='s' name='uuid' direction='in'/>"
	"    </method>"
	"    <method name='Cancel'/>"
	"  </interface>"
	"</node>";

/* 用于延迟执行自动信任操作的结构体 */
typedef struct {
    bluez_con_info *pcon;
    char dev_mac[18];
} auto_trust_data_t;
static GVariant* bluez_device_get_property(GDBusConnection *con,
	const gchar *path, GVariant *var);

/* 从设备对象路径读取地址属性，返回静态缓冲的 MAC 字符串指针 */
static const char* agent_get_mac_from_path(const char *device_path)
{
	static char mac_buf[18];
	if (!g_agent_pcon || !device_path) return NULL;
	GVariant *result = NULL; GVariant *prop = NULL;
	GVariant *var = g_variant_new("(ss)", "org.bluez.Device1", "Address");
	result = bluez_device_get_property((GDBusConnection*)g_agent_pcon->con, device_path, var);
	if (!result) return NULL;
	g_variant_get(result, "(v)", &prop);
	const gchar *addr = g_variant_get_string(prop, NULL);
	if (addr) {
		g_strlcpy(mac_buf, addr, sizeof(mac_buf));
	} else {
		mac_buf[0] = '\0';
	}
	g_variant_unref(result);
	return mac_buf[0] ? mac_buf : NULL;
}

/* Agent1 方法处理：自动接受 Just Works / 授权请求，并打印（可根据需要扩展为回调上报） */
static void agent_method_call(GDBusConnection *connection,
							  const char *sender,
							  const char *object_path,
							  const char *interface_name,
							  const char *method_name,
							  GVariant *parameters,
							  GDBusMethodInvocation *invocation,
							  gpointer user_data)
{
	(void)connection; (void)sender; (void)object_path; (void)interface_name; (void)user_data;

	g_print("[Agent] Method Call: %s\n", method_name);
	if (g_strcmp0(method_name, "RequestAuthorization") == 0) {
		const char *device = NULL;
		g_variant_get(parameters, "(o)", &device);
		const char *mac = agent_get_mac_from_path(device);
		if (agent_auto_accept || !mac || !msg_cb) {
			g_print("[Agent] RequestAuthorization from %s (%s) -> auto-accept\n",
					device, mac ? mac : "unknown");
			g_dbus_method_invocation_return_value(invocation, NULL);
		} else {
			g_print("[Agent] RequestAuthorization from %s (%s) -> pending\n",
					device, mac);
			if (!pending_authorize) pending_authorize = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
			g_hash_table_insert(pending_authorize, g_strdup(mac), invocation);
			if (msg_cb && mac) {
				agent_auth_info info;
				memset(&info, 0, sizeof(info));
				g_strlcpy(info.mac, mac, sizeof(info.mac));
				msg_cb(AGENT_AUTH_REQ, &info, sizeof(info));
			}
			/* 不立即返回，由上层稍后调用回复API */
		}
		return;
	}

	if (g_strcmp0(method_name, "RequestConfirmation") == 0) {
		const char *device = NULL; guint32 passkey = 0;
		g_variant_get(parameters, "(ou)", &device, &passkey);
		const char *mac = agent_get_mac_from_path(device);
		if (msg_cb && mac) {
			agent_confirm_info info; memset(&info, 0, sizeof(info));
			g_strlcpy(info.mac, mac, sizeof(info.mac));
			info.passkey = passkey;
			msg_cb(AGENT_CONFIRM_REQ, &info, sizeof(info));
		}
		if (agent_auto_accept || !mac) {
			g_print("[Agent] RequestConfirmation passkey=%06u from %s (%s) -> auto-accept\n",
					passkey, device, mac ? mac : "unknown");
			g_dbus_method_invocation_return_value(invocation, NULL);
		} else {
			g_print("[Agent] RequestConfirmation passkey=%06u from %s (%s) -> pending\n",
					passkey, device, mac);
			if (!pending_confirm) pending_confirm = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
			g_hash_table_insert(pending_confirm, g_strdup(mac), invocation);
			/* 不立即返回，由上层稍后调用回复API */
		}
		return;
	}

	if (g_strcmp0(method_name, "AuthorizeService") == 0) {
		const char *device = NULL; const char *uuid = NULL;
		g_variant_get(parameters, "(os)", &device, &uuid);
		const char *mac = agent_get_mac_from_path(device);
		g_print("[Agent] AuthorizeService uuid=%s from %s (%s) -> auto-accept\n",
				uuid, device, mac ? mac : "unknown");
		g_dbus_method_invocation_return_value(invocation, NULL);
		return;
	}

	if (g_strcmp0(method_name, "Release") == 0 ||
		g_strcmp0(method_name, "Cancel") == 0 ||
		g_strcmp0(method_name, "DisplayPinCode") == 0 ||
		g_strcmp0(method_name, "DisplayPasskey") == 0) {
		/* 对于无需操作或仅提示类方法，直接返回 */
		g_dbus_method_invocation_return_value(invocation, NULL);
		return;
	}

	if (g_strcmp0(method_name, "RequestPinCode") == 0 ||
		g_strcmp0(method_name, "RequestPasskey") == 0) {
		/* 注册为 NoInputNoOutput，一般不会被要求输入 PIN/PASSKEY，返回不支持 */
		g_dbus_method_invocation_return_dbus_error(invocation,
			"org.bluez.Error.Rejected", "No input capability");
		return;
	}

	/* 未覆盖的方法，统一拒绝以避免异常状态 */
	g_dbus_method_invocation_return_dbus_error(invocation,
		"org.bluez.Error.Rejected", "Unsupported agent method");
}

static const GDBusInterfaceVTable agent_vtable = {
	agent_method_call,
	NULL,
	NULL
};

/* ========== Agent 上层控制相关 API 实现 ========== */
void bluez_agent_set_auto_accept(int enable)
{
	agent_auto_accept = enable ? 1 : 0;
	g_print("Agent auto-accept %s\n", agent_auto_accept ? "enabled" : "disabled");
}

typedef struct {
	GDBusMethodInvocation *inv;
	int accept;
} agent_reply_ctx;

static gboolean agent_finish_reply_cb(gpointer user_data)
{
	agent_reply_ctx *ctx = (agent_reply_ctx*)user_data;

	if (!ctx || !ctx->inv) return FALSE;
	if (ctx->accept) {
		g_dbus_method_invocation_return_value(ctx->inv, NULL);
	} else {
		g_dbus_method_invocation_return_dbus_error(ctx->inv,
			"org.bluez.Error.Rejected", "User rejected");
	}
	g_free(ctx);
	return FALSE;
}

static int agent_reply_common(GHashTable *table, const char *mac, int accept)
{
	if (!table || !mac)
		return -1;
	GDBusMethodInvocation *inv = (GDBusMethodInvocation*)g_hash_table_lookup(table, mac);
	if (!inv)
		return -1;
	/* 从表中移除，避免重复 */
	g_hash_table_remove(table, mac);
	agent_reply_ctx *ctx = g_new0(agent_reply_ctx, 1);
	ctx->inv = inv;
	ctx->accept = accept ? 1 : 0;
	/* 使用主循环空闲回调完成应答，避免跨线程直接调用 */
	g_idle_add(agent_finish_reply_cb, ctx);
	return 0;
}

int bluez_agent_reply_confirmation(const char *mac, int accept)
{
	return agent_reply_common(pending_confirm, mac, accept);
}

int bluez_agent_reply_authorization(const char *mac, int accept)
{
	return agent_reply_common(pending_authorize, mac, accept);
}

static int bluez_export_agent_object(bluez_con_info *pcon)
{
	if (!pcon) return -1;
	if (agent_reg_id != 0) return 0; /* 已注册 */
	GError *error = NULL;
	agent_node_info = g_dbus_node_info_new_for_xml(agent_introspection_xml, &error);
	if (error) {
		g_print("Agent introspection XML error: %s\n", error->message);
		g_error_free(error);
		return -1;
	}
	const GDBusInterfaceInfo *iface_info = g_dbus_node_info_lookup_interface(agent_node_info, "org.bluez.Agent1");
	if (!iface_info) {
		g_print("Agent interface info not found\n");
		return -1;
	}
	/* 在系统总线上导出对象，供 bluetoothd 调用 */
	agent_reg_id = g_dbus_connection_register_object((GDBusConnection*)pcon->con,
													 AGENT_PATH,
													 iface_info,
													 &agent_vtable,
													 NULL, NULL, &error);
	if (error) {
		g_print("Register Agent1 object failed: %s\n", error->message);
		g_error_free(error);
		agent_reg_id = 0;
		return -1;
	}
	g_print("bluez_export_agent_object succeed\n");
	g_agent_pcon = pcon;
	return 0;
}
/**
 * @description: 延迟执行自动信任操作的回调函数
 * @param user_data - [in] auto_trust_data_t 结构体指针
 * @return: FALSE （不重复执行）
 */
static gboolean auto_trust_delayed_callback(gpointer user_data)
{
    auto_trust_data_t *data = (auto_trust_data_t*)user_data;

    if (data && data->pcon) {
        int trust_result = bluez_trust_device(data->pcon, data->dev_mac, 1);
        if (trust_result == 0) {
            g_print("Auto-trusted device: %s\n", data->dev_mac);
            if (msg_cb) {
                msg_cb(DEV_AUTO_TRUSTED, data->dev_mac, strlen(data->dev_mac));
            }
        } else {
            g_print("Failed to auto-trust device: %s\n", data->dev_mac);
        }
    }

    /* 释放内存 */
    if (data) {
        g_free(data);
    }

    return FALSE; /* 不重复执行 */
}


#if 0
static void bluez_property_value(const gchar *key, GVariant *value)
{
    const gchar *type = g_variant_get_type_string(value);

    g_print("\t%s : ", key);
    switch(*type) {
        case 'o':
        case 's':
            g_print("%s\n", g_variant_get_string(value, NULL));
            break;
        case 'b':
            g_print("%d\n", g_variant_get_boolean(value));
            break;
        case 'u':
            g_print("%d\n", g_variant_get_uint32(value));
            break;
        case 'a':
        /* TODO Handling only 'as', but not array of dicts */
            if(g_strcmp0(type, "as"))
                break;
            g_print("\n");
            const gchar *uuid;
            GVariantIter i;
            g_variant_iter_init(&i, value);
            while(g_variant_iter_next(&i, "s", &uuid))
                g_print("\t\t%s\n", uuid);
            break;
        default:
            g_print("Other\n");
            break;
    }
}
#endif

static int bluez_register_agent(bluez_con_info *pcon,const char* agent_arg)
{
	if (pcon == NULL) {
		return -1;
	}
	GError *error = NULL;
	g_dbus_connection_call_sync(pcon->con,
								"org.bluez",			   // ??¡À¨º¡¤t??
								"/org/bluez",			   // ???¨®?¡¤??
								"org.bluez.AgentManager1", // ?¨®?¨²
								"RegisterAgent",		   // ¡¤?¡¤¡§??
								g_variant_new("(os)",AGENT_PATH,agent_arg), // 2?¨ºy¡êoagent???¨®?¡¤??o¨ª¡ä¨²¨¤¨ª¨¤¨¤D¨ª
								NULL,					   // ¡¤¦Ì???¦Ì??¨º?
								G_DBUS_CALL_FLAGS_NONE,
								-1,
								NULL,
								&error);
	if (error) {
		g_print("RegisterAgent error: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	
	g_dbus_connection_call_sync(pcon->con,
								"org.bluez",
								"/org/bluez",
								"org.bluez.AgentManager1",
								"RequestDefaultAgent",
								g_variant_new("(o)",AGENT_PATH),
								NULL,
								G_DBUS_CALL_FLAGS_NONE,
								-1,
								NULL,
								&error);
	if (error) {
		g_print("**RequestDefaultAgent error: %s\n", error->message);
		g_error_free(error);
		return -1;
	}
	return 0;
}

static int bluez_unregister_agent(bluez_con_info *pcon)
{
	if (pcon == NULL) {
		return -1;
	}
	GError *error = NULL;
	g_dbus_connection_call_sync(pcon->con,
								"org.bluez",
								"/org/bluez",
								"org.bluez.AgentManager1",
								"UnregisterAgent",
								g_variant_new("(o)",AGENT_PATH),
								NULL,
								G_DBUS_CALL_FLAGS_NONE,
								-1,
								NULL,
								&error);
	if (error) {
		g_print("UnRegisterAgent error: %s\n", error->message);
		g_error_free(error);
		return -1;
	}	
	return 0;
}

static GVariant* bluez_adapter_get_property(GDBusConnection *con,
		const gchar *path, GVariant *var)
{
	GVariant *result;
	GError *error = NULL;

	result = g_dbus_connection_call_sync(con,
					     "org.bluez",
					     path,
					     "org.freedesktop.DBus.Properties",
					     "Get",
					     var,
					     NULL,
					     G_DBUS_CALL_FLAGS_NONE,
					     -1,
					     NULL,
					     &error);
	if(error != NULL)
		return NULL;

	return result;
}


static void bluez_device_appeared(GDBusConnection *con,
                const gchar *sender_name,
                const gchar *object_path,
                const gchar *interface,
                const gchar *signal_name,
                GVariant *parameters,
                gpointer user_data)
{
    GVariantIter *interfaces;
    const char *object;
    const gchar *interface_name;
    GVariant *properties;
	const gchar *dev_mac = NULL,*dev_alias = NULL;
	bt_dev_info devinfo;

	g_variant_get(parameters, "(&oa{sa{sv}})", &object, &interfaces);
    while(g_variant_iter_next(interfaces, "{&s@a{sv}}",
					&interface_name, &properties)) {
        if(g_strstr_len(g_ascii_strdown(interface_name, -1), -1, "device") ||
			g_strstr_len(g_ascii_strdown(interface_name, -1), -1, "adapter")) {
            const gchar *property_name;
            GVariantIter i;
            GVariant *prop_val;
            g_variant_iter_init(&i, properties);
            while(g_variant_iter_next(&i, "{&sv}", &property_name, &prop_val)) {
                //bluez_property_value(property_name, prop_val);
				if (strcmp(property_name,"Address") == 0) {
					dev_mac =  g_variant_get_string(prop_val, NULL);
				} 
				if(strcmp(property_name,"Alias") == 0) {
					dev_alias = g_variant_get_string(prop_val, NULL);
				}				
				g_variant_unref(prop_val);
			}
			if (msg_cb) {
				int cp_len = 0;
				memset(&devinfo,0,sizeof(bt_dev_info));
				cp_len = strlen(dev_alias);
				if (cp_len > sizeof(devinfo.alias))
					cp_len = sizeof(devinfo.alias)-1;
				memcpy(devinfo.alias,dev_alias,cp_len);
				memcpy(devinfo.mac,dev_mac,17);
				if (g_strstr_len(g_ascii_strdown(interface_name, -1),
					-1, "device")) {
					msg_cb(SCAN_DEV,(void*)&devinfo,sizeof(bt_dev_info));
				} else if (g_strstr_len(g_ascii_strdown(interface_name, -1),
					-1, "adapter")) {					
					msg_cb(CONTROLLER_IN,(void*)&devinfo,sizeof(bt_dev_info));
				}
					
			}
        }
        g_variant_unref(properties);
    }
    return;
}


static void bluez_device_disappeared(GDBusConnection *sig,
                const gchar *sender_name,
                const gchar *object_path,
                const gchar *interface,
                const gchar *signal_name,
                GVariant *parameters,
                gpointer user_data)
{
    (void)sig;
    (void)sender_name;
    (void)object_path;
    (void)interface;
    (void)signal_name;

    GVariantIter *interfaces;
    const char *object;
    const gchar *interface_name;

    g_variant_get(parameters, "(&oas)", &object, &interfaces);
    while(g_variant_iter_next(interfaces, "s", &interface_name)) {
		if(g_strstr_len(g_ascii_strdown(interface_name, -1), -1, "adapter")) {
			if (msg_cb) {
				msg_cb(CONTROLLER_OUT,object,strlen(object));
			}
		}
		//todo: handle device disappeared，notify msg_cb，REMOVE_DEV
		else if(g_strstr_len(g_ascii_strdown(interface_name, -1), -1, "device")) {
			/* 从object path中提取MAC地址 */
			gchar *mac_addr = NULL;
			if (g_str_has_prefix(object, "/org/bluez/hci0/dev_")) {
				/* 提取dev_后面的部分并替换下划线为冒号 */
				const gchar *dev_part = object + strlen("/org/bluez/hci0/dev_");
				mac_addr = g_strdup(dev_part);
				/* 将下划线替换为冒号 */
				for (gchar *p = mac_addr; *p; p++) {
					if (*p == '_') *p = ':';
				}
			}

			if (msg_cb) {
				if (mac_addr) {
					msg_cb(REMOVE_DEV, mac_addr, strlen(mac_addr));
				} else {
					/* 如果无法提取MAC地址，回退到使用object path */
					msg_cb(REMOVE_DEV, object, strlen(object));
				}
			}
			g_free(mac_addr);
		}
    }
    return;
}

static void bluez_signal_adapter_changed(GDBusConnection *conn,
                    const gchar *sender,
                    const gchar *path,
                    const gchar *interface,
                    const gchar *signal,
                    GVariant *params,
                    void *userdata)
{
    GVariantIter *properties = NULL;
    GVariantIter *unknown = NULL;
    const char *iface;
    const char *key;
    GVariant *value = NULL;
	bluez_con_info *pcon = (bluez_con_info*)userdata;

    g_variant_get(params, "(&sa{sv}as)", &iface, &properties, &unknown);

	if(g_strstr_len(g_ascii_strdown(path, -1), -1, "/org/bluez/hci0/dev_")) {
		if(!g_strcmp0(iface,"org.bluez.Device1")) {
			GVariant *var,*result,*prop;
			gchar *dev_addr;
			int isconnect = 0;
			var =  g_variant_new("(ss)", "org.bluez.Device1", "Address");
			result = bluez_device_get_property(pcon->con,path,var);
			if (!result) {
				g_print("device %s not exist\n",path);
				goto done;
			}
			g_variant_get(result, "(v)", &prop);
			dev_addr = g_variant_get_string(prop,NULL);
			g_variant_unref(result);

		   	 while(g_variant_iter_next(properties, "{&sv}", &key, &value)) {
				if(!g_strcmp0(key, "Connected")) {
		            if(!g_variant_is_of_type(value, G_VARIANT_TYPE_BOOLEAN)) {
		                goto done;
		            }
					isconnect = g_variant_get_boolean(value);
					if (msg_cb) {
						if (isconnect == 0) {
							msg_cb(DEV_DISCONNECT,dev_addr,strlen(dev_addr));
						} else {
							msg_cb(DEV_CONNECTED,dev_addr,strlen(dev_addr));
						}
					}
					/* 在设备连接成功时，尝试设置为 Trusted，以支持无绑定场景的免交互自动重连 */
					if (isconnect && auto_trust_enabled) {
						auto_trust_data_t *trust_data = g_malloc(sizeof(auto_trust_data_t));
						if (trust_data) {
							trust_data->pcon = pcon;
							memcpy(trust_data->dev_mac, dev_addr, 17);
							trust_data->dev_mac[17] = '\0';
							g_idle_add(auto_trust_delayed_callback, trust_data);
						} else {
							g_print("Failed to allocate memory for auto-trust data\n");
						}
					}
		        }else if (!g_strcmp0(key,"Paired")) {
					/* 仍然兼容在 Paired 改变时设置 Trusted 的旧逻辑，但主要动作已在 Connected=true 时完成 */
					if (auto_trust_enabled) {
						auto_trust_data_t *trust_data = g_malloc(sizeof(auto_trust_data_t));
						if (trust_data) {
							trust_data->pcon = pcon;
							memcpy(trust_data->dev_mac, dev_addr, 17);
							trust_data->dev_mac[17] = '\0';
							g_idle_add(auto_trust_delayed_callback, trust_data);
						} else {
							g_print("Failed to allocate memory for auto-trust data\n");
						}
					}
		        }		
		    }	
		}
	}

done:
	if(properties != NULL)
		g_variant_iter_free(properties);
	if(value != NULL)
		g_variant_unref(value);
}

static int bluez_adapter_call_method(bluez_con_info *pcon,
				const char *method)
{
    GVariant *result;
    GError *error = NULL;

    result = g_dbus_connection_call_sync(pcon->con,
                         "org.bluez",
                    /* TODO Find the adapter path runtime */
                         pcon->hci_path,
                         "org.bluez.Adapter1",
                         method,
                         NULL,
                         NULL,
                         G_DBUS_CALL_FLAGS_NONE,
                         -1,
                         NULL,
                         &error);
    if(error != NULL)
        return 1;

    g_variant_unref(result);
    return 0;
}

static int bluez_adapter_set_property(bluez_con_info *pcon,
					const char *prop, GVariant *value)
{
    GVariant *result;
    GError *error = NULL;

    result = g_dbus_connection_call_sync(pcon->con,
                     "org.bluez",
                     pcon->hci_path,
                     "org.freedesktop.DBus.Properties",
                     "Set",
                     g_variant_new("(ssv)", "org.bluez.Adapter1", prop, value),
                     NULL,
                     G_DBUS_CALL_FLAGS_NONE,
                     -1,
                     NULL,
                     &error);
    if(error != NULL)
        return 1;

    g_variant_unref(result);
    return 0;
}


static GVariant* bluez_device_get_property(GDBusConnection *con,
	const gchar *path, GVariant *var)
{
	GVariant *result;
	GError *error = NULL;
	
	result = g_dbus_connection_call_sync(con,
					     "org.bluez",
					     path,
					     "org.freedesktop.DBus.Properties",
					     "Get",
					     var,
					     NULL,
					     G_DBUS_CALL_FLAGS_NONE,
					     -1,
					     NULL,
					     &error);
	if(error != NULL)
		return NULL;

	return result;
}

static void* start_discovery(void* para)
{
	int rc = 0;
	bluez_con_info *pcon = (bluez_con_info*)para;
	
	if (pcon == NULL) {
		return NULL;
	}
	rc = bluez_adapter_call_method(pcon,"StartDiscovery");
	if(rc) {
		g_print("Not able to scan for new devices\n");
		rc = -1;
		goto exit;
	}
	//g_main_loop_run(pcon->loop);	
exit:	
	return NULL;
	
}

static int adaptor_is_discoverying(bluez_con_info *pcon)
{
	GVariant *prop;
	int prop_val;
	GVariant *discovery;
	GVariant *var = NULL;
	
	discovery =  g_variant_new("(ss)", "org.bluez.Adapter1", "Discovering");
	var = bluez_adapter_get_property(pcon->con,pcon->hci_path,discovery);
	if (!var) {
		g_print("get adapter discoverying status failed\n");
		return -1;
	}
	g_variant_get(var, "(v)", &prop);	
	prop_val = g_variant_get_boolean(prop);	
	g_variant_unref(var);	
	return prop_val;
}

static int device_is_paried(bluez_con_info *pcon,gchar *obj_path)
{
	GVariant *prop;
	int prop_val;
	GVariant *result;
	GVariant *var = NULL;
	
	var =  g_variant_new("(ss)", "org.bluez.Device1", "Paired");
	result = bluez_device_get_property(pcon->con,obj_path,var);
	if (!result) {
		g_print("get device pair status failed\n");
		return 0;
	}
	g_variant_get(result, "(v)", &prop);	
	prop_val = g_variant_get_boolean(prop);
	g_variant_unref(result);	
	return prop_val;	
}

static int device_is_connected(bluez_con_info *pcon,gchar *obj_path)
{
	GVariant *prop;
	int prop_val;
	GVariant *result;
	GVariant *var = NULL;
	
	var =  g_variant_new("(ss)", "org.bluez.Device1", "Connected");
	result = bluez_device_get_property(pcon->con,obj_path,var);
	if (!result) {
		g_print("get device connect status failed\n");
		return 0;
	}
	g_variant_get(result, "(v)", &prop);	
	prop_val = g_variant_get_boolean(prop);	
	g_variant_unref(result);	
	return prop_val;	
}


static void bluez_list_devices(GDBusConnection *con,
				GAsyncResult *res,
				gpointer data)
{
	GVariant *result = NULL;
	GMainLoop *loop = NULL;
	GVariantIter i;
	const gchar *object_path;
	GVariant *ifaces_and_properties;
	const gchar *dev_mac = NULL,*dev_alias = NULL;
	bt_dev_info devinfo;

	loop = (GMainLoop *)data;
	result = g_dbus_connection_call_finish(con, res, NULL);
	if(result == NULL)
		g_print("Unable to get result for GetManagedObjects\n");

	/* Parse the result */
	if(result) {
		result = g_variant_get_child_value(result, 0);
		g_variant_iter_init(&i, result);
		while(g_variant_iter_next(&i, "{&o@a{sa{sv}}}", &object_path,
				&ifaces_and_properties)) {
			const gchar *interface_name;
			GVariant *properties;
			GVariantIter ii;
			g_variant_iter_init(&ii, ifaces_and_properties);
			while(g_variant_iter_next(&ii, "{&s@a{sv}}", 
					&interface_name, &properties)) {
				if(g_strstr_len(g_ascii_strdown(interface_name, -1),
							-1, "device1")) {
					GVariantIter iii;
					GVariant *prop_v;
					gchar *prop_n;
					g_variant_iter_init(&iii, properties);	
					while(g_variant_iter_next(&iii,"{&sv}",&prop_n,&prop_v)) {
						if (strcmp(prop_n,"Address") == 0) {
							dev_mac =  g_variant_get_string(prop_v, NULL);
						} 
						if(strcmp(prop_n,"Alias") == 0) {
							dev_alias = g_variant_get_string(prop_v, NULL);
						}
						g_variant_unref(prop_v);
					}
					if (msg_cb) {
						int cp_len = 0;
						memset(&devinfo,0,sizeof(bt_dev_info));
						cp_len = strlen(dev_alias);
						if (cp_len > sizeof(devinfo.alias))
							cp_len = sizeof(devinfo.alias)-1;
						memcpy(devinfo.alias,dev_alias,cp_len);
						memcpy(devinfo.mac,dev_mac,17);
						msg_cb(LIST_DEV,(void*)&devinfo,sizeof(bt_dev_info));				
					}
				}
				g_variant_unref(properties);
			}
			g_variant_unref(ifaces_and_properties);
		}
		g_variant_unref(result);
	}
	g_main_loop_quit(loop);
}

static void bluez_paired_devices(GDBusConnection *con,
				GAsyncResult *res,
				gpointer data)
{
	GVariant *result = NULL;
	GMainLoop *loop = NULL;
	GVariantIter i;
	const gchar *object_path;
	GVariant *ifaces_and_properties;
	const gchar *dev_mac = NULL,*dev_alias = NULL;
	bt_dev_info devinfo;

	loop = (GMainLoop *)data;
	result = g_dbus_connection_call_finish(con, res, NULL);
	if(result == NULL)
		g_print("Unable to get result for GetManagedObjects\n");

	/* Parse the result */
	if(result) {
		result = g_variant_get_child_value(result, 0);
		g_variant_iter_init(&i, result);
		while(g_variant_iter_next(&i, "{&o@a{sa{sv}}}", &object_path, 
					&ifaces_and_properties)) {
			const gchar *interface_name;
			GVariant *properties;
			GVariantIter ii;
			g_variant_iter_init(&ii, ifaces_and_properties);
			while(g_variant_iter_next(&ii, "{&s@a{sv}}", 
							&interface_name, &properties)) {
				if(g_strstr_len(g_ascii_strdown(interface_name, -1), 
							-1, "device1")) {
					GVariantIter iii;
					GVariant *prop_v;
					gchar *prop_n;
					int dev_paired = 0;
					
					g_variant_iter_init(&iii, properties);					
					while(g_variant_iter_next(&iii,"{&sv}",&prop_n,&prop_v)) {
						if (strcmp(prop_n,"Address") == 0) {
							dev_mac =  g_variant_get_string(prop_v, NULL);
						} 
						if(strcmp(prop_n,"Alias") == 0) {
							dev_alias = g_variant_get_string(prop_v, NULL);
						}
						if (strcmp(prop_n,"Paired") == 0) {
							dev_paired = g_variant_get_boolean(prop_v);
						}
						g_variant_unref(prop_v);
					}					
					if (msg_cb && dev_paired) {
						int cp_len = 0;
						memset(&devinfo,0,sizeof(bt_dev_info));
						cp_len = strlen(dev_alias);
						if (cp_len > sizeof(devinfo.alias))
							cp_len = sizeof(devinfo.alias)-1;
						memcpy(devinfo.alias,dev_alias,cp_len);
						memcpy(devinfo.mac,dev_mac,17);
						msg_cb(PAIRED_DEV,(void*)&devinfo,sizeof(bt_dev_info));				
					}
				}
				g_variant_unref(properties);
			}
			g_variant_unref(ifaces_and_properties);
		}
		g_variant_unref(result);
	}
	g_main_loop_quit(loop);
}

static void* bluez_msg_thread(void *para)
{
	bluez_con_info *pcon = (bluez_con_info*)para;

	pcon->prop_changed = g_dbus_connection_signal_subscribe(pcon->con,
						"org.bluez",
						"org.freedesktop.DBus.Properties",
						"PropertiesChanged",
						NULL,
						NULL,
						G_DBUS_SIGNAL_FLAGS_NONE,
						bluez_signal_adapter_changed,
						pcon,
						NULL);
	
   	
    pcon->iface_added = g_dbus_connection_signal_subscribe(pcon->con,
                            "org.bluez",
                            "org.freedesktop.DBus.ObjectManager",
                            "InterfacesAdded",
                            NULL,
                            NULL,
                            G_DBUS_SIGNAL_FLAGS_NONE,
                            bluez_device_appeared,
                            pcon->loop,
                            NULL);
    pcon->iface_removed = g_dbus_connection_signal_subscribe(pcon->con,
                            "org.bluez",
                            "org.freedesktop.DBus.ObjectManager",
                            "InterfacesRemoved",
                            NULL,
                            NULL,
                            G_DBUS_SIGNAL_FLAGS_NONE,
                            bluez_device_disappeared,
                            pcon->loop,
                            NULL);
	bluez_adapter_set_property(pcon,"Powered", g_variant_new("b", TRUE));
	g_main_loop_run(pcon->loop);	
	g_dbus_connection_signal_unsubscribe(pcon->con, pcon->prop_changed);
	g_dbus_connection_signal_unsubscribe(pcon->con, pcon->iface_added);
	g_dbus_connection_signal_unsubscribe(pcon->con, pcon->iface_removed);	
	return NULL;
}

void bluez_set_callback(bluez_callback callback)
{
	msg_cb = callback;
}
/********************************
description: get bluez dbus connection
return:
	if success: return pointer of connect,else return NULL
********************************/
bluez_con_info* bluez_con_init(char *hci)
{
	bluez_con_info * pcon = NULL;	

	if (!hci) {
		return NULL;
	}
	pcon = (bluez_con_info*)malloc(sizeof(bluez_con_info));
	if (!pcon) {
		return NULL;
	}
	memset(pcon,0x0,sizeof(bluez_con_info));
	snprintf(pcon->hci_path,32,"/org/bluez/%s",hci);

	pcon->con = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
    if(pcon->con == NULL) {
        g_print("Not able to get connection to system bus\n");
		free(pcon);
        return NULL;
    }
	pcon->loop = g_main_loop_new(NULL, FALSE);
	if (!pending_confirm) pending_confirm = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
	if (!pending_authorize) pending_authorize = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

	/* 导出并注册 Agent，使我们能接收 "Accept pairing (yes/no)" 相关请求 */
	if (bluez_export_agent_object(pcon) != 0) {
		g_print("Export Agent1 object failed, pairing prompts will not be captured.\n");
	}
	bluez_register_agent(pcon,"NoInputNoOutput");
	pthread_create(&pcon->threadid,NULL,bluez_msg_thread,(void*)pcon);
   	return pcon;
}

void bluez_con_deinit(bluez_con_info *pcon)
{
	bluez_unregister_agent(pcon);
	g_main_loop_quit(pcon->loop);
	pthread_join(pcon->threadid,NULL);
    g_object_unref(pcon->con);
	/* 注销导出的 Agent1 对象并释放资源 */
	if (agent_reg_id && pcon && pcon->con) {
		g_dbus_connection_unregister_object((GDBusConnection*)pcon->con, agent_reg_id);
		agent_reg_id = 0;
	}
	if (agent_node_info) {
		g_dbus_node_info_unref(agent_node_info);
		agent_node_info = NULL;
	}
	g_agent_pcon = NULL;
	if (pending_confirm) { g_hash_table_destroy(pending_confirm); pending_confirm = NULL; }
	if (pending_authorize) { g_hash_table_destroy(pending_authorize); pending_authorize = NULL; }
	free(pcon);
}
int bluez_start_discovery(bluez_con_info *pcon)
{
	pthread_t threadid;
	int prop_val;
	if (!pcon) {
		return -1;
	}
	bluez_adapter_set_property(pcon,"Powered", g_variant_new("b", TRUE));
	
	prop_val = adaptor_is_discoverying(pcon);
	if (prop_val == 1) {
		g_print("the apdapter is discoverying\n");
		return 0;
	}
					
	if (pthread_create(&threadid,NULL,start_discovery,pcon) != 0) {
		g_print("create discovery thread failed\n");
		return -1;
	}	
	
	return 0;
}

int bluez_stop_discovery(bluez_con_info *pcon)
{
	int rc;

	if(!pcon) {
		return -1;
	}
	
	rc = bluez_adapter_call_method(pcon,"StopDiscovery");
    if(rc) {
		g_print("Not able to stop scanning\n");
		return -1;
	}
	return 0;
}

/*
dev_mac: device mac addr:		xx:xx:xx:xx:xx:xx
*/
int bluez_connect_device(bluez_con_info *pcon,char *dev_mac)
{
	GVariant *result;
    GError *error = NULL;
	gchar obj_path[128];
	int i = 2;
	gchar btmac[18];

	if(!pcon || !dev_mac) {
		return -1;
	}
	
	memcpy(btmac,dev_mac,17);
	while (i < 17) {
		btmac[i] = '_';
		i += 3;
	}
	btmac[17] = 0;
	memset(obj_path,0,sizeof(obj_path));
	sprintf(obj_path,"%s/dev_%s",pcon->hci_path,btmac);

	/* Default to Connect-only (no Pair) to support non-bonding workflow */
	result = g_dbus_connection_call_sync(pcon->con,
						 "org.bluez",
						 obj_path,
						 "org.bluez.Device1",
						 "Connect",
						 NULL,
						 NULL,
						 G_DBUS_CALL_FLAGS_NONE,
						 -1,
						 NULL,
						 &error);
	if (error != NULL) {
		g_print("Failed to connect to %s: %s\n", dev_mac, error->message);
		g_error_free(error);
		return -1;
	}
	g_variant_unref(result);

	return 0; 
}

/**
 * @description: 主动对指定设备发起配对（调用 org.bluez.Device1.Pair）
 * @param pcon - [in] 蓝牙连接信息结构体指针
 * @param dev_mac - [in] 设备MAC地址，格式为 xx:xx:xx:xx:xx:xx
 * @return 0: 成功, -1: 失败
 */
int bluez_pair_device(bluez_con_info *pcon, char *dev_mac)
{
	GVariant *result;
	GError *error = NULL;
	gchar obj_path[128];
	int i = 2;
	gchar btmac[18];

	if (!pcon || !dev_mac) {
		return -1;
	}

	memcpy(btmac, dev_mac, 17);
	while (i < 17) {
		btmac[i] = '_';
		i += 3;
	}
	btmac[17] = 0;
	memset(obj_path, 0, sizeof(obj_path));
	sprintf(obj_path, "%s/dev_%s", pcon->hci_path, btmac);

	result = g_dbus_connection_call_sync(pcon->con,
						 "org.bluez",
						 obj_path,
						 "org.bluez.Device1",
						 "Pair",
						 NULL,
						 NULL,
						 G_DBUS_CALL_FLAGS_NONE,
						 -1,
						 NULL,
						 &error);
	if (error != NULL) {
		g_print("Failed to pair with %s: %s\n", dev_mac, error->message);
		g_error_free(error);
		return -1;
	}

	g_variant_unref(result);
	return 0;
}


/*********************************************
dev_mac: device mac addr:		xx:xx:xx:xx:xx:xx
**********************************************/
int bluez_disconnect_device(bluez_con_info *pcon,char *dev_mac)
{
	GVariant *result;
    GError *error = NULL;
	gchar obj_path[128];
	int i = 2;
	gchar btmac[18];

	if (!pcon || !dev_mac) {
		return -1;
	}
	memcpy(btmac,dev_mac,17);
	while (i < 17) {
		btmac[i] = '_';
		i += 3;
	}
	btmac[17] = 0;
	memset(obj_path,0,sizeof(obj_path));
	sprintf(obj_path,"%s/dev_%s",pcon->hci_path,btmac);						 
    result = g_dbus_connection_call_sync(pcon->con,
                         "org.bluez",
                    /* TODO Find the adapter path runtime */
                         obj_path,
                         "org.bluez.Device1",
                         "Disconnect",
                         NULL,
                         NULL,
                         G_DBUS_CALL_FLAGS_NONE,
                         -1,
                         NULL,
                         &error);
    if(error != NULL)
        return -1;
    g_variant_unref(result);
    return 0;	
}

int bluez_remove_device(bluez_con_info *pcon,char *dev_mac)
{
	GVariant *result;
	GError *error = NULL;
	gchar obj_path[128];
	int i = 2;
	gchar btmac[18];
	//GMainLoop * loop;

	if (!pcon || !dev_mac)
		return -1;
	memcpy(btmac,dev_mac,17);
	while (i < 17) {
		btmac[i] = '_';
		i += 3;
	}
	btmac[17] = 0;
	memset(obj_path,0,sizeof(obj_path));
	sprintf(obj_path,"%s/dev_%s",pcon->hci_path,btmac);

	
	result = g_dbus_connection_call_sync(pcon->con,
						 "org.bluez",
					/* TODO Find the adapter path runtime */
						 pcon->hci_path,
						 "org.bluez.Adapter1",
						 "RemoveDevice",
						 g_variant_new("(o)", obj_path),
						 NULL,
						 G_DBUS_CALL_FLAGS_NONE,
						 -1,
						 NULL,
						 &error);
	if(error != NULL)
		return 1;
	g_variant_unref(result);	
	return 0;	
}
int bluez_get_available_devices(bluez_con_info *pcon)
{	
	GMainLoop *loop;
	
	if (pcon == NULL) {
		return -1;
	}
	loop = g_main_loop_new(NULL, FALSE);
	if (!loop) {
		return -1;
	}
	g_dbus_connection_call(pcon->con,
				"org.bluez",
				"/",
				"org.freedesktop.DBus.ObjectManager",
				"GetManagedObjects",
				NULL,
				G_VARIANT_TYPE("(a{oa{sa{sv}}})"),
				G_DBUS_CALL_FLAGS_NONE,
				-1,
				NULL,
				(GAsyncReadyCallback)bluez_list_devices,
				loop);

	g_main_loop_run(loop);
	return 0;
}

int bluez_device_is_paired(bluez_con_info *pcon,char *dev_mac)
{
	gchar obj_path[128];
	int i = 2;
	gchar btmac[18];
	int prop_val;
	
	memcpy(btmac,dev_mac,17);
	while (i < 17) {
		btmac[i] = '_';
		i += 3;
	}
	btmac[17] = 0;
	memset(obj_path,0,sizeof(obj_path));
	sprintf(obj_path,"%s/dev_%s",pcon->hci_path,btmac);

	prop_val = device_is_paried(pcon,obj_path);
	return prop_val;
}

int bluez_device_is_connected(bluez_con_info *pcon,char *dev_mac)
{
	gchar obj_path[128];
	int i = 2;
	gchar btmac[18];
	int prop_val;
	
	memcpy(btmac,dev_mac,17);
	while (i < 17) {
		btmac[i] = '_';
		i += 3;
	}
	btmac[17] = 0;
	memset(obj_path,0,sizeof(obj_path));
	sprintf(obj_path,"%s/dev_%s",pcon->hci_path,btmac);

	prop_val = device_is_connected(pcon,obj_path);
	return prop_val;
}

int bluez_controller_power(bluez_con_info *pcon,int power)
{
	int rc = 0;
	if (power) {
		rc = bluez_adapter_set_property(pcon,"Powered", 
					g_variant_new("b", TRUE));
	} else {
		rc = bluez_adapter_set_property(pcon,"Powered", 
					g_variant_new("b", FALSE));
	}
	if (rc) {
		return -1;
	}else {
		return 0;
	}
}

int bluez_get_paired_devices(bluez_con_info *pcon)
{	
	GMainLoop *loop;
	
	if (pcon == NULL) {
		return -1;
	}
	loop = g_main_loop_new(NULL, FALSE);
	if (!loop) {
		return -1;
	}
	g_dbus_connection_call(pcon->con,
				"org.bluez",
				"/",
				"org.freedesktop.DBus.ObjectManager",
				"GetManagedObjects",
				NULL,
				G_VARIANT_TYPE("(a{oa{sa{sv}}})"),
				G_DBUS_CALL_FLAGS_NONE,
				-1,
				NULL,
				(GAsyncReadyCallback)bluez_paired_devices,
				loop);

	g_main_loop_run(loop);
	return 0;
}

/**
 * @description: 设置蓝牙适配器的可配对状态
 * @param pcon - [in] 蓝牙连接信息结构体指针
 * @param pairable - [in] 配对状态，1表示可配对，0表示不可配对
 * @return 0: 成功, -1: 失败
 */
int bluez_set_pairable(bluez_con_info *pcon, int pairable)
{
    int rc = 0;

    if (!pcon) {
        g_print("Invalid connection handle\n");
        return -1;
    }

    if (pairable) {
        rc = bluez_adapter_set_property(pcon, "Pairable", g_variant_new("b", TRUE));
    } else {
        rc = bluez_adapter_set_property(pcon, "Pairable", g_variant_new("b", FALSE));
    }

    if (rc) {
        g_print("Failed to set pairable status\n");
        return -1;
    } else {
        g_print("Pairable status set to %s\n", pairable ? "on" : "off");
        return 0;
    }
}
/**
 * @description: 设置蓝牙适配器的可发现状态
 * @param pcon - [in] 蓝牙连接信息结构体指针
 * @param discoverable - [in] 可发现状态，1表示可发现，0表示不可发现
 * @return 0: 成功, -1: 失败
 */
int bluez_set_discoverable(bluez_con_info *pcon, int discoverable)
{
    int rc = 0;

    if (!pcon) {
        g_print("Invalid connection handle\n");
        return -1;
    }

    if (discoverable) {
        rc = bluez_adapter_set_property(pcon, "Discoverable", g_variant_new("b", TRUE));
    } else {
        rc = bluez_adapter_set_property(pcon, "Discoverable", g_variant_new("b", FALSE));
    }

    if (rc) {
        g_print("Failed to set discoverable status\n");
        return -1;
    } else {
        g_print("Discoverable status set to %s\n", discoverable ? "on" : "off");
        return 0;
    }
}
/**
 * @description: 信任指定的蓝牙设备
 * @param pcon - [in] 蓝牙连接信息结构体指针
 * @param dev_mac - [in] 设备MAC地址，格式为 xx:xx:xx:xx:xx:xx
 * @param trust - [in] 信任状态，1表示信任，0表示不信任
 * @return 0: 成功, -1: 失败
 */
int bluez_trust_device(bluez_con_info *pcon, char *dev_mac, int trust)
{
    GVariant *result;
    GError *error = NULL;
    gchar obj_path[128];
    int i = 2;
    gchar btmac[18];

    if (!pcon || !dev_mac) {
        g_print("Invalid parameters\n");
        return -1;
    }

    // 将MAC地址中的':'替换为'_'以构建对象路径
    memcpy(btmac, dev_mac, 17);
    while (i < 17) {
        btmac[i] = '_';
        i += 3;
    }
    btmac[17] = 0;

    memset(obj_path, 0, sizeof(obj_path));
    sprintf(obj_path, "%s/dev_%s", pcon->hci_path, btmac);

    // 设置设备的信任属性
    result = g_dbus_connection_call_sync(pcon->con,
                         "org.bluez",
                         obj_path,
                         "org.freedesktop.DBus.Properties",
                         "Set",
                         g_variant_new("(ssv)", "org.bluez.Device1", "Trusted",
                                      g_variant_new("b", trust ? TRUE : FALSE)),
                         NULL,
                         G_DBUS_CALL_FLAGS_NONE,
                         -1,
                         NULL,
                         &error);

    if (error != NULL) {
        g_print("Failed to set trust status for device %s: %s\n", dev_mac, error->message);
        g_error_free(error);
        return -1;
    }

    g_variant_unref(result);
    g_print("Device %s trust status set to %s\n", dev_mac, trust ? "trusted" : "untrusted");
    return 0;
}
/**
 * @description: 启用或禁用设备连接时的自动信任功能
 * @param enable - [in] 1表示启用自动信任，0表示禁用
 * @return: void
 */
void bluez_set_auto_trust(int enable)
{
    auto_trust_enabled = enable;
    g_print("Auto-trust feature %s\n", enable ? "enabled" : "disabled");
}

/**
 * @description: 获取自动信任功能的状态
 * @return: 1表示已启用，0表示已禁用
 */
int bluez_get_auto_trust_status(void)
{
    return auto_trust_enabled;
}
/**
 * @description: 设置蓝牙适配器的系统别名（system-alias）
 * @param pcon - [in] 蓝牙连接信息结构体指针
 * @param alias - [in] 新的系统别名字符串
 * @return: 0 成功, -1 失败
 */
int bluez_set_system_alias(bluez_con_info *pcon, const char *alias)
{
    int rc = 0;

    if (!pcon || !alias) {
        g_print("Invalid parameters\n");
        return -1;
    }

    /* 检查别名长度，防止过长的别名 */
    if (strlen(alias) == 0) {
        g_print("Alias cannot be empty\n");
        return -1;
    }

    if (strlen(alias) > 248) { /* BlueZ 的最大别名长度 */
        g_print("Alias too long (max 248 characters)\n");
        return -1;
    }

    /* 使用 bluez_adapter_set_property 设置 Alias 属性 */
    rc = bluez_adapter_set_property(pcon, "Alias", g_variant_new("s", alias));

    if (rc) {
        g_print("Failed to set system alias to '%s'\n", alias);
        return -1;
    } else {
        g_print("System alias set to '%s' successfully\n", alias);
        return 0;
    }
}
/**
 * @description: 获取蓝牙适配器的当前系统别名
 * @param pcon - [in] 蓝牙连接信息结构体指针
 * @param alias - [out] 存储别名的缓冲区
 * @param alias_size - [in] 缓冲区大小
 * @return: 0 成功, -1 失败
 */
int bluez_get_system_alias(bluez_con_info *pcon, char *alias, size_t alias_size)
{
    GVariant *prop;
    const gchar *alias_str;
    GVariant *result;
    GVariant *var = NULL;

    if (!pcon || !alias || alias_size == 0) {
        g_print("Invalid parameters\n");
        return -1;
    }

    /* 创建参数以获取 Alias 属性 */
    var = g_variant_new("(ss)", "org.bluez.Adapter1", "Alias");
    result = bluez_adapter_get_property(pcon->con, pcon->hci_path, var);

    if (!result) {
        g_print("Failed to get system alias\n");
        return -1;
    }

    /* 解析返回的 GVariant */
    g_variant_get(result, "(v)", &prop);
    alias_str = g_variant_get_string(prop, NULL);

    /* 检查缓冲区大小是否足够 */
    if (strlen(alias_str) >= alias_size) {
        g_print("Buffer too small for alias\n");
        g_variant_unref(result);
        return -1;
    }

    /* 复制别名到输出缓冲区 */
    strncpy(alias, alias_str, alias_size - 1);
    alias[alias_size - 1] = '\0'; /* 确保字符串终止 */
    g_variant_unref(result);

    g_print("Current system alias: '%s'\n", alias);
    return 0;
}
