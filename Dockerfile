FROM ubuntu:14.04

MAINTAINER Darragh Grealish "darragh@monetas.net"

WORKDIR /home/otbuilder/

#install the following dependencies;
RUN set +x; \
		apt-get update \
		&& apt-get install -y --no-install-recommends software-properties-common \
		&& add-apt-repository ppa:hamrle/ppa \
		&& apt-get update \
		&& apt-get install -y --no-install-recommends g++ make cmake libssl-dev protobuf-compiler libprotobuf-dev libzmq3-dev git python3-dev swig3.0 clang-format-3.5 wget \
		&& ln -s /usr/bin/swig3.0 /usr/bin/swig \
		&& apt-get autoremove
ENV DEBIAN_FRONTEND noninteractive
RUN set +x; \
		dpkg-reconfigure locales \
		&& locale-gen C.UTF-8 \
		&& update-locale LANG=C.UTF-8 || true 
#ENV LC_ALL C.UTF-8

RUN set +x; \
		cd /tmp \
		&& wget http://skylink.dl.sourceforge.net/project/cppcheck/cppcheck/1.67/cppcheck-1.67.tar.bz2 \
		&& tar -xf cppcheck-1.67.tar.bz2 \
		&& cd cppcheck-1.67 \
		&& make -j 4 -l 2 \
		&& make install \
		&& /usr/bin/cppcheck --version

# setup a non-root user
RUN useradd -ms /bin/bash otuser
# become otuser and create build folder but check if it already exists 
USER otuser
ENV HOME /home/otuser
WORKDIR /home/otuser

RUN set +x; \
		cd ~/ \
		&& mkdir opentxs-source \
		&& cd opentxs-source \
		&& mkdir build

ADD CMakeLists.txt .clang-format .gitignore .gitmodules /home/otuser/opentxs-source/
ADD cmake /home/otuser/opentxs-source/cmake
ADD deps /home/otuser/opentxs-source/deps
ADD include /home/otuser/opentxs-source/include
ADD scripts /home/otuser/opentxs-source/scripts
ADD src /home/otuser/opentxs-source/src
ADD tests /home/otuser/opentxs-source/tests
ADD wrappers /home/otuser/opentxs-source/wrappers
ADD .git /home/otuser/opentxs-source/.git
USER root
RUN chown -R otuser:otuser /home/otuser/
USER otuser
RUN set +x; \
		cd /home/otuser/opentxs-source/build \
		&& cmake .. \             
	        -DPYTHON=1 \
	        -DSIGNAL_HANLDER=ON \
	        -DPYTHON_EXECUTABLE=/usr/bin/python3 \
	        -DPYTHON_LIBRARY=/usr/lib/x86_64-linux-gnu/libpython3.4m.so \
	        -DPYTHON_INCLUDE_DIR=/usr/include/python3.4 \
		&& make -j 4 -l 2 
USER root
RUN set +x; \
		cd /home/otuser/opentxs-source/build \
		&& make install \
		&& ldconfig
# we can install sample data here. or pipe it through when running the image e.g docker run <image> << bash script
USER root
CMD opentxs

