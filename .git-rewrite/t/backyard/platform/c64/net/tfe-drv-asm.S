
;---------------------------------------------------------------------       
	.export		_cs8900a_rxtxreg
	.export		_cs8900a_txcmd
	.export		_cs8900a_txlen
	.export		_cs8900a_packetpp
	.export		_cs8900a_ppdata
	

;---------------------------------------------------------------------

	_cs8900a_rxtxreg	= $de00
	_cs8900a_txcmd		= $de04
	_cs8900a_txlen		= $de06
	_cs8900a_packetpp	= $de0a
	_cs8900a_ppdata		= $de0c

