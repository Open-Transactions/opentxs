ARG BASE_FEDORA_VERSION="39"
ARG ANDROID_BUILD_TOOLS=33.0.0
ARG ANDROID_LEVEL=33
ARG ANDROID_LEVEL_TOOLCHAIN=33
ARG ANDROID_TOOLS=8092744
ARG BOOST_FOR_ANDROID_COMMIT_HASH=64a5ba21b3ce0ca192d0ea0fbc8e21854f0ffcc7
ARG CMAKE_BUILD_TYPE=Release
ARG CMAKE_VERSION="3.28.1"
ARG GTEST=f8d7d77c06936315286eb55f8de22cd23c188571
ARG JAVA_HOME_ARG=/usr/lib/jvm/java
ARG JOBS=2
ARG LIBZMQ_COMMIT_HASH=4097855ddaaa65ed7b5e8cb86d143842a594eebd
ARG LMDB_COMMIT_HASH=8d0cbbc936091eb85972501a9b31a8f86d4c51a7
ARG NDK_VERSION=26.0.10792818
ARG OPENSSL_VERSION=3.1.4
ARG OTCOMMON_VERSION="2.0.0"
ARG PROTOBUF_COMMIT_HASH=2c5fa078d8e86e5f4bd34e6f4c9ea9e8d7d4d44a
ARG QT_BRANCH=6.6
ARG QT_RELEASE=official_releases
ARG QT_VERSION=6.6.1
ARG SODIUM_COMMIT_HASH=940ef42797baa0278df6b7fd9e67c7590f87744b

FROM fedora:${BASE_FEDORA_VERSION} AS run
ARG BASE_FEDORA_VERSION
RUN --mount=type=tmpfs,target=/var/cache/dnf dnf install -y \
    git \
    ninja-build \
    pcre2-devel

FROM alpine AS download
RUN mkdir -p /usr/src
RUN --mount=type=tmpfs,target=/var/cache/apk apk add \
    bzip2 \
    git \
    lzo \
    p7zip \
    rsync \
    wget \
    xz

FROM run AS tools
RUN --mount=type=tmpfs,target=/var/cache/dnf dnf install -y \
    automake \
    bzip2 \
    diffutils \
    findutils \
    gcc \
    gcc-c++ \
    java-devel \
    libtool \
    make \
    patch \
    perl-core \
    python \
    rsync \
    unzip \
    wget \
    which \
    xz

FROM tools AS android
ARG ANDROID_TOOLS
ARG ANDROID_LEVEL
ARG ANDROID_BUILD_TOOLS
ARG NDK_VERSION
ARG JAVA_HOME_ARG
ENV JAVA_HOME=$JAVA_HOME_ARG
RUN mkdir -p /opt/android && \
    cd /opt/android && \
    wget "https://dl.google.com/android/repository/commandlinetools-linux-${ANDROID_TOOLS}_latest.zip" && \
    unzip "commandlinetools-linux-${ANDROID_TOOLS}_latest.zip" && \
    rm "commandlinetools-linux-${ANDROID_TOOLS}_latest.zip" && \
    yes | /opt/android/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android-sdk-linux --update && \
    yes | /opt/android/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android-sdk-linux "ndk;${NDK_VERSION}" && \
    yes | /opt/android/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android-sdk-linux "platforms;android-${ANDROID_LEVEL}" && \
    yes | /opt/android/cmdline-tools/bin/sdkmanager --sdk_root=/opt/android-sdk-linux "build-tools;${ANDROID_BUILD_TOOLS}" && \
    find /opt/android-sdk-linux/ndk -name libz.a -delete

FROM download AS openssl_download
ARG OPENSSL_VERSION
RUN mkdir -p /usr/src \
    && cd /usr/src \
    && wget "https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz" \
    && tar -xf "openssl-${OPENSSL_VERSION}.tar.gz" \
    && rm "openssl-${OPENSSL_VERSION}.tar.gz" \
    && mv "openssl-${OPENSSL_VERSION}" openssl

FROM tools AS openssl_host
ARG JOBS
COPY --from=openssl_download /usr/src/openssl /usr/src/openssl
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && /usr/src/openssl/Configure --prefix=/usr/local \
    && make -j${JOBS} build_sw \
    && make install_sw

FROM download AS cmake-download
ARG CMAKE_VERSION
RUN mkdir -p /usr/src \
    && cd /usr/src \
    && wget -O /usr/src/cmake.tar.gz "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}.tar.gz" \
    && tar -xf cmake.tar.gz \
    && rm cmake.tar.gz \
    && mv "cmake-${CMAKE_VERSION}" cmake

FROM tools AS cmake
RUN --mount=type=tmpfs,target=/var/cache/dnf dnf install -y cmake
COPY --from=openssl_host /usr/local /usr/local
COPY --from=cmake-download /usr/src/cmake /usr/src/cmake
RUN mkdir -p /tmp/build \
    && cd /tmp/build \
    && cmake -GNinja /usr/src/cmake \
    && cmake --build . \
    && cmake --install .

FROM tools AS build_base
ARG NDK_VERSION
COPY --from=android /opt/android-sdk-linux /opt/android-sdk-linux
RUN ln -s "/opt/android-sdk-linux/ndk/${NDK_VERSION}" /opt/ndk
COPY --from=cmake /usr/local /usr/local
COPY --from=openssl_host /usr/local /usr/local

FROM download AS boost_download
ARG BOOST_FOR_ANDROID_COMMIT_HASH
RUN mkdir -p /usr/src && cd /usr/src \
    && wget "https://github.com/Open-Transactions/Boost-for-Android/archive/${BOOST_FOR_ANDROID_COMMIT_HASH}.zip" \
    && unzip "${BOOST_FOR_ANDROID_COMMIT_HASH}.zip" \
    && rm "${BOOST_FOR_ANDROID_COMMIT_HASH}.zip" \
    && mv "Boost-for-Android-${BOOST_FOR_ANDROID_COMMIT_HASH}" boost \
    && cd /usr/src/boost \
    && wget https://boostorg.jfrog.io/artifactory/main/release/1.84.0/source/boost_1_84_0.tar.bz2

FROM build_base AS boost
COPY --from=boost_download /usr/src/boost /usr/src/boost
ARG ANDROID_LEVEL
COPY ./boost.patch /usr/src/boost/patches/boost-1_84_0/suffix.patch
RUN --mount=type=tmpfs,target=/tmp/build rsync -a /usr/src/boost/ /tmp/build/ \
    && cd /tmp/build \
    && ./build-android.sh --boost=1.84.0 --arch=armeabi-v7a,arm64-v8a,x86,x86_64 --target-version="${ANDROID_LEVEL}" --layout=system /opt/ndk \
    && mkdir -p /usr/android \
    && mv /tmp/build/build/out/arm64-v8a /usr/android/arm64 \
    && mv /tmp/build/build/out/armeabi-v7a /usr/android/arm \
    && mv /tmp/build/build/out/x86_64 /usr/android/x64 \
    && mv /tmp/build/build/out/x86 /usr/android/x86

FROM download AS sodium_download
ARG SODIUM_COMMIT_HASH
RUN mkdir -p /usr/src && cd /usr/src && wget "https://github.com/jedisct1/libsodium/archive/${SODIUM_COMMIT_HASH}.zip" \
    && unzip "${SODIUM_COMMIT_HASH}.zip" && rm "${SODIUM_COMMIT_HASH}.zip" && mv "libsodium-${SODIUM_COMMIT_HASH}" sodium

FROM build_base AS sodium
COPY --from=sodium_download /usr/src/sodium /usr/src/sodium
ARG ANDROID_LEVEL
ARG ANDROID_LEVEL_TOOLCHAIN
ARG JOBS
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && export AR="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar" \
    && export AS="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-as" \
    && export CC="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android${ANDROID_LEVEL_TOOLCHAIN}-clang" \
    && export CXX="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android${ANDROID_LEVEL_TOOLCHAIN}-clang++" \
    && export LD="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/ld" \
    && export RANLIB="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ranlib" \
    && export STRIP="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip" \
    && /usr/src/sodium/configure \
        --prefix=/usr/android/arm64 \
        --host=aarch64-linux-android \
        --with-sysroot="/opt/ndk/sysroot/usr" \
        --disable-asm \
        --disable-minimal \
        --disable-soname-versions \
        --enable-shared \
        --disable-static \
    && make -j$JOBS install
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
        && export AR="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar" \
        && export AS="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-as" \
        && export CC="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi${ANDROID_LEVEL_TOOLCHAIN}-clang" \
        && export CXX="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi${ANDROID_LEVEL_TOOLCHAIN}-clang++" \
        && export LD="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/ld" \
        && export RANLIB="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ranlib" \
        && export STRIP="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip" \
    && /usr/src/sodium/configure \
        --prefix=/usr/android/arm \
        --host=armv7a-linux-androideabi \
        --with-sysroot="/opt/ndk/sysroot/usr" \
        --disable-asm \
        --disable-minimal \
        --disable-soname-versions \
        --enable-shared \
        --disable-static \
    && make -j$JOBS install
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
        && export AR="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar" \
        && export AS="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-as" \
        && export CC="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/x86_64-linux-android${ANDROID_LEVEL_TOOLCHAIN}-clang" \
        && export CXX="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/x86_64-linux-android${ANDROID_LEVEL_TOOLCHAIN}-clang++" \
        && export LD="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/ld" \
        && export RANLIB="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ranlib" \
        && export STRIP="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip" \
    && /usr/src/sodium/configure \
        --prefix=/usr/android/x64 \
        --host=x86_64-linux-android \
        --with-sysroot="/opt/ndk/sysroot/usr" \
        --disable-asm \
        --disable-minimal \
        --disable-soname-versions \
        --enable-shared \
        --disable-static \
    && make -j$JOBS install
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
        && export AR="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar" \
        && export AS="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-as" \
        && export CC="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/i686-linux-android${ANDROID_LEVEL_TOOLCHAIN}-clang" \
        && export CXX="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/i686-linux-android${ANDROID_LEVEL_TOOLCHAIN}-clang++" \
        && export LD="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/ld" \
        && export RANLIB="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ranlib" \
        && export STRIP="/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip" \
    && /usr/src/sodium/configure \
        --prefix=/usr/android/x86 \
        --host=i686-linux-android \
        --with-sysroot="/opt/ndk/sysroot/usr" \
        --disable-asm \
        --disable-minimal \
        --disable-soname-versions \
        --enable-shared \
        --disable-static \
    && make -j$JOBS install

FROM download AS lmdb_download
ARG LMDB_COMMIT_HASH
RUN mkdir -p /usr/src && cd /usr/src && wget "https://github.com/LMDB/lmdb/archive/${LMDB_COMMIT_HASH}.zip" \
    && unzip "${LMDB_COMMIT_HASH}.zip" && rm "${LMDB_COMMIT_HASH}.zip" && mv "lmdb-${LMDB_COMMIT_HASH}" lmdb

FROM build_base AS lmdb
COPY --from=lmdb_download /usr/src/lmdb /usr/src/lmdb
ARG ANDROID_LEVEL_TOOLCHAIN
ARG JOBS
RUN --mount=type=tmpfs,target=/tmp/build \
    rsync -a /usr/src/lmdb/libraries/liblmdb/ /tmp/build \
    && sed -i "s~gcc~/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android${ANDROID_LEVEL_TOOLCHAIN}-clang~" /tmp/build/Makefile \
    && sed -i "s~= ar~= /opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar~" /tmp/build/Makefile \
    && sed -i "s~/usr/local~/usr/android/arm64~" /tmp/build/Makefile \
    && sed -i "s/liblmdb.a liblmdb\$(SOEXT)/liblmdb\$(SOEXT)/" /tmp/build/Makefile \
    && cd /tmp/build \
    && make -j$JOBS install
RUN --mount=type=tmpfs,target=/tmp/build \
    rsync -a /usr/src/lmdb/libraries/liblmdb/ /tmp/build \
    && sed -i "s~gcc~/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi${ANDROID_LEVEL_TOOLCHAIN}-clang~" /tmp/build/Makefile \
    && sed -i "s~= ar~= /opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar~" /tmp/build/Makefile \
    && sed -i "s~/usr/local~/usr/android/arm~" /tmp/build/Makefile \
    && sed -i "s/liblmdb.a liblmdb\$(SOEXT)/liblmdb\$(SOEXT)/" /tmp/build/Makefile \
    && cd /tmp/build \
    && make -j$JOBS install
RUN --mount=type=tmpfs,target=/tmp/build \
    rsync -a /usr/src/lmdb/libraries/liblmdb/ /tmp/build \
    && sed -i "s~gcc~/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/x86_64-linux-android${ANDROID_LEVEL_TOOLCHAIN}-clang~" /tmp/build/Makefile \
    && sed -i "s~= ar~= /opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar~" /tmp/build/Makefile \
    && sed -i "s~/usr/local~/usr/android/x64~" /tmp/build/Makefile \
    && sed -i "s/liblmdb.a liblmdb\$(SOEXT)/liblmdb\$(SOEXT)/" /tmp/build/Makefile \
    && cd /tmp/build \
    && make -j$JOBS install
RUN --mount=type=tmpfs,target=/tmp/build \
    rsync -a /usr/src/lmdb/libraries/liblmdb/ /tmp/build \
    && sed -i "s~gcc~/opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/i686-linux-android${ANDROID_LEVEL_TOOLCHAIN}-clang~" /tmp/build/Makefile \
    && sed -i "s~= ar~= /opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar~" /tmp/build/Makefile \
    && sed -i "s~/usr/local~/usr/android/x86~" /tmp/build/Makefile \
    && sed -i "s/liblmdb.a liblmdb\$(SOEXT)/liblmdb\$(SOEXT)/" /tmp/build/Makefile \
    && cd /tmp/build \
    && make -j$JOBS install

FROM download AS protobuf_download
ARG PROTOBUF_COMMIT_HASH
RUN mkdir -p /usr/src \
    && cd /usr/src \
    && git clone https://github.com/protocolbuffers/protobuf /usr/src/protobuf \
    && git -C /usr/src/protobuf reset --hard "${PROTOBUF_COMMIT_HASH}" \
    && git -C /usr/src/protobuf submodule update --init --recursive

FROM build_base AS protobuf
COPY --from=protobuf_download /usr/src/protobuf /usr/src/protobuf
ARG ANDROID_LEVEL
ARG CMAKE_BUILD_TYPE
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && /usr/local/bin/cmake \
        -GNinja \
        -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
        -DCMAKE_INSTALL_PREFIX=/usr/local/protobuf \
        -DBUILD_SHARED_LIBS=ON \
        -Dprotobuf_BUILD_TESTS=OFF \
        -Dprotobuf_BUILD_CONFORMANCE=OFF \
        -Dprotobuf_BUILD_PROTOC_BINARIES=ON \
        -Dprotobuf_BUILD_SHARED_LIBS=ON \
        /usr/src/protobuf \
    && ninja install
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && /usr/local/bin/cmake \
        -GNinja \
        -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
        -DANDROID_ABI=arm64-v8a \
        -DANDROID_PLATFORM="android-${ANDROID_LEVEL}" \
        -DANDROID_STL="c++_shared" \
        -DCMAKE_TOOLCHAIN_FILE="/opt/ndk/build/cmake/android.toolchain.cmake" \
        -DCMAKE_INSTALL_PREFIX=/usr/android/arm64 \
        -DBUILD_SHARED_LIBS=ON \
        -Dprotobuf_BUILD_TESTS=OFF \
        -Dprotobuf_BUILD_CONFORMANCE=OFF \
        -Dprotobuf_BUILD_PROTOC_BINARIES=OFF \
        -Dprotobuf_WITH_ZLIB=OFF \
        -Dprotobuf_BUILD_SHARED_LIBS=ON \
        -DCMAKE_CXX_FLAGS="-Wl,-llog" \
        /usr/src/protobuf \
    && ninja install
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && /usr/local/bin/cmake \
        -GNinja \
        -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
        -DANDROID_ABI=armeabi-v7a \
        -DANDROID_PLATFORM="android-${ANDROID_LEVEL}" \
        -DANDROID_STL="c++_shared" \
        -DCMAKE_TOOLCHAIN_FILE="/opt/ndk/build/cmake/android.toolchain.cmake" \
        -DCMAKE_INSTALL_PREFIX=/usr/android/arm \
        -DBUILD_SHARED_LIBS=ON \
        -Dprotobuf_BUILD_TESTS=OFF \
        -Dprotobuf_BUILD_CONFORMANCE=OFF \
        -Dprotobuf_BUILD_PROTOC_BINARIES=OFF \
        -Dprotobuf_WITH_ZLIB=OFF \
        -Dprotobuf_BUILD_SHARED_LIBS=ON \
        -DCMAKE_CXX_FLAGS="-Wl,-llog" \
        /usr/src/protobuf \
    && ninja install
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && /usr/local/bin/cmake \
        -GNinja \
        -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
        -DANDROID_ABI=x86_64 \
        -DANDROID_PLATFORM="android-${ANDROID_LEVEL}" \
        -DANDROID_STL="c++_shared" \
        -DCMAKE_TOOLCHAIN_FILE="/opt/ndk/build/cmake/android.toolchain.cmake" \
        -DCMAKE_INSTALL_PREFIX=/usr/android/x64 \
        -DBUILD_SHARED_LIBS=ON \
        -Dprotobuf_BUILD_TESTS=OFF \
        -Dprotobuf_BUILD_CONFORMANCE=OFF \
        -Dprotobuf_BUILD_PROTOC_BINARIES=OFF \
        -Dprotobuf_WITH_ZLIB=OFF \
        -Dprotobuf_BUILD_SHARED_LIBS=ON \
        -DCMAKE_CXX_FLAGS="-Wl,-llog" \
        /usr/src/protobuf \
    && ninja install
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && /usr/local/bin/cmake \
        -GNinja \
        -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
        -DANDROID_ABI=x86 \
        -DANDROID_PLATFORM="android-${ANDROID_LEVEL}" \
        -DANDROID_STL="c++_shared" \
        -DCMAKE_TOOLCHAIN_FILE="/opt/ndk/build/cmake/android.toolchain.cmake" \
        -DCMAKE_INSTALL_PREFIX=/usr/android/x86 \
        -DBUILD_SHARED_LIBS=ON \
        -Dprotobuf_BUILD_TESTS=OFF \
        -Dprotobuf_BUILD_CONFORMANCE=OFF \
        -Dprotobuf_BUILD_PROTOC_BINARIES=OFF \
        -Dprotobuf_WITH_ZLIB=OFF \
        -Dprotobuf_BUILD_SHARED_LIBS=ON \
        -DCMAKE_CXX_FLAGS="-Wl,-llog" \
        /usr/src/protobuf \
    && ninja install

FROM build_base AS openssl
COPY --from=openssl_download /usr/src/openssl /usr/src/openssl
ARG JOBS
ARG ANDROID_LEVEL
ENV ANDROID_NDK_ROOT=/opt/ndk
RUN --mount=type=tmpfs,target=/tmp/build export export PATH="${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH" \
    && cd /tmp/build \
    && /usr/src/openssl/Configure --prefix=/usr/android/arm64 android-arm64 "-D__ANDROID_API__=${ANDROID_LEVEL}" \
    && make -j${JOBS} build_sw \
    && make install_sw
RUN --mount=type=tmpfs,target=/tmp/build export export PATH="${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH" \
    && cd /tmp/build \
    && /usr/src/openssl/Configure --prefix=/usr/android/arm android-arm "-D__ANDROID_API__=${ANDROID_LEVEL}" \
    && make -j${JOBS} build_sw \
    && make install_sw
RUN --mount=type=tmpfs,target=/tmp/build export export PATH="${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH" \
    && cd /tmp/build \
    && /usr/src/openssl/Configure --prefix=/usr/android/x64 android-x86_64 "-D__ANDROID_API__=${ANDROID_LEVEL}" \
    && make -j${JOBS} build_sw \
    && make install_sw
RUN --mount=type=tmpfs,target=/tmp/build export export PATH="${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH" \
    && cd /tmp/build \
    && /usr/src/openssl/Configure --prefix=/usr/android/x86 android-x86 "-D__ANDROID_API__=${ANDROID_LEVEL}" \
    && make -j${JOBS} build_sw \
    && make install_sw

FROM download AS zeromq_download
ARG LIBZMQ_COMMIT_HASH
RUN mkdir -p /usr/src && cd /usr/src && wget "https://github.com/zeromq/libzmq/archive/${LIBZMQ_COMMIT_HASH}.zip" \
    && unzip "${LIBZMQ_COMMIT_HASH}.zip" && rm "${LIBZMQ_COMMIT_HASH}.zip" && mv "libzmq-${LIBZMQ_COMMIT_HASH}" zeromq

FROM build_base AS zeromq
COPY --from=zeromq_download /usr/src/zeromq /usr/src/zeromq
COPY --from=sodium /usr/android /usr/android
ARG ANDROID_LEVEL
ARG CMAKE_BUILD_TYPE
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && /usr/local/bin/cmake \
        -GNinja \
        -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
        -DANDROID_ABI=arm64-v8a \
        -DANDROID_PLATFORM="android-${ANDROID_LEVEL}" \
        -DANDROID_STL="c++_shared" \
        -DCMAKE_TOOLCHAIN_FILE="/opt/ndk/build/cmake/android.toolchain.cmake" \
        -DCMAKE_INSTALL_PREFIX=/usr/android/arm64 \
        -DCMAKE_FIND_ROOT_PATH=/usr/android/arm64 \
        -DBUILD_SHARED_LIBS=ON \
        -DENABLE_ASAN=OFF \
        -DENABLE_INTRINSICS=OFF \
        -DWITH_OPENPGM=OFF \
        -DWITH_VMCI=OFF \
        -DZMQ_BUILD_FRAMEWORK=OFF \
        -DWITH_LIBSODIUM=ON \
        -DENABLE_CURVE=ON \
        -DENABLE_DRAFTS=OFF \
        -DWITH_MILITANT=OFF \
        -DLIBZMQ_PEDANTIC=OFF \
        -DLIBZMQ_WERROR=OFF \
        -DWITH_DOCS=OFF \
        -DBUILD_SHARED=ON \
        -DBUILD_STATIC=OFF \
        -DWITH_PERF_TOOL=OFF \
        -DBUILD_TESTS=OFF \
        -DENABLE_CPACK=OFF \
        /usr/src/zeromq \
    && ninja install
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && cmake \
        -GNinja \
        -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
        -DANDROID_ABI=armeabi-v7a \
        -DANDROID_PLATFORM="android-${ANDROID_LEVEL}" \
        -DANDROID_STL="c++_shared" \
        -DCMAKE_TOOLCHAIN_FILE="/opt/ndk/build/cmake/android.toolchain.cmake" \
        -DCMAKE_INSTALL_PREFIX=/usr/android/arm \
        -DCMAKE_FIND_ROOT_PATH=/usr/android/arm \
        -DBUILD_SHARED_LIBS=ON \
        -DENABLE_ASAN=OFF \
        -DENABLE_INTRINSICS=OFF \
        -DWITH_OPENPGM=OFF \
        -DWITH_VMCI=OFF \
        -DZMQ_BUILD_FRAMEWORK=OFF \
        -DWITH_LIBSODIUM=ON \
        -DENABLE_CURVE=ON \
        -DENABLE_DRAFTS=OFF \
        -DWITH_MILITANT=OFF \
        -DLIBZMQ_PEDANTIC=OFF \
        -DLIBZMQ_WERROR=OFF \
        -DWITH_DOCS=OFF \
        -DBUILD_SHARED=ON \
        -DBUILD_STATIC=OFF \
        -DWITH_PERF_TOOL=OFF \
        -DBUILD_TESTS=OFF \
        -DENABLE_CPACK=OFF \
        /usr/src/zeromq \
    && ninja install
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && /usr/local/bin/cmake \
        -GNinja \
        -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
        -DANDROID_ABI=x86_64 \
        -DANDROID_PLATFORM="android-${ANDROID_LEVEL}" \
        -DANDROID_STL="c++_shared" \
        -DCMAKE_TOOLCHAIN_FILE="/opt/ndk/build/cmake/android.toolchain.cmake" \
        -DCMAKE_INSTALL_PREFIX=/usr/android/x64 \
        -DCMAKE_FIND_ROOT_PATH=/usr/android/x64 \
        -DENABLE_ASAN=OFF \
        -DENABLE_INTRINSICS=OFF \
        -DWITH_OPENPGM=OFF \
        -DWITH_VMCI=OFF \
        -DZMQ_BUILD_FRAMEWORK=OFF \
        -DWITH_LIBSODIUM=ON \
        -DENABLE_CURVE=ON \
        -DENABLE_DRAFTS=OFF \
        -DWITH_MILITANT=OFF \
        -DLIBZMQ_PEDANTIC=OFF \
        -DLIBZMQ_WERROR=OFF \
        -DWITH_DOCS=OFF \
        -DBUILD_SHARED=ON \
        -DBUILD_STATIC=OFF \
        -DWITH_PERF_TOOL=OFF \
        -DBUILD_TESTS=OFF \
        -DENABLE_CPACK=OFF \
        -DBUILD_SHARED_LIBS=ON \
        /usr/src/zeromq \
    && ninja install
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && /usr/local/bin/cmake \
        -GNinja \
        -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
        -DANDROID_ABI=x86 \
        -DANDROID_PLATFORM="android-${ANDROID_LEVEL}" \
        -DANDROID_STL="c++_shared" \
        -DCMAKE_TOOLCHAIN_FILE="/opt/ndk/build/cmake/android.toolchain.cmake" \
        -DCMAKE_INSTALL_PREFIX=/usr/android/x86 \
        -DCMAKE_FIND_ROOT_PATH=/usr/android/x86 \
        -DENABLE_ASAN=OFF \
        -DENABLE_INTRINSICS=OFF \
        -DWITH_OPENPGM=OFF \
        -DWITH_VMCI=OFF \
        -DZMQ_BUILD_FRAMEWORK=OFF \
        -DWITH_LIBSODIUM=ON \
        -DENABLE_CURVE=ON \
        -DENABLE_DRAFTS=OFF \
        -DWITH_MILITANT=OFF \
        -DLIBZMQ_PEDANTIC=OFF \
        -DLIBZMQ_WERROR=OFF \
        -DWITH_DOCS=OFF \
        -DBUILD_SHARED=ON \
        -DBUILD_STATIC=OFF \
        -DWITH_PERF_TOOL=OFF \
        -DBUILD_TESTS=OFF \
        -DENABLE_CPACK=OFF \
        -DBUILD_SHARED_LIBS=ON \
        /usr/src/zeromq \
    && ninja install

FROM download AS gtest_download
ARG GTEST
RUN mkdir -p /usr/src && cd /usr/src && git clone https://github.com/google/googletest.git \
    && cd googletest && git checkout "${GTEST}" && git submodule update --init --recursive

FROM build_base as gtest
COPY --from=gtest_download /usr/src/googletest /usr/src/googletest
ARG ANDROID_LEVEL
ARG CMAKE_BUILD_TYPE
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && /usr/local/bin/cmake \
        -GNinja \
        -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
        -DANDROID_ABI=arm64-v8a \
        -DANDROID_PLATFORM="android-${ANDROID_LEVEL}" \
        -DANDROID_STL="c++_shared" \
        -DCMAKE_TOOLCHAIN_FILE="/opt/ndk/build/cmake/android.toolchain.cmake" \
        -DCMAKE_INSTALL_PREFIX=/usr/android/arm64 \
        -DCMAKE_FIND_ROOT_PATH=/usr/android/arm64 \
        -DBUILD_SHARED_LIBS=ON \
        /usr/src/googletest \
    && ninja install
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && /usr/local/bin/cmake \
        -GNinja \
        -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
        -DANDROID_ABI=armeabi-v7a \
        -DANDROID_PLATFORM="android-${ANDROID_LEVEL}" \
        -DANDROID_STL="c++_shared" \
        -DCMAKE_TOOLCHAIN_FILE="/opt/ndk/build/cmake/android.toolchain.cmake" \
        -DCMAKE_INSTALL_PREFIX=/usr/android/arm \
        -DCMAKE_FIND_ROOT_PATH=/usr/android/arm \
        -DBUILD_SHARED_LIBS=ON \
        /usr/src/googletest \
    && ninja install
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && /usr/local/bin/cmake \
        -GNinja \
        -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
        -DANDROID_ABI=x86_64 \
        -DANDROID_PLATFORM="android-${ANDROID_LEVEL}" \
        -DANDROID_STL="c++_shared" \
        -DCMAKE_TOOLCHAIN_FILE="/opt/ndk/build/cmake/android.toolchain.cmake" \
        -DCMAKE_INSTALL_PREFIX=/usr/android/x64 \
        -DCMAKE_FIND_ROOT_PATH=/usr/android/x64 \
        -DBUILD_SHARED_LIBS=ON \
        /usr/src/googletest \
    && ninja install
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && /usr/local/bin/cmake \
        -GNinja \
        -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
        -DANDROID_ABI=x86 \
        -DANDROID_PLATFORM="android-${ANDROID_LEVEL}" \
        -DANDROID_STL="c++_shared" \
        -DCMAKE_TOOLCHAIN_FILE="/opt/ndk/build/cmake/android.toolchain.cmake" \
        -DCMAKE_INSTALL_PREFIX=/usr/android/x86 \
        -DCMAKE_FIND_ROOT_PATH=/usr/android/x86 \
        -DBUILD_SHARED_LIBS=ON \
        /usr/src/googletest \
    && ninja install

FROM download AS otcommon_download
ARG OTCOMMON_VERSION
RUN --mount=type=tmpfs,target=/tmp/download/ cd /tmp/download \
    && wget -O /tmp/download/otcommon.tar.gz "https://github.com/Open-Transactions/otcommon/archive/refs/tags/${OTCOMMON_VERSION}.tar.gz" \
    && tar -xf otcommon.tar.gz \
    && mv "otcommon-${OTCOMMON_VERSION}" /usr/src/otcommon

FROM build_base AS otcommon
ARG OTCOMMON_VERSION
COPY --from=otcommon_download /usr/src/otcommon /usr/src/otcommon
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && cmake \
        -DCMAKE_INSTALL_PREFIX=/usr/otcommon \
        -GNinja \
        -Dotcommon_GIT_VERSION=${OTCOMMON_VERSION} \
        /usr/src/otcommon \
    && cmake --install .

FROM download AS qt_download
ARG QT_RELEASE
ARG QT_BRANCH
ARG QT_VERSION
RUN mkdir -p /usr/src && cd /usr/src \
    && wget "https://download.qt.io/${QT_RELEASE}/qt/${QT_BRANCH}/${QT_VERSION}/single/qt-everywhere-src-${QT_VERSION}.tar.xz" \
    && tar -xf "qt-everywhere-src-${QT_VERSION}.tar.xz" \
    && rm "qt-everywhere-src-${QT_VERSION}.tar.xz" \
    && mv qt-everywhere-src-${QT_VERSION} qt

FROM build_base as host_qt
RUN --mount=type=tmpfs,target=/var/cache/dnf \
    dnf install -y \
    SDL2-devel \
    assimp-devel \
    bison \
    bluez-libs-devel \
    clang-devel \
    cups-devel \
    dbus-devel \
    egl-wayland-devel \
    expat-devel \
    flex \
    fontconfig-devel \
    freetype-devel \
    gperf \
    harfbuzz-devel \
    jsoncpp-devel \
    lcms2-devel \
    libX11-devel \
    libXext-devel \
    libXfixes-devel \
    libXi-devel \
    libXinerama-devel \
    libXrandr-devel \
    libXrender-devel \
    libevent-devel \
    libglvnd-devel \
    libicu-devel \
    libicu-devel \
    libjpeg-devel \
    libpng-devel \
    libproxy-devel \
    libstdc++-devel \
    libstdc++-static \
    libtiff-devel \
    libvpx-devel \
    libwebp-devel \
    libxcb-devel \
    libxkbcommon-devel \
    libxkbcommon-x11-devel \
    libxml2-devel \
    libxslt-devel \
    llvm-devel \
    mesa-libOSMesa-devel \
    nodejs \
    nss-devel \
    opus-devel \
    pcre2-devel \
    perl-English \
    protobuf-devel \
    python2 \
    snappy-devel \
    sqlite-devel \
    vulkan-devel \
    wayland-devel \
    zlib-devel
ARG JAVA_HOME_ARG
ENV JAVA_HOME=$JAVA_HOME_ARG
COPY --from=qt_download /usr/src/qt /usr/src/qt
ARG JOBS
ARG ANDROID_LEVEL_TOOLCHAIN
RUN cd /opt/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/ && \
    ln -s llvm-ranlib i686-linux-android-ranlib && \
    ln -s llvm-ranlib x86_64-linux-android-ranlib && \
    ln -s llvm-ranlib arm-linux-androideabi-ranlib && \
    ln -s llvm-ranlib aarch64-linux-android-ranlib
RUN mkdir -p /tmp/build && cd /tmp/build && /usr/src/qt/configure \
    --prefix=/usr/host_qt \
    -opensource \
    -confirm-license \
    -nomake tests \
    -nomake examples \
    -no-warnings-are-errors \
    -c++std c++20 \
    && ninja install \
    && rm -rf /tmp/build

FROM host_qt as qt
ARG JAVA_HOME_ARG
ENV JAVA_HOME=$JAVA_HOME_ARG
ARG JOBS
ARG ANDROID_LEVEL_TOOLCHAIN
RUN mkdir -p /tmp/build && cd /tmp/build && export PATH=$PATH:$JAVA_HOME/bin && /usr/src/qt/configure \
    --prefix=/usr/qt-for-android/arm64 \
    --disable-rpath \
    -qt-host-path /usr/host_qt \
    -opensource \
    -confirm-license \
    -linker lld \
    -xplatform android-clang \
    -android-ndk-platform android-${ANDROID_LEVEL_TOOLCHAIN} \
    -android-ndk /opt/ndk \
    -android-sdk /opt/android-sdk-linux \
    -android-abis arm64-v8a \
    -nomake tests \
    -nomake examples \
    -no-warnings-are-errors \
    -c++std c++20 \
    && ninja install \
    && rm -rf /tmp/build
RUN mkdir -p /tmp/build && cd /tmp/build && export PATH=$PATH:$JAVA_HOME/bin && /usr/src/qt/configure \
    --prefix=/usr/qt-for-android/arm \
    --disable-rpath \
    -qt-host-path /usr/host_qt \
    -opensource \
    -confirm-license \
    -linker lld \
    -xplatform android-clang \
    -android-ndk-platform android-${ANDROID_LEVEL_TOOLCHAIN} \
    -android-ndk /opt/ndk \
    -android-sdk /opt/android-sdk-linux \
    -android-abis armeabi-v7a \
    -nomake tests \
    -nomake examples \
    -no-warnings-are-errors \
    -c++std c++20 \
    && ninja install \
    && rm -rf /tmp/build
RUN mkdir -p /tmp/build && cd /tmp/build && export PATH=$PATH:$JAVA_HOME/bin && /usr/src/qt/configure \
    --prefix=/usr/qt-for-android/x64 \
    --disable-rpath \
    -qt-host-path /usr/host_qt \
    -opensource \
    -confirm-license \
    -linker lld \
    -xplatform android-clang \
    -android-ndk-platform android-${ANDROID_LEVEL_TOOLCHAIN} \
    -android-ndk /opt/ndk \
    -android-sdk /opt/android-sdk-linux \
    -android-abis x86_64 \
    -nomake tests \
    -nomake examples \
    -no-warnings-are-errors \
    -c++std c++20 \
    && ninja install \
    && rm -rf /tmp/build
RUN mkdir -p /tmp/build && cd /tmp/build && export PATH=$PATH:$JAVA_HOME/bin && /usr/src/qt/configure \
    --prefix=/usr/qt-for-android/x86 \
    --disable-rpath \
    -qt-host-path /usr/host_qt \
    -opensource \
    -confirm-license \
    -linker lld \
    -xplatform android-clang \
    -android-ndk-platform android-${ANDROID_LEVEL_TOOLCHAIN} \
    -android-ndk /opt/ndk \
    -android-sdk /opt/android-sdk-linux \
    -android-abis x86 \
    -nomake tests \
    -nomake examples \
    -no-warnings-are-errors \
    -c++std c++20 \
    && ninja install \
    && rm -rf /tmp/build

FROM run AS final
ARG NDK_VERSION
COPY --from=android /opt/android-sdk-linux/ndk /opt/android-sdk-linux/ndk
RUN ln -s "/opt/android-sdk-linux/ndk/${NDK_VERSION}" /opt/ndk
COPY --from=host_qt /usr/host_qt /usr/host_qt
COPY --from=qt /usr/qt-for-android /usr/qt-for-android
COPY --from=boost /usr/android /usr/android
COPY --from=sodium /usr/android /usr/android
COPY --from=lmdb /usr/android /usr/android
COPY --from=protobuf /usr/android /usr/android
COPY --from=protobuf /usr/local/protobuf /usr
COPY --from=openssl /usr/android /usr/android
COPY --from=zeromq /usr/android /usr/android
COPY --from=gtest /usr/android /usr/android
COPY --from=cmake /usr/local /usr/local
COPY --from=otcommon /usr/otcommon /usr/otcommon
COPY ./build /usr/bin
RUN /usr/bin/git config --system --add safe.directory '*'
ENTRYPOINT [ "/usr/bin/build" ]
CMD []
