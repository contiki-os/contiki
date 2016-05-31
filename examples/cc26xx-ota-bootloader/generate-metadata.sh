var fs = require('fs');

var firmware_bin_file = process.argv[2];
var firmware_version = parseInt( process.argv[3] );
var firmware_uuid = parseInt( process.argv[4] );

var firmware_binary = fs.readFileSync( firmware_bin_file );

output_file = new Buffer( 256 + firmware_binary.length );

//  Write OTA metadata
output_file.writeUInt16LE(0x0, 0); // crc
output_file.writeUInt16LE(0x0, 2); // crc shadow
output_file.writeUInt32LE(firmware_binary.length, 4); // binary size
output_file.writeUInt16LE(firmware_version, 8); // version
output_file.writeUInt32LE(firmware_uuid, 10); // uuid

//  Fill remaining space until start of firmware binary
for (var j=14; j<256; j++) {
  output_file.writeUInt8(0xff, j);
}

//  Write firmware data
firmware_binary.copy( output_file, 256 );

fs.writeFileSync("firmware-with-metadata.bin", output_file);
