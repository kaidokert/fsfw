FROM ubuntu:focal

RUN apt-get update
RUN apt-get --yes upgrade

#tzdata is a dependency, won't install otherwise
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get --yes install gcc g++ cmake make lcov git valgrind nano iputils-ping python3 pip doxygen graphviz

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
