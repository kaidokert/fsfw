FROM ubuntu:focal

RUN apt-get update
RUN apt-get --yes upgrade

#tzdata is a dependency, won't install otherwise
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get --yes install gcc g++ cmake make lcov git valgrind nano iputils-ping

RUN git clone https://github.com/catchorg/Catch2.git && \
	cd Catch2 && \
	git checkout v3.0.0-preview5 && \
	cmake -Bbuild -H. -DBUILD_TESTING=OFF && \
	cmake --build build/ --target install

RUN git clone https://github.com/ETLCPP/etl.git && \
	cd etl && \
	git checkout 20.28.0 && \
	cmake -B build . && \
	cmake --install build/
