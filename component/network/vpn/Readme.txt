VPN使用说明

支持的VPN类型：
  L2TP/Raw VPN，即使用L2TP隧道，通过PPP加密认证(VPN 用户名和密码)
  L2TP/IPSec VPN 的PSK模式(IPV4),即使用L2TP隧道，通过PPP加密认证(VPN 用户名和密码)后再经IPSec加密，基于IKEv1实现，
    IKEv1使用最简单的PSK认证模式，其他如证书模式请自行验证实施
  OPENVPN,即使用tun 3层网络隧道，使用aes cbc加密，需要认证VPN服务器根证书，使用vpn 用户名和密码认证

  L2TP VPN验证过Linux的VPN和Windows VPN(softethernet)
  OPENVPN 仅验证过Windows VPN(softethernet)

测试脚本vpn.sh说明
  关于L2TP,OPENVPN的配置文件等，绝大部分文件都是即时生成，后续如果要自己构建VPN控制，可以根据生成的配置文件提前写入文件系统。
 
  IPSec开启时，strongswan的配置文件 stongswan.conf采用文件存储


1. L2TP/Raw VPN
  测试环境：softether搭建服务器，用户信息： rwei，11111111，VPN server IP:10.48.144.253
  测试指令:
    连接VPN - vpn.sh action=connect mode=l2tp server=10.48.144.253 psk=www.montage.vpn user=rwei passwd=11111111
    查询VPN状态 - vpn.sh action=status mode=l2tp
    断开VPN - vpn.sh action=disconnect mode=l2tp

2. L2TP/IPSec VPN
  测试环境：softether搭建服务器，用户信息： rwei，11111111，VPN server IP:10.48.144.253
  测试指令:
    连接VPN - vpn.sh action=connect mode=l2tp server=10.48.144.253 psk=www.montage.vpn user=rwei passwd=11111111 ipsec=yes
    查询VPN状态 - vpn.sh action=status mode=l2tp
    断开VPN - vpn.sh action=disconnect mode=l2tp ipsec=yes

3. OPENVPN 用户+根证书
  测试环境: softether搭建服务器，导出根证书用于openvpn使用，另外提供用户信息 wliao,12345678
    测试L3层远程访问功能
  测试指令:
    连接VPN - vpn.sh action=connect mode=openvpn server=10.200.200.21 user=wliao passwd=12345678
    查询VPN状态 - vpn.sh action=status mode=openvpn
    断开VPN - vpn.sh action=disconnect mode=openvpn
