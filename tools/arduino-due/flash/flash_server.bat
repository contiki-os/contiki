mode com3:1200,n,8,1
TIMEOUT 1
bossac --port=COM3 -U false -i
bossac --port=COM3 -U false -e -w -v -b ..\..\..\Debug\module_80211_dtls_server.bin -R