FROM ubuntu:focal

RUN apt-get update
RUN apt-get --yes upgrade

#tzdata is a dependency, won't install otherwise
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get --yes install gcc g++ cmake make lcov git valgrind nano iputils-ping python3 pip doxygen graphviz rsync

RUN python3 -m pip install sphinx breathe

RUN git clone https://github.com/catchorg/Catch2.git && \
        cd Catch2 && \
        git checkout v3.1.0 && \
        cmake -Bbuild -H. -DBUILD_TESTING=OFF && \
        cmake --build build/ --target install

RUN git clone https://github.com/ETLCPP/etl.git && \
        cd etl && \
        git checkout 20.28.0 && \
        cmake -B build . && \
        cmake --install build/

#ssh needs a valid user to work
RUN adduser --uid 114 jenkins

#add documentation server to known hosts
RUN echo "|1|/LzCV4BuTmTb2wKnD146l9fTKgQ=|NJJtVjvWbtRt8OYqFgcYRnMQyVw= ecdsa-sha2-nistp256 AAAAE2VjZHNhLXNoYTItbmlzdHAyNTYAAAAIbmlzdHAyNTYAAABBBNL8ssTonYtgiR/6RRlSIK9WU1ywOcJmxFTLcEblAwH7oifZzmYq3XRfwXrgfMpylEfMFYfCU8JRqtmi19xc21A=" >> /etc/ssh/ssh_known_hosts
RUN echo "|1|CcBvBc3EG03G+XM5rqRHs6gK/Gg=|oGeJQ+1I8NGI2THIkJsW92DpTzs= ecdsa-sha2-nistp256 AAAAE2VjZHNhLXNoYTItbmlzdHAyNTYAAAAIbmlzdHAyNTYAAABBBNL8ssTonYtgiR/6RRlSIK9WU1ywOcJmxFTLcEblAwH7oifZzmYq3XRfwXrgfMpylEfMFYfCU8JRqtmi19xc21A=" >> /etc/ssh/ssh_known_hosts