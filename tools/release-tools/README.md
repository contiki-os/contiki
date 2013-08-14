Release tools
=============

The tools/release-tools directory contains files for building and testing
Contiki releases. To compile a release of Contiki, run

  make RELEASE=2.6

for Contiki 2.6. It is also possible to compile a release of a specific git tag
or other non-numbered version, such as the current git HEAD, like this:

  make TAG=HEAD

The compile-examples and compile-platform directories are used for running
nightly builds of the git version of Contiki.
