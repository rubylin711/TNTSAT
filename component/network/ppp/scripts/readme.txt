1. ppp/
  包含与运营商和3G Dongle厂商相关的参数配置脚本
  gprs-3gnetdial:
    中国联通设置，包含APN信息以及需要发送的AT Command
    APN信息：
      名称：3gnet
      用户名：无
      密码：无
      拨号命令：*99#

  gprs-cmnetchat：
    中国移动网络设置，包含APN信息以及必要的AT Command
    APN信息：
      名称：cmnet
      用户名：无
      密码：无
      拨号命令：*99#
        注意：
          移动TD-SCDMA拨号命令应当为*98*1# 
          移动GPRS/EDGE拨号命令应当未*99***1#
          但是*99#表示使用手机或者上网卡默认的连接方式拨号，所以一般使用*99#
      
  gprs-ctnetchat：
    中国移动网络设置，包含APN信息以及必要的AT Command
    APN信息：
      名称：ctnet
      用户名：ctnet@mycdma.cn
      密码：vnet.mobi
        注意，理论上大陆境内运营商普通上网卡上网军不需要使用APN用户名和密码
      拨号命令：*99#
        注意：
          电信拨号命令应当为#777#
          但是*99#表示使用手机或者上网卡默认的连接方式拨号，所以一般使用*99#

  ip-down：
    断开时删除网络配置，例如dns设定等

  ip-up：
    连接时配置网络，例如DNS,如有其它网络存在，例如Ethernet，wifi，则可能需要添加路由设置

  options：
    pppd程序的参数设定，不要更改

  ppp-off:
    断开PPP拨号脚本，直接调用执行即可。
    
2.ppp/peers/
  该目录包含pppd的拨号配置和执行脚本，pppd首先调用ppp/peers/下的脚本，获取到配置信息，根据配置信息再调用ppp下的脚本
  参数说明：
    115200，波特率，可以根据3G Dongle的参数来选择波特率
    debug，调试选项，去掉之后会减少log输出
    user，password，apn账户密码，没有需求的情况下可以频闭
    connect，连接时需要执行的脚本，这里指向../gprs-3gxxchat

    gprs-3gnetdial:
      中国联通拨号脚本
    gprs-3gctnetchat：
      中国电信拨号脚本
    gprs-ctnetdial:
      中国移动拨号脚本

3. 连接与断开
  连接--执行命令(以联通为例)：
  pppd call gprs-3gnet /dev/ttyUSB0
  注意，ttyUSB0是根据3G Dongle型号获取的，不通的型号这个数值不一样，可以是ttyUSB1,ttyUSB2等，需要先确定Dongle使用哪个端口拨号

  断开--执行命令
  ppp-off
