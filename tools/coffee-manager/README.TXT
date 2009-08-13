se.sics.coffee is a library for the Coffee File System. It is able to 
generate file system images and to extract information from existing 
images.

Building:

./build.sh

Usage:

java -jar coffee.jar [-p <platform>] [-i|e|r <file>] [-l|s] <file system image>

Options:

-p   Selects the platform configuration of Coffee to use.
     Valid choices: sky (default), esb.
-i   Inserts a new file into the file system.
-e   Extracts a file from the file system and saves it locally.
-r   Removes a file from the file system.
-l   Lists all files.
-s   Prints file system statistics.

Author:

Nicolas Tsiftes <nvt@sics.se>
