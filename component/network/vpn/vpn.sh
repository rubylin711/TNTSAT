#!/bin/sh

#parameter description
#ACTION: connect/disconnect
#MODE: l2tp/openvpn/ikev2
#SERVER: VPN address
#PSK: only support PSK authentication, so this is pre-shared key
#USER: vpn username
#PASSWD: vpn password
#IPV6: support IPV6 or not
IPSEC="no"
IPV6="no"
useage(){
  echo "==================parameter description================"
  echo "action: connect/disconnect/status"
  echo "mode: select vpn mode. l2tp/openvpn/ikev2"
  echo "server: vpn server,can be IP:Port or domain name"
  echo "psk: pre-shared key for PSK VPN authenticaion"
  echo "user: user name for vpn client"
  echo "passwd: user password for vpn client"
  echo " "
  echo "========================use cases======================"
  echo "1. connect L2TP Raw VPN without IPV6"
  echo "vpn.sh action=connect mode=l2tp server=10.48.144.253 psk=www.montage.vpn user=rwei passwd=11111111 <ipsec=no> <ipv6=no>"
  echo "2. connect L2TP IPsec VPN without IPV6"
  echo "vpn.sh action=connect mode=l2tp server=10.48.144.253 psk=www.montage.vpn user=rwei passwd=11111111 ipsec=yes <ipv6=no>"
  echo "3. connect OPEN VPN without IPV6"
  echo "vpn.sh action=connect mode=openvpn server=10.48.144.253 psk=www.montage.vpn user=rwei passwd=11111111"
  echo "4. disconnect from L2TP Raw VPN without IPV6"
  echo "vpn.sh action=disconnect mode=l2tp"
  echo "5. disconnect from L2TP IPSec VPN without IPV6"
  echo "vpn.sh action=disconnect mode=l2tp ipsec=yes"
  echo "6. get connect status from L2TP Raw VPN without IPV6"
  echo "vpn.sh action=status mode=l2tp ipsec=yes"
  echo "7. get connect status from L2TP IPSec VPN without IPV6"
  echo "vpn.sh action=status mode=l2tp"
  echo "  returns status(connected/disconnec),VPN IP"
}

parse_args() {
  for arg in "$@"; do
      # 判断参数是否包含等号
      case $arg in
          *=*)
              key=$(echo "$arg" | cut -d= -f1)
              val=$(echo "$arg" | cut -d= -f2-)
              key=$(echo "$key" | tr '[:lower:]-' '[:upper:]_')
              eval "${key}='${val}'"
              ;;
          *)
              useage;;
      esac
  done
}


#setup environment
#l2tp mode: vpn_pre_setup l2tp <ipv6>
#ikev2 mode: vpn_pre_setup ikev2 <ipv6>
#openvpn mode: vpn_pre_setup openvpn
vpn_pre_setup()
{
  if [ x"${IPSEC}" = "xyes" ];then
    modprobe xfrm4_tunnel 2>/dev/null
  fi

  if [ x"${IPV6}" = "xipv6" ];then
    modprobe ipv6 2>/dev/null
    if [ x"${IPSEC}" = "xyes" ];then
      modprobe xfrm6_tunnel 2>/dev/null
    fi
  fi

  if [ x"${MODE}" = "xl2tp" ];then
    modprobe ppp_async 2>/dev/null

    modprobe l2tp_ppp 2>/dev/null
    mkdir -p /var/run/xl2tpd -p 2>/dev/null
    mkdir -p /tmp/xl2tpd -p 2>/dev/null
    mkdir -p /etc/swanctl 2>/dev/null
  else #OPENVPN
    modprobe tun 2>/dev/null
    mkdir -p /var/log/openvpn  2>/dev/null
    mkdir -p /tmp/ovpncfg 2>/dev/null
  fi
}

# now only support PSK
# ipsec_psk_setup vpn_name server psk
#
ipsec_psk_setup()
{
  OUTFILE="/etc/swanctl/swanctl.conf"

  cat > "$OUTFILE" <<EOF
  connections {
    vpn {
      local_addrs  = %any
      remote_addrs = ${SERVER}

      local {
        auth = psk
        id = %any
      }
      remote {
        auth = psk
        id = ${SERVER}
      }

      children {
        vpn {
          local_ts = 0.0.0.0/0
          remote_ts = 0.0.0.0/0
          updown = /usr/libexec/ipsec/_updown iptables
          esp_proposals = aes128-sha1
          mode = transport
        }
      }
      version = 1
      proposals = aes128-sha1-modp1024
    }
  }

  secrets {
    ike-ipsec {
      id-1 = %any
      id-2 = ${SERVER}
      secret = ${PSK}
    }
  }
EOF
}



# now only support PSK
# l2tp_psk_setup vpn_name server ipsec/raw
#
l2tp_psk_setup()
{
  SAREF="no"
  OUTFILE="/tmp/xl2tpd/xl2tpd.conf"

  if [ x"${IPSEC}" = "xyes" ];then
    SAREF="yes"
  fi

  cat > "$OUTFILE" <<EOF
  [global]
  port = 1701
  ipsec saref = $SAREF

  [lac vpn]
  lns = ${SERVER}
  ppp debug = yes
  pppoptfile = /etc/ppp/options.l2tpd
  require authentication = yes
  length bit = yes

EOF
  sleep 0.5

  #Start xl2tpd
  killall xl2tpd 2>/dev/null
  xl2tpd -c /tmp/xl2tpd/xl2tpd.conf -D &
}


# now only support PSK
# ppp_setup vpn_name username password
#
ppp_setup()
{
  OUTFILE="/etc/ppp/options.l2tpd"

  cat > "$OUTFILE" <<EOF

  refuse-pap
  require-chap
  noauth
  crtscts
  debug
  logfile /var/log/pppd.log
  name ${USER}
  password ${PASSWD}

EOF

  #chap-secrets
  OUTFILE="/etc/ppp/chap-secrets"

  cat > "$OUTFILE" <<EOF

  ${USER}            *     "${PASSWD}"  0.0.0.0/0

EOF
}

l2tp_vpn_connect()
{
  echo ">>>prepare L2TP for VPN...."
  l2tp_psk_setup

  echo ">>>prepare PPP for VPN...."
  ppp_setup

  if [ x"${IPSEC}" = "xyes" ];then
    echo ">>>Connect to L2TP/IPsec VPN Server...."
    killall charon
    /usr/local/bin/charon 2 > /dev/null &
    echo ">>>starting strongswan deamon...."
    sleep 1
    swanctl --load-all vpn
    swanctl --initiate  --child vpn
  else
    echo ">>>Connect to L2TP/Raw VPN Server...."
  fi
  echo "c vpn" > /var/run/xl2tpd/l2tp-control

}

openvpn_connect()
{

  #generate /tmp/ovpncfg/ca.cert,it's a demo root CA
  cat >"/tmp/ovpncfg/ca.cert" <<EOF
-----BEGIN CERTIFICATE-----
MIID0DCCArigAwIBAgIBADANBgkqhkiG9w0BAQsFADBnMRwwGgYDVQQDDBN3bGlh
by5zb2Z0ZXRoZXIubmV0MRwwGgYDVQQKDBN3bGlhby5zb2Z0ZXRoZXIubmV0MRww
GgYDVQQLDBN3bGlhby5zb2Z0ZXRoZXIubmV0MQswCQYDVQQGEwJVUzAeFw0yNTEw
MjIwODExMjBaFw0zNzEyMzEwODExMjBaMGcxHDAaBgNVBAMME3dsaWFvLnNvZnRl
dGhlci5uZXQxHDAaBgNVBAoME3dsaWFvLnNvZnRldGhlci5uZXQxHDAaBgNVBAsM
E3dsaWFvLnNvZnRldGhlci5uZXQxCzAJBgNVBAYTAlVTMIIBIjANBgkqhkiG9w0B
AQEFAAOCAQ8AMIIBCgKCAQEAp+1NuCMhtXai3fLR7AlqNJNs0D2kppHPIBy6DIoF
a3VDcqiO7DWsvs29q10AA7uVMw3yu51KGs4rENM3Rr2xBvu8ENjzlr2QzTABh6sR
6RungiAUk5NhdbsjypnX0MNiRAUjYH7GpxCNpTnh675uefCPmeAyUVxvim+6leeT
z4sbw+XqC8DYF3Q+8q37rudhJZkxL9h3OQ9VJfes3FACiSgeXFdgkRQHi7A+Hzwf
21lqcWCSo3VSc6WvlwF7FoJWjSmgMv09zsKZocMzT6a7rmMWIvEEXI70RjpcsLoT
rTP0W2V1n5V/vMlFYR4/yAgXFJeYUoyWYfBvdeINQ8coEQIDAQABo4GGMIGDMA8G
A1UdEwEB/wQFMAMBAf8wCwYDVR0PBAQDAgH2MGMGA1UdJQRcMFoGCCsGAQUFBwMB
BggrBgEFBQcDAgYIKwYBBQUHAwMGCCsGAQUFBwMEBggrBgEFBQcDBQYIKwYBBQUH
AwYGCCsGAQUFBwMHBggrBgEFBQcDCAYIKwYBBQUHAwkwDQYJKoZIhvcNAQELBQAD
ggEBAGLqAw7W8reOzDKSrOJusqKMCMgCx0iTk8qUflGENXtvm3WJhXGyPMCgDw8h
ib1fhanJi4xp4Gsuh/dXJHLNjH84MEaJgptk4ae46KBMcIoC/7gHb9ll4xZmuToo
Lhrf2HDSTSyQ1d7eetDNunm262vMYRHQoLAALkgJAI0NHJ6rXXO3VqZ2hjYlaQ7l
WSjc/YIqwH5tV9qZhN8br5zStR9+n2t4P3rQFH4ez3IrawNtA2l9WV06q0hTGLa+
A1GfFWmzbA8Ot7IB9Zi2cSQ3Dm16qSrpqZepkFjokrnlSNEbZunI2/UO3Rv9D80j
0KRhaWCBywUwfedSZgiALfXHNTg=
-----END CERTIFICATE-----

EOF

  #generate /tmp/ovpncfg/auth.txt, soppose the vpn user is wliao,12345678
  cat >"/tmp/ovpncfg/auth.txt" <<EOF
${USER}
${PASSWD}
EOF
  #generate /tmp/ovpncfg/client.ovpn, base on static user/password + server root certification
  cat >"/tmp/ovpncfg/client.ovpn" <<EOF

  client
  dev tun
  proto udp

  remote ${SERVER} 1194
  resolv-retry infinite
  nobind

  persist-key
  persist-tun

  auth-user-pass /tmp/ovpncfg/auth.txt

  auth-nocache

  data-ciphers AES-256-GCM:AES-128-CBC:aes-256-cbc
  data-ciphers-fallback aes-256-cbc
  auth SHA256

  ca /tmp/ovpncfg/ca.cert

  remote-cert-eku "TLS Web Server Authentication"

  script-security 3

  log-append /var/log/openvpn/client.log
  verb 3

EOF

  #start openvpn deamon
  killall openvpn
  sleep 0.5
  openvpn --config /tmp/ovpncfg/client.ovpn --daemon
}

l2tp_disconnect()
{
  echo ">>>teardown ppp conversation..."
  echo "d vpn" > /var/run/xl2tpd/l2tp-control
  sleep 0.5
  if [ x"${IPSEC}" = "xyes" ];then
    echo ">>>teardown IPsec conversation..."
    swanctl --terminate --force --ike vpn
  fi
  return
}

openvpn_disconnect()
{
  echo ">>>teardown openvpn conversation..."
  killall openvpn
}

vpn_status()
{
  iface=`ifconfig $1 2>/dev/null | grep "inet"`
  if [ x"$iface" = "x" ];then
    echo "disconnect"
    return
  fi
  #connected
  ipaddr=`echo "$iface" | awk -F ":" '{print $2}' | awk -F " " '{print $1}'`
  gateway=`echo "$iface" | awk -F ":" '{print $3}' | awk -F " " '{print $1}'`
  echo "connected,ip=${ipaddr},gw=${gateway}"
}




#=====================================main start===============================
# vpn.sh [connect] <mode> <server> <psk> <username> <password> <ipsec> <ipv6>"
# mode - l2tp/openvpn/ikev2
# psk - pre
# vpn_pre_setup [l2tp/openvpn/ikev2] <ipv6>

# Parameter valid check
if [ $# -le 1 ] ;then
  useage
  return
fi
echo ">>>Parse input arguments...."
parse_args "$@"

echo ">>>dump input arguments...."
echo "ACTION=$ACTION"
echo "MODE=$MODE"
echo "SERVER=$SERVER"
echo "PSK=$PSK"
echo "USER=$USER"
echo "PASSWD=$PASSWD"
echo "IPSEC=$IPSEC"
echo "IPV6=$IPV6"
echo ">>>prepare for VPN environment...."

#handle disconnect or status if needed
if [ x"${ACTION}" = "xdisconnect" ];then
  if [ x"${MODE}" = "xl2tp" ];then
    l2tp_disconnect
  elif [ x"${MODE}" = "xopenvpn" ];then
    openvpn_disconnect
  fi

  return
elif [ x"${ACTION}" = "xstatus" ];then
  if [ x"${MODE}" = "xl2tp" ];then
    vpn_status ppp0
  elif [ x"${MODE}" = "xopenvpn" ];then
    vpn_status tun0
  fi
  return
fi

#handle connect
vpn_pre_setup

if [ x"${IPSEC}" = "xyes" ];then
  echo ">>>prepare IPSec for VPN...."
  ipsec_psk_setup
fi

if [ x"${MODE}" = "xl2tp" ];then
  l2tp_vpn_connect
elif [ x"${MODE}" = "xopenvpn" ];then
  openvpn_connect
fi