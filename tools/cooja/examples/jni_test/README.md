_The JNI tests have been replaced by the Cooja configuration wizard._

The JNI tests assisted in configuring Cooja for compiling and linking Java
Native Interface (JNI) enabled Contiki libraries. Such Contiki libraries are
used by Cooja's Contiki Motes: motes simulated at the operating system
abstraction level.

The new configuration wizard is started from inside Cooja, and exercises the
same functionality as the JNI tests. In contrast, the wizard is directly
connected to the current Cooja configuration, removing the need to migrate the
configuration between JNI tests and Cooja.

To start the wizard:

    cd tools/cooja
    ant run # Start COOJA

Menu > Settings > Compiler configuration wizard

-- Fredrik Osterlind, fros@sics.se, March 2009
