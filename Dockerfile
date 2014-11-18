FROM ubuntu:14.04

MAINTAINER Darragh Grealish "darragh@monetas.net"

WORKDIR /home/otbuilder/

#install the following dependencies;
RUN set +x; \
		apt-get update \
		&& apt-get install -y build-essential cmake pkg-config libssl-dev protobuf-compiler libprotobuf-dev g++ gdc libzmq3-dev libzmq3 --no-install-recommends \
		&& apt-get install -y git wget curl libpcre3-dev python3 python3-pip python3-dev openjdk-6-jdk openjdk-6-source --no-install-recommends ruby-dev \
		&& apt-get autoremove
ENV DEBIAN_FRONTEND noninteractive
RUN set +x; \
		dpkg-reconfigure locales \
		&& locale-gen C.UTF-8 \
		&& update-locale LANG=C.UTF-8 || true 
#ENV LC_ALL C.UTF-8

RUN set +x; \
		apt-get install -y cppcheck ccache clang-format-3.5 

RUN useradd -ms /bin/bash otuser
# install SWIG
RUN set +x; \
		cd /tmp/ \
		&& mkdir tools \
		&& cd tools \
		&& wget http://archive.ubuntu.com/ubuntu/pool/universe/s/swig/swig_3.0.2.orig.tar.gz \
		&& wget http://archive.ubuntu.com/ubuntu/pool/universe/s/swig/swig_3.0.2-1ubuntu1.debian.tar.xz \
		&& tar -xzf swig_3.0.2.orig.tar.gz \
		&& xzcat swig_3.0.2-1ubuntu1.debian.tar.xz | tar x -C swig-3.0.2/ \
	        && cd swig-3.0.2/ \
	        && ./configure --prefix=/usr \
	        && make \
	        && make install \
	        && install -v -m755 -d /usr/share/doc/swig-3.0.2 \
	        && cp -v -R Doc/* /usr/share/doc/swig-3.0.2

# become otuser and create build folder but check if it already exists 
USER otuser
ENV HOME /home/otuser
WORKDIR /home/otuser

RUN set +x; \
		cd ~/ \
		&& mkdir opentxs-source \
		&& cd opentxs-source \ 
		&& rm -rf build || true \
		&& mkdir build || true
		
ADD . /home/otuser/opentxs-source/
USER root
RUN chown -R otuser:otuser /home/otuser/
USER otuser
RUN set +x; \
		cd /home/otuser/opentxs-source/build \
		&& cmake .. \             
		--debug-output  
	        -DPYTHON=1 \
	        -DSIGNAL_HANLDER=ON \
	        -DPYTHON_EXECUTABLE=/usr/bin/python3 \
	        -DPYTHON_LIBRARY=/usr/lib/x86_64-linux-gnu/libpython3.4m.so \
	        -DPYTHON_INCLUDE_DIR=/usr/include/python3
		&& make -j 4 -l 2 
USER root
RUN set +x; \
		cd /home/otuser/opentxs-source/build \
		&& make install \
		&& ldconfig
# we can install sample data here. or pipe it through when running the image e.g docker run <image> << bash script
CMD opentxs

