#!/bin/sh
(cd se/sics/coffee && javac *.java) && \
jar cvfm coffee.jar MANIFEST.MF se/sics/coffee/*.class *.properties
