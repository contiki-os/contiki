#!/bin/sh
(cd org/contikios/coffee && javac *.java) && \
jar cvfm coffee.jar MANIFEST.MF org/contikios/coffee/*.class *.properties
