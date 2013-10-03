<?php

error_reporting(0);

$cfg  = chr($_GET['addr-a']).chr($_GET['addr-b']).chr($_GET['addr-c']).chr($_GET['addr-d']);
$cfg .= chr($_GET['mask-a']).chr($_GET['mask-b']).chr($_GET['mask-c']).chr($_GET['mask-d']);
$cfg .= chr($_GET['dgw-a']). chr($_GET['dgw-b']). chr($_GET['dgw-c']). chr($_GET['dgw-d']);
$cfg .= chr($_GET['dns-a']). chr($_GET['dns-b']). chr($_GET['dns-c']). chr($_GET['dns-d']);

switch ($_GET['machine']) {
case 'apple2':
    $hex = $_GET['apple2-addr'];
    $drv = $_GET['apple2-drv'];
    break;
case 'atari':
    $hex = strtok($_GET['atari-addr-drv'], '-');
    $drv = strtok('-');
    break;
case 'c64':
    $hex = strtok($_GET['c64-addr-drv'],  '-');
    $drv = strtok('-');
    break;
case 'c128':
    $hex = strtok($_GET['c128-addr-drv'], '-');
    $drv = strtok('-');
    break;
}

$addr = hexdec($hex);
$cfg .= chr($addr % 0x100).chr($addr / 0x100);
$cfg .= $drv;

header('Content-Type: application/octetstream');
header('Content-Disposition: attachment; filename=contiki.cfg');
print($cfg);

?>
