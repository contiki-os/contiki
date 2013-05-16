ELFloader and shell command 'exec' example for Sky platform
-----------------------------------------------------------

Compiles the Contiki hello-world application as a Contiki executable (.ce).
The Contiki executable is then uploaded to the Sky platform via serial, and is
stored in the filesystem.  Finally, the executable is loaded via the shell
command 'exec'.

NOTE: You may have to reduce the ELF loader memory usage
(/platform/sky/contiki-conf.h).  Since hello-world uses very little memory:

#define ELFLOADER_CONF_DATAMEMORY_SIZE 0x100
#define ELFLOADER_CONF_TEXTMEMORY_SIZE 0x100

1. Upload Sky shell with 'exec' command and symbols (requires several
   recompilations to generate correct symbols):

    make sky-shell-exec.sky
    make sky-shell-exec.sky CORE=sky-shell-exec.sky
    make sky-shell-exec.upload CORE=sky-shell-exec.sky

2. Verify access to the shell and the filesystem:

    > make login
    SHELL> echo hello shell
    SHELL> echo test | write mytest.txt
    SHELL> ls
    SHELL> read mytest.txt
    [CTRL-C] to exit the shell

3. Upload Contiki executable hello-world.ce:

    > make upload-executable
    [CTRL-C] to exit the shell when the entire file has been uploaded (after
    ~30 sec)

4. Verify that hello-world.ce exists in CFS:

    > make login
    SHELL> ls
    SHELL> read hello-world.ce | size

The last command output should equal the size of hello-world.ce in this
directory!

5. Load and start hello world:

    SHELL> exec hello-world.ce

The program should now start: the output 'Hello, World' appears.
