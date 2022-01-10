FROM ubuntu:focal

RUN apt-get update
RUN apt-get --yes upgrade

#tzdata is a dependency, won't install otherwise
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get --yes install gcc g++ cmake make lcov git valgrind nano
