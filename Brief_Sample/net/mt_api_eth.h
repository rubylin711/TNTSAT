#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/* eth */
typedef enum mtEth_LinkStatus_E
{
    ETH_LINK_STATUS_OFF = 0,
    ETH_LINK_STATUS_ON,
    ETH_LINK_STATUS_MAX
} Eth_LinkStatus_E, *PTR_Eth_LinkStatus_E;

typedef enum mtEth_Port_E
{
    ETH_PORT_ETH0 = 0,
    ETH_PORT_ETH1,
    ETH_PORT_MAX
} Eth_Port_E;

typedef enum mtEth_WorkMode_E
{
    ETH_WORKMODE_UMDS = 0,
    ETH_WORKMODE_DMUS,
    ETH_WORKMODE_UMDM,
    ETH_WORKMODE_MAX
} Eth_WorkMode_E;

typedef enum mtEth_LinkCfg_E
{
    ETH_LinkCfg_10M_HALF = 0,
    ETH_LinkCfg_10M_FULL,
    ETH_LinkCfg_100M_HALF,
    ETH_LinkCfg_100M_FULL,
    ETH_LinkCfg_AUTONEG,
    ETH_LinkCfg_MAX
} Eth_LinkCfg_E;

typedef enum mtEth_MacTblIdx_E
{
    ETH_MACTBL_INDEX_0 = 0,
    ETH_MACTBL_INDEX_1,
    ETH_MACTBL_INDEX_2,
    ETH_MACTBL_INDEX_3,
    ETH_MACTBL_INDEX_4,
    ETH_MACTBL_INDEX_5,
    ETH_MACTBL_INDEX_6,
    ETH_MACTBL_INDEX_7,
    ETH_MACTBL_INDEX_MAX
} Eth_MacTblIdx_E;

typedef enum mtEth_VlanTblIdx_E
{
    ETH_VLANTBL_INDEX_0 = 0,
    ETH_VLANTBL_INDEX_1,
    ETH_VLANTBL_INDEX_2,
    ETH_VLANTBL_INDEX_3,
    ETH_VLANTBL_INDEX_4,
    ETH_VLANTBL_INDEX_5,
    ETH_VLANTBL_INDEX_6,
    ETH_VLANTBL_INDEX_7,
    ETH_VLANTBL_INDEX_MAX
} Eth_VlanTblIdx_E;

typedef enum mtEth_Forward_E
{
    ETH_FW_ALL2CPU_D = 0,
    ETH_FW_ALL2UP_D,
    ETH_FW_ENA2CPU_D,
    ETH_FW_ENA2UP_D,
    ETH_FW_ALL2CPU_U,
    ETH_FW_ALL2UP_U,
    ETH_FW_ENA2CPU_U,
    ETH_FW_ENA2UP_U,
    ETH_FW_REG_CFG,
    ETH_FW_MAX
} Eth_Forward_E;

typedef enum mtEth_Mactctrl_E
{
    ETH_MACT_BROAD2CPU_D = 0,
    ETH_MACT_BROAD2UP_D,
    ETH_MACT_MULTI2CPU_D,
    ETH_MACT_MULTI2UP_D,
    ETH_MACT_UNI2CPU_D,
    ETH_MACT_UNI2UP_D,
    ETH_MACT_BROAD2CPU_U,
    ETH_MACT_BROAD2DOWN_U,
    ETH_MACT_MULTI2CPU_U,
    ETH_MACT_MULTI2DOWN_U,
    ETH_MACT_UNI2CPU_U,
    ETH_MACT_UNI2DOWN_U,
    ETH_MACT_REG_CFG,
    ETH_MACT_MAX
} Eth_Mactctrl_E;

MT_S32	MT_ETH_Open(Eth_Port_E ePort);
MT_S32	MT_ETH_Close(Eth_Port_E ePort);
MT_S32	MT_ETH_IPAddressSet (Eth_Port_E ePort, MT_CHAR* ipAdd);
MT_S32	MT_ETH_IPAddressGet (Eth_Port_E ePort, MT_CHAR *ipAdd);
MT_S32	MT_ETH_SubNetmaskSet (Eth_Port_E ePort, MT_CHAR* subNetmask);
MT_S32	MT_ETH_SubNetmaskGet (Eth_Port_E ePort, MT_CHAR* subNetmask);
MT_S32	MT_ETH_GatewaySet (Eth_Port_E ePort, MT_CHAR* gateway);
MT_S32	MT_ETH_GatewayGet (Eth_Port_E ePort, MT_CHAR* gateway);
MT_S32	MT_ETH_DNSSet (Eth_Port_E ePort, MT_BOOL enable, MT_CHAR *dns);
MT_S32	MT_ETH_DNSGet (Eth_Port_E ePort, MT_CHAR *dns);
MT_S32	MT_ETH_SetMac(Eth_Port_E ePort, MT_CHAR *mac);
MT_S32	MT_ETH_GetMac(Eth_Port_E ePort, MT_CHAR *mac);
MT_S32	MT_ETH_GetLinkStatus(Eth_Port_E ePort, PTR_Eth_LinkStatus_E ptrLinkStatus);
MT_S32	MT_ETH_GetLinkStatus_Special(Eth_Port_E ePort, PTR_Eth_LinkStatus_E ptrLinkStatus);
MT_S32	MT_ETH_LinkCfg(Eth_Port_E ePort, Eth_LinkCfg_E eLinkCfg);
MT_S32	MT_ETH_WorkModeSet(Eth_WorkMode_E eWorkMode);
MT_S32	MT_ETH_FwCtrlSet(Eth_Forward_E eItem, MT_U32 u32Fw);
MT_S32	MT_ETH_MactCtrlSet(Eth_Mactctrl_E eItem, MT_U32 u32Macctrl);
MT_S32	MT_ETH_MacTableEnable(Eth_Port_E ePort);
MT_S32	MT_ETH_MacTableDisable(Eth_Port_E ePort);
MT_S32	MT_ETH_MacTableSet(Eth_MacTblIdx_E eMacTblIndex, MT_U32 u32MacHi16, MT_U32 u32MacLo32);
MT_VOID MT_ETH_VlanEnable(MT_VOID);
MT_VOID MT_ETH_VlanDisable(MT_VOID);
MT_S32	MT_ETH_VlanTableSet(Eth_VlanTblIdx_E eVlanTblIndex, MT_U32 u32Vlan);
MT_VOID MT_ETH_CpuVlanCtrlSet(MT_U32 u32CpuVlanCtrl);
MT_VOID MT_ETH_NoVlanCtrlSet(MT_U32 u32NoVlanCtrl);
MT_VOID MT_ETH_SpecVlanCtrlSet(MT_U32 u32SpecVlanCtrl);
MT_VOID MT_ETH_UnkownVlanSet(MT_U32 u32UnkownVlanCtrl);
MT_VOID MT_ETH_PortVlanIdSet(MT_U32 u32PortVlanId);
MT_S32	MT_ETH_FcTimeCtrl(Eth_Port_E ePort, MT_U32 u32FcTimeCtrl);
MT_S32	MT_ETH_FcRxLimit(Eth_Port_E ePort, MT_U32 u32FcRxLimit);
MT_S32	MT_ETH_FcDropCtrl(Eth_Port_E ePort, MT_U32 u32FcDropCtrl);
MT_S32	MT_ETH_FcLevel(Eth_Port_E ePort, MT_U32 u32FcLevel);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */
