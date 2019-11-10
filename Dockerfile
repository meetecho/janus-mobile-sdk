from ubuntu:18.04

run apt-get update && apt-get install -y build-essential cmake git inotify-tools valgrind python3-pip openjdk-8-jdk curl uuid-dev libtool autotools-dev golang doxygen python
run pip3 install gdbgui

env SHELL=/bin/bash

cmd /bin/bash
