multiRun.py - run an exported cooja simulation several times
============================================================

Cooja can export a complete simulation setup to a jar file. This can be
executed independently from the contiki / cooja environment.
This python script allows to execute one simulation file
several times with different seeds. This may be useful for example for
montecarlo evaluations of a simulation setup.

Usage
=====

usage: multiRun.py [-h] [--max_threads MAX_THREADS] [--runs N] [--break]
                   [--execute <program name>]
                   progname

* -h prints a help message
* --max_threads sets the maximum number of threads. This should not be larger
    then the number of processor cores in your system. Default value: 4
* --runs sets the number of simulation runs. Default value: 10
* --break Break simulation if executed jar file exited with an error code
* --execute Execute the given command when simulation finished
* progname the jar archive to be executed


When executed, the app creates a directory with the current date and time.
Inside this, a subdirectory for each simulation run is created Inside these
directories, the simulations are executed and the results are stored inside
each simulation's folder.

Evaluation
==========

The results can easily be evaluated using python. For example os.path.walk
could be used to walk through the several simulation run directories. Each
simulation log can be processed using regular expressions:

    RE_STRING = r'^(?P<node_id>\d+)@(?P<timestamp>\d+): (?P<bytes>\d+)$'
    re_compiled = re.compile(RE_STRING)

    for line in lines:
        l = re_compiled.search(line)
        if l:
            print "timestamp:", l.group('timestamp')

CHANGELOG
=========
0.1     Initial check-in
0.2     - Keep individual jar files just while the thread is working to avoid
            overusing disk space.
        - Small fix: Handle an absolute path to the .jar file correctly

Author
======

Jens Dede (C) 2014, <jd@comnets.uni-bremen.de>
