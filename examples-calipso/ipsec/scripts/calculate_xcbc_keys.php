<?php

	$cipher = mcrypt_module_open(MCRYPT_RIJNDAEL_128, '', MCRYPT_MODE_ECB, '');

	$iv =  '0000000000000000';
	$aeskey =  pack("H*", $argv[1]);
	printf("AES key: %s\n",bin2hex($aeskey));

	if (mcrypt_generic_init($cipher, $aeskey, $iv) != -1) {
		for ($i = 0; $i < 3; $i++) {
			$input = '';
			for ($j = 0; $j < 16; $j++) {
				$input .= pack("H*", '0'.($i+1));
			}
			printf("\ninput: %s\n",bin2hex($input));
			$xcbckey = mcrypt_generic($cipher,$input );
			printf("K%d: {0x%s", $i+1, bin2hex($xcbckey[0]));
			for ($j = 1; $j < 16; $j++) {
				printf(",0x%s",bin2hex($xcbckey[$j]));
			}
			printf("}\n");
		}
	}

?>