all: udp-client udp-server


#CFLAGS+=-DPROJECT_CONF_H=\"project-conf.h\"
APPS=servreg-hack
CONTIKI=../..


ifdef WITH_COMPOWER
APPS+=powertrace
CFLAGS+= -DCONTIKIMAC_CONF_COMPOWER=1 -DWITH_COMPOWER=1 -DQUEUEBUF_CONF_NUM=4
endif

ifdef SERVER_REPLY
CFLAGS+=-DSERVER_REPLY=$(SERVER_REPLY)
endif
ifdef PERIOD
CFLAGS+=-DPERIOD=$(PERIOD)
endif
ifdef SIM_POWER
CFLAGS+=-DSIM_POWER=$(SIM_POWER)
endif
ifdef RPL_CONF_FIXED_DIO
CFLAGS+=-DRPL_CONF_FIXED_DIO=$(RPL_CONF_FIXED_DIO) 
endif
ifdef RPL_CONF_DIO_ON_INCONSISTENCY
CFLAGS+=-DRPL_CONF_DIO_ON_INCONSISTENCY=$(RPL_CONF_DIO_ON_INCONSISTENCY)
endif
ifdef RPL_CONF_DIO_DAO_ON_NEW_PARENT
CFLAGS+=-DRPL_CONF_DIO_DAO_ON_NEW_PARENT=$(RPL_CONF_DIO_DAO_ON_NEW_PARENT)
endif
ifdef RPL_CONF_PROBE_ON_NEW_NEIGHBOR
CFLAGS+=-DRPL_CONF_PROBE_ON_NEW_NEIGHBOR=$(RPL_CONF_PROBE_ON_NEW_NEIGHBOR)
endif
ifdef RPL_CONF_REVERSE_TRICKLE
CFLAGS+=-DRPL_CONF_REVERSE_TRICKLE=$(RPL_CONF_REVERSE_TRICKLE)
endif
ifdef RPL_CONF_MOBILE
CFLAGS+=-DRPL_CONF_MOBILE=$(RPL_CONF_MOBILE)
endif

#touch rpl-conf.h to recompile dependant files with new conf values
touch:
	touch $(CONTIKI)/core/net/rpl/*
	
udp-client-static.sky: touch udp-client.sky
	mv udp-client.sky udp-client-static.sky
	
udp-client-mobile.sky: touch udp-client.sky
	mv udp-client.sky udp-client-mobile.sky
	
udp-server-custom.sky: touch udp-server.sky touch
	mv udp-server.sky udp-server-custom.sky


CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
