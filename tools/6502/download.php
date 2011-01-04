<?php

error_reporting(0);

$cfg  = chr($_GET['addr-a']).chr($_GET['addr-b']).chr($_GET['addr-c']).chr($_GET['addr-d']);
$cfg .= chr($_GET['mask-a']).chr($_GET['mask-b']).chr($_GET['mask-c']).chr($_GET['mask-d']);
$cfg .= chr($_GET['dgw-a']). chr($_GET['dgw-b']). chr($_GET['dgw-c']). chr($_GET['dgw-d']);
$cfg .= chr($_GET['dns-a']). chr($_GET['dns-b']). chr($_GET['dns-c']). chr($_GET['dns-d']);

$dsk = $_GET['disk'];

switch ($_GET['machine']) {
case 'apple2':
    $hex = $_GET['apple2-addr'];
    $drv = $_GET['apple2-drv'];
    $ext = '-' . $dsk . '.dsk';
    $ofs = 0x05B00;
    break;
case 'c64':
    $hex = strtok($_GET['c64-addr-drv'],  '-');
    $drv = strtok('-');
    $ext = '-' . $dsk . '.d64';
    $ofs = 0x15002;
    break;
case 'c128':
    $hex = strtok($_GET['c128-addr-drv'], '-');
    $drv = strtok('-');
    $ext = '-' . $dsk . '.d71';
    $ofs = 0x15002;
    break;
case 'atari':
    $hex = strtok($_GET['atari-addr-drv'], '-');
    $drv = strtok('-');
    $ext = '-' . $dsk . '.atr';
    $blk = array(0, 5, 4, 4, 275);
    $ofs = 0x00010 + ($blk[$dsk] - 1) * 0x80;
    break;
}

$addr = hexdec($hex);
$cfg .= chr($addr % 0x100).chr($addr / 0x100);
$cfg .= $drv;

if ($dsk) {
    $img = file_get_contents('contiki'. $ext);
    if ($img)
      $out = substr_replace($img, $cfg, $ofs, strlen($cfg));
    else
      $out = '';
} else {
    $ext = '.cfg';
    $out = $cfg;
}

header('Content-Type: application/octetstream');
header('Content-Disposition: attachment; filename=contiki' . $ext);
print($out);

?>
