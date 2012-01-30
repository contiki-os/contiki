#!/usr/bin/perl
#Generate a .c source that preinitializes a file system
#David Kopf <dak664@embarqmail.com> July 2009
#Extended from the existing non-coffee tool

#The simplest format is used with httpd-fs.c. It contains a linked
#list pointing to file names and file contents.

#An extension uses the same linked list that points to a the file names
#and contents within a coffee file system having fixed file sizes.
#This allows the content to be rewritten by coffee while still being
#readable by httpd-fs.c New files or sector extension of existing files
#is not possible, but a larger initial sector allocation could be given
#to some files to allow a limited size increase. Since this would leave the
#wrong file size in the linked list, httpd-fs would have to terminate on
#a reverse zero scan like coffee does. Rewriting the linked list would
#probably be OK if in eeprom, but not if in program flash. Suggestions
#for a workaround would be welcome!

#Lastly a full coffee file system can be preinitialized. File reads must
#then be done using coffee.

#Assumes the coffee file_header structure is
#struct file_header {
# coffee_page_t log_page;
# uint16_t log_records;
# uint16_t log_record_size;
# coffee_page_t max_pages;
# uint8_t deprecated_eof_hint;
# uint8_t flags=3 for the initial value;
# char name[COFFEE_NAME_LENGTH];
# } __attribute__((packed));

goto DEFAULTS;
START:$version="1.1";

#Process options
for($n=0;$n<=$#ARGV;$n++) {
  $arg=$ARGV[$n];
  if      ($arg eq "-v") {
    print "makefsdata Version $version\n";
  } elsif ($arg eq "-A") {
    $n++;$attribute=$ARGV[$n];
  } elsif ($arg eq "-C") {
    $coffee=1;
  } elsif ($arg eq "-c") {
    $complement=1;
  } elsif ($arg eq "-i") {
    $n++;$includefile=$ARGV[$n];
# } elsif ($arg eq "-p") {
#   $n++;$coffee_page_length=$ARGV[$n];
  } elsif ($arg eq "-s") {
    $n++;$coffee_sector_size=$ARGV[$n];
  } elsif ($arg eq "-t") {
    $n++;$coffee_page_t     =$ARGV[$n];
  } elsif ($arg eq "-f") {
    $n++;$coffee_name_length=$ARGV[$n];
   } elsif ($arg eq "-S") {
    $n++;$sectionname=$ARGV[$n];
  } elsif ($arg eq "-l") {
    $linkedlist=1;
  } elsif ($arg eq "-d") {
    $n++;$directory=$ARGV[$n];
  } elsif ($arg eq "-o") {
    $n++;$outputfile=$ARGV[$n];
    $coffeefile=$outputfile;
  } else {
DEFAULTS:
#Set up defaults
$coffee=0;
#$coffee_page_length=256;
$coffee_sector_size=256;
$coffee_page_t=1;
$coffee_name_length=16;
$complement=0;
$directory="";
$outputfile="httpd-fsdata.c";
$coffeefile="httpd-coffeedata.c";
$includefile="makefsdata.h";
$linkedlist=0;
$attribute="";
$sectionname=".coffeefiles";
if (!$version) {goto START;}
    print "\n";
    print "Usage: makefsdata <option(s)> <-d input_directory> <-o output_file>\n\n";
    print " Generates c source file to pre-initialize a contiki file system.\n";
    print " The default output is a simple linked list readable by httpd-fs.c\n";
    print " and written to $outputfile.\n\n";
    print " The -C option makes a coffee file system to default output $coffeefile.\n";
    print " A linked list can be still be generated for use with httpd-fs.c so long\n";
    print " as coffee does not extend, delete, or add any files.\n\n";
    print " The input directory structure is copied. If input_directory is specified\n";
    print " it becomes the root \"/\". If no input_directory is specified the first\n";
    print " subdirectory found in the current directory is used as the root.\n\n";
    print " WARNING : If the output file exists it will be overwritten without confirmation!\n\n";
    print " Options are:\n";
    print " -v               Display the version number\n";
    print " -A attribute     Append \"attribute\" to the declaration, e.g. PROGMEM to put data in AVR program flash memory\n";
    print " -C               Use coffee file system format\n";
    print " -c               Complement the data, useful for obscurity or fast page erases for coffee\n";
    print " -i filename      Treat any input files with name \"filename\" as include files.\n";
    print "                  Useful for giving a server a name and ip address associated with the web content.\n";
    print "                  The default is $includefile.\n\n";
    print "   The following apply only to coffee file system\n";
#   print " -p pagesize      Page size in bytes (default $coffee_page_length)\n";
    print " -s sectorsize    Sector size in bytes (default $coffee_sector_size)\n";
    print " -t page_t        Number of bytes in coffee_page_t (1,2,or 4, default $coffee_page_t)\n";
    print " -f namesize      File name field size in bytes (default $coffee_name_length)\n";
    print " -S section       Section name for data (default $sectionname)\n";
    print " -l               Append a linked list for use with httpd-fs\n";
    exit;
  }
}

#--------------------Configure parameters-----------------------
if ($coffee) {
  $outputfile=$coffeefile;
  $coffee_header_length=2*$coffee_page_t+$coffee_name_length+6;
  if ($coffee_page_t==1) {
    $coffeemax=0xff;
  } elsif ($coffee_page_t==2) {
    $coffeemax=0xffff;
  } elsif ($coffee_page_t==4) {
    $coffeemax=0xffffffff;
  } else {
   die "Unsupported coffee_page_t $coffee_page_t\n";
  }
} else {
# $coffee_page_length=1;
  $coffee_sector_size=1;
  $linkedlist=1;
  $coffee_name_length=256;
  $coffee_max=0xffffffff;
  $coffee_header_length=0;
}
$null="0x00";if ($complement) {$null="0xff";}
$tab="  ";  #optional tabs or spaces at beginning of line, e.g. "\t\t"

#--------------------Create output file-------------------------
#awkward but could not figure out how to compare paths later unless the file exists -- dak
if (!open(OUTPUT, "> $outputfile")) {die "Aborted: Could not create output file $outputfile";}
print(OUTPUT "\n");
close($outputfile);
use Cwd qw(abs_path);
if (!open(OUTPUT, "> $outputfile")) {die "Aborted: Could not create output file $outputfile";}
$outputfile=abs_path($outputfile);

#--------------------Get a list of input files------------------
if ($directory eq "") {
  opendir(DIR, ".");
  @files =  grep { !/^\./ && !/(CVS|~)/ } readdir(DIR);
  closedir(DIR);
  foreach $file (@files) {
    if(-d $file && $file !~ /^\./) {
      $directory=$file;
      break;
    }
  }
}
if ($directory eq "") {die "Aborted: No subdirectory in current directory";}
if (!chdir("$directory")) {die "Aborted: Directory \"$directory\" does not exist!";}

if ($coffee) {
  print "Processing directory $directory as root of coffee file system\n";
} else {
  print "Processing directory $directory as root of packed httpd-fs file system\n";
}
opendir(DIR, ".");
@files =  grep { !/^\./ && !/(CVS|~)/ && !/(makefsdata.ignore)/ } readdir(DIR);
closedir(DIR);

foreach $file (@files) {
  if(-d $file && $file !~ /^\./) {
    print "Adding subdirectory $file\n";
    opendir(DIR, $file);
    @newfiles =  grep { !/^\./ && !/(CVS|~)/ } readdir(DIR);
    closedir(DIR);
#   printf "Adding files @newfiles\n";
    @files = (@files, map { $_ = "$file/$_" } @newfiles);
    next;
  }
}
#--------------------Write the output file-------------------
print "Writing to $outputfile\n";
($DAY, $MONTH, $YEAR) = (localtime)[3,4,5];
printf(OUTPUT "/*********Generated by contiki/tools/makefsdata on %04d-%02d-%02d*********/\n", $YEAR+1900, $MONTH+1, $DAY);
if ($coffee) {
  print(OUTPUT "/*For coffee filesystem of sector size $coffee_sector_size and header length $coffee_header_length bytes*/\n");
}
print(OUTPUT "\n");
#--------------------Process include file-------------------
foreach $file (@files) {if ($file eq $includefile) {
  open(FILE, $file) || die "Aborted: Could not open include file $file\n";
  print "Including text from $file\n";
  $file_length= -s FILE;
  read(FILE, $data, $file_length);
  print OUTPUT ($data); 
  close(FILE);
  next;        #include all include file matches
# break;       #include only first include file match
}}

#--------------------Process data files-------------------
$n=0;$coffeesize=0;$coffeesectors=0;
foreach $file (@files) {if(-f $file) {
  if (length($file)>=($coffee_name_length-1)) {die "Aborted: File name $file is too long";}
  if (abs_path("$file") eq abs_path("$outputfile")) {
    print "Skipping output file $outputfile - recursive input NOT allowed\n";
    next;
  }
  if ($file eq $includefile) {next;}  
  open(FILE, $file) || die "Aborted: Could not open file $file\n";
  print "Adding /$file\n";
  if (grep /.png/||/.jpg/||/jpeg/||/.pdf/||/.gif/||/.bin/||/.zip/,$file) {binmode FILE;} 

  $file_length= -s FILE;
  $file =~ s-^-/-;
  $fvar = $file;
  $fvar =~ s-/-_-g;
  $fvar =~ s-\.-_-g;

  if ($coffee) {
    $coffee_sectors=int(($coffee_header_length+$file_length+$coffee_sector_size-1)/$coffee_sector_size);
#   $coffee_sectors=sprintf("%.0f",($coffee_header_length+$file_length+$coffee_sector_size-1)/$coffee_sector_size)-1;
    $coffee_length=$coffee_sectors*$coffee_sector_size;
  } else {
    $coffee_length=$file_length+length($file)+1;
  }
  $flen[$n]=$file_length;
  $clen[$n]=$coffee_length;
  $n++;$coffeesectors+=$coffee_sectors;$coffeesize+=$coffee_length;
  if ($coffee) {
    if ($coffeesectors>$coffeemax) {
      print "Warning: sector number $coffeesectors overflows allocated sector size in coffee header\n";
    }
    print(OUTPUT "\n__attribute__ ((section (\"$sectionname\")))\n");
    print(OUTPUT "volatile const char data".$fvar."[$coffee_length] = {\n");
  } else {
    print(OUTPUT "\nconst char data".$fvar."[$coffee_length] $attribute = {\n");
  }
  print(OUTPUT "$tab/* $file */\n$tab");
#--------------------Header-----------------------------
#log_page
  if ($coffee) {
    print (OUTPUT " ");
    for($j=0;$j<$coffee_page_t;$j++) {print (OUTPUT "$ null ,");}
#log_records, log_record_size
    for($j=0;$j<4;$j++) {print (OUTPUT "$null, ");}
#max_pages 
    if ($complement) {$coffee_sectors=$coffee_sectors^0xffffffff;}
    if ($coffee_page_t==1) {
      printf(OUTPUT "0x%2.2x, ",($coffee_sectors    )&0xff);
    } elsif ($coffee_page_t==2) {
      printf(OUTPUT "0x%2.2x, ",($coffee_sectors>> 8)&0xff);
      printf(OUTPUT "0x%2.2x, ",($coffee_sectors    )&0xff);
    } elsif ($coffee_page_t==4) {
      printf(OUTPUT "0x%2.2x, ",($coffee_sectors>>24)&0xff);
      printf(OUTPUT "0x%2.2x, ",($coffee_sectors>>16)&0xff);
      printf(OUTPUT "0x%2.2x, ",($coffee_sectors>> 8)&0xff);
      printf(OUTPUT "0x%2.2x, ",($coffee_sectors    )&0xff);
    }
    if ($complement) {$coffee_sectors=$coffee_sectors^0xffffffff;}
#eof hint and flags
    if ($complement) {
      print(OUTPUT "0xff, 0xfc,\n$tab");
    } else {
      print(OUTPUT "0x00, 0x03,\n$tab");
    }
  }

#-------------------File name--------------------------
  for($j = 0; $j < length($file); $j++) {
    $temp=unpack("C", substr($file, $j, 1));
    if ($complement) {$temp=$temp^0xff;}
    printf(OUTPUT " %#02.2x,", $temp);
  }
  if ($coffee) {
    for(; $j < $coffee_name_length-1; $j++) {printf(OUTPUT " $null,");}
    {print(OUTPUT " $null");}
  } else {
    {printf(OUTPUT " $null");}
  }
#------------------File Data---------------------------
  $coffee_length-=$coffee_header_length;
  $i = 10;        
  while(read(FILE, $data, 1)) { 
    $temp=unpack("C", $data);   
    if ($complement) {$temp=$temp^0xff;}
    if($i == 10) {
      printf(OUTPUT ",\n$tab 0x%2.2x", $temp);
      $i = 0;
    } else {
      printf(OUTPUT ", 0x%2.2x", $temp)
    }
    $i++;$coffee_length--;
  }

  if ($coffee) {
    print (OUTPUT ",");
    while (--$coffee_length) {
      if($i==9) {
        print(OUTPUT " $null,\n$tab");
        $i = 0;
      } else {
        print (OUTPUT " $null,");
        $i++;
      }
    }
    print (OUTPUT " $null");
  }
  print (OUTPUT "};\n");
  close(FILE);
  push(@fvars, $fvar);
  push(@pfiles, $file);
}}

if ($linkedlist) {
#-------------------httpd_fsdata_file links-------------------
#The non-coffee PROGMEM flash file system for the Raven webserver uses a linked flash list as follows:
print(OUTPUT "\n\n/* Structure of linked list (all offsets relative to start of section):\n");
print(OUTPUT "struct httpd_fsdata_file {\n");
print(OUTPUT "$tab const struct httpd_fsdata_file *next; //actual flash address of next link\n");
print(OUTPUT "$tab const char *name;                     //offset to coffee file name\n");
print(OUTPUT "$tab const char *data;                     //offset to coffee file data\n");
print(OUTPUT "$tab const int len;                        //length of file data\n");
print(OUTPUT "#if HTTPD_FS_STATISTICS == 1               //not enabled since list is in PROGMEM\n");
print(OUTPUT "$tab uint16_t count;                       //storage for file statistics\n");
print(OUTPUT "#endif\n");
print(OUTPUT "}\n*/\n");

# For the static httpd-fs.c file system the file name and data addresses in the linked list
# point to the actual memory locations.
# For the coffee file system the addresses start from zero. The starting address must be added
# in the coffee read routine.

for($i = 0; $i < @fvars; $i++) {
  $file = $pfiles[$i];
  $fvar = $fvars[$i];
  if($i == 0) {
      $prevfile = "NULL";
      $data_offset=0;
  } else {
      $data_offset=$data_offset+$clen[$i-1];
      $prevfile = "file" . $fvars[$i - 1];
  }
  $filename_offset=$data_offset+6+2*$coffee_page_t;
  $coffee_offset=$data_offset+$coffee_header_length;
  if ($coffee_offset>0xffff) {print "Warning : Linked list offset field overflow\n";}
  print(OUTPUT "const struct httpd_fsdata_file");
  for ($t=length($file);$t<16;$t++) {print(OUTPUT " ")};
  print(OUTPUT " file".$fvar."[] ");
  if ($attribute) {print(OUTPUT "$attribute ");}
  print(OUTPUT "={{");
  for ($t=length($prevfile);$t<20;$t++) {print(OUTPUT " ")};
  print(OUTPUT "$prevfile, ");
  if ($coffee) {
    printf(OUTPUT "(const char *)0x%4.4x, ",$filename_offset);
    printf(OUTPUT "(const char *)0x%4.4x, ",$coffee_offset);
    printf(OUTPUT "%5u}};\n",$flen[$i]);
  } else {
    print(OUTPUT "data$fvar");
    for ($t=length($file);$t<15;$t++) {print(OUTPUT " ")};
    print(OUTPUT ", data$fvar");
    for ($t=length($file);$t<15;$t++) {print(OUTPUT " ")};
    print(OUTPUT " +".(length($file)+1).", sizeof(data$fvar)");
    for ($t=length($file);$t<16;$t++) {print(OUTPUT " ")};
    print(OUTPUT " -".(length($file)+1)."}};\n");
  }
}
print(OUTPUT "\n#define HTTPD_FS_ROOT  file$fvars[$n-1]\n");
print(OUTPUT "#define HTTPD_FS_NUMFILES  $n\n");
print(OUTPUT "#define HTTPD_FS_SIZE $coffeesize\n");
}
print "All done, files occupy $coffeesize bytes\n";

