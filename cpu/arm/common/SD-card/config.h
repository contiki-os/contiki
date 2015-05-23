#ifndef EFSL_CONFIG_H_
#define EFSL_CONFIG_H_

/* Close enough */
#define HW_ENDPOINT_LPC2000_SD

/* Memory configuration
   --------------------
 
 * Here you must configure wheter your processor can access memory byte
 * oriented. All x86 processors can do it, AVR's can do it to. Some DSP
 * or other microcontrollers can't. If you have an 8 bit system you're safe.
 * If you are really unsure, leave the setting commented out, it will be slower
 * but it will work for sure.
*/

	/* disabled for ARM (mt): #define BYTE_ALIGNMENT */

/* Cache configuration
   -------------------
   
 * Here you must configure how much memory of cache you can/want to use.
 * The number you put at IOMAN_NUMBUFFER is multiplied by 512. So 1 means 
 * 512 bytes cache, 4 means 2048 bytes cache. More is better.
 * The number after IOMAN_NUMITERATIONS should be untouched.
 * The last field (IOMAN_DO_MEMALLOC) is to tell ioman to allocate it's
 * own memory in it's structure, or not. If you choose to do it yourself
 * you will have to pass a pointer to the memory as the last argument of
 * ioman_init.
*/
	/*#define IOMAN_NUMBUFFER 1*/
	#define IOMAN_NUMBUFFER 6
	#define IOMAN_NUMITERATIONS 3
	#define IOMAN_DO_MEMALLOC

/* Cluster pre-allocation 
   ----------------------
   
 * When writing files, the function that performs the actual write has to
 * calculate how many clusters it will need for that request. It then allocates
 * that number of new clusters to the file. Since this involves some calculations
 * and writing of the FAT, you might find it beneficial to limit the number of
 * allocations, and allow fwrite to pre-allocate a number of clusters extra.
 * This setting determines how many clusters will be extra allocated whenever
 * this is required.
 * Take in carefull consideration how large your clustersize is, putting 10 here
 * with a clustersize of 32kb means you might waste 320 kb.
 * The first option is for preallocating files, the other is used when enlarging
 * a directory to accomodate more files
*/
      /*#define CLUSTER_PREALLOC_FILE 0*/
	  #define CLUSTER_PREALLOC_FILE 2
      #define CLUSTER_PREALLOC_DIRECTORY 0


/* Endianess configuration
   -----------------------
   
 * Here you can configure wheter your architecture is little or big endian. This
 * is important since all FAT structures are stored in intel little endian order.
 * So if you have a big endian system the library has to convert all figures to
 * big endian in order to work.
 */
	#define LITTLE_ENDIAN
	

/* Date and Time support
   ---------------------
   
 * Here you can enable or disable date and time support. If you enable
 * it you will have to create 6 functions, that are described in the
 * EFSL manual. If the functions are not present when linking your
 * program with the library you will get unresolved dependencies.
 */
	/*#define DATE_TIME_SUPPORT*/
		
/* Error reporting support
   -----------------------
   
 * When you receive an error in userland, it usually only gives limited
 * information (most likely, fail or success). If error detection and
 * reporting is important for you, you can enable more detailed error
 * reporting here. This is optional, the costs are 1 byte per object,
 * and a small increase in code size.
 * You can enable error recording for all object, or you can select the
 * object manually.
 * For full error reporting use FULL_ERROR_SUPPORT
 * For only the base-core of the library use BASE_ERROR_SUPPORT
 * For IO/Man use ERRSUP_IOMAN
 * For Disc   use ERRSUP_IOMAN
 * For Part   use ERRSUP_PARTITION
 * For Fs     use ERRSUP_FILESYSTEM
 * For File   use ERRSUP_FILE
*/

	#define FULL_ERROR_SUPPORT
	/*#define BASE_ERROR_SUPPORT*/

/* List options 
   ------------
   
 * In this section youcan configure what kind of data you will get from
 * directory listing requests. Please refer to the documentation for
 * more information
*/

#define LIST_MAXLENFILENAME 12




/* Debugging configuration
   -----------------------
   
 * Here you can configure the debugging behaviour. Debugging is different
 * on every platform (see debug.h for more information). 
 * If your hardware has no means of output (printf) dont define any anything, 
 * and nothing will happen. For real world use debugging should be turned off.
*/

/* 	#define DEBUG */

#define lpc2000_debug_printf dbg_blocking_printf

#endif
