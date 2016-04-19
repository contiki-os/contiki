Link layer security is implemented as a new netstack layer, which is located in between the MAC and the NETWORK layer. The interface of LLSEC drivers is defined in [llsec.h](llsec.h). Additionally, LLSEC drivers may define a special [FRAMER](../mac/framer.h), which calls the actual FRAMER underneath. By default, the LLSEC driver `nullsec` is used, which does nothing.

# TODO

* Most main files do not call LLSEC.init, yet