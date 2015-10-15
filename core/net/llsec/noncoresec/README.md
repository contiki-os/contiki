`noncoresec` is a noncompromise-resilient 802.15.4 security implementation, which uses a network-wide key. Add these lines to your `project_conf.h` to enable `noncoresec`:

```c
#undef LLSEC802154_CONF_ENABLED
#define LLSEC802154_CONF_ENABLED          1
#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER              noncoresec_framer
#undef NETSTACK_CONF_LLSEC
#define NETSTACK_CONF_LLSEC               noncoresec_driver
#undef NONCORESEC_CONF_SEC_LVL
#define NONCORESEC_CONF_SEC_LVL           1
```
`NONCORESEC_CONF_SEC_LVL` defines the length of MICs and whether encryption is enabled or not.

Setting the network-wide key works as follows:
```c
#define NONCORESEC_CONF_KEY { 0x00 , 0x01 , 0x02 , 0x03 , \
                              0x04 , 0x05 , 0x06 , 0x07 , \ 
                              0x08 , 0x09 , 0x0A , 0x0B , \ 
                              0x0C , 0x0D , 0x0E , 0x0F } 
```
