// log.h; the header file which defines Log(); and LogErr();
 
 
extern bool LogCreated;      // keeps track whether the log file is created or not
 
void Log (char *message);    // logs a message to LOGFILE
void LogErr (char *message); // logs a message; execution is interrupted
