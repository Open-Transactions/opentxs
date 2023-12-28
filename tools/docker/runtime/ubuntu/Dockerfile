ARG BASE_UBUNTU_VERSION="22.04"
ARG DOWNLOAD_ALPINE_VERSION="3.18.5"
ARG OTCOMMON_COMMIT_HASH="a8cb2a10e7eff4fdece28feea681541b70728053"
ARG OTCOMMON_VERSION="2.0.0-0-ga8cb2a1"
ARG OPENTXS_REPO="https://github.com/open-transactions/opentxs"
ARG BOOST_MAJOR="1"
ARG BOOST_MINOR="84"
ARG BOOST_PATCH="0"
ARG CMAKE_VERSION="3.28.1"

FROM alpine:${DOWNLOAD_ALPINE_VERSION} AS download
RUN mkdir -p /usr/src
RUN --mount=type=tmpfs,target=/var/cache/apk apk add \
    bzip2 \
    git \
    lzo \
    p7zip \
    rsync \
    wget \
    xz

FROM ubuntu:${BASE_UBUNTU_VERSION} AS base

FROM base AS build
RUN --mount=type=tmpfs,target=/var/lib/apt/lists apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get -qq install \
    g++-12 \
    gcc-12 \
    git \
    liblmdb-dev \
    libprotobuf-dev \
    libsecp256k1-dev \
    libsodium-dev \
    libssl-dev \
    libtbb-dev \
    libzmq3-dev \
    ninja-build \
    protobuf-compiler \
    rsync \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 100 --slave /usr/bin/g++ g++ /usr/bin/g++-12 --slave /usr/bin/gcov gcov /usr/bin/gcov-12

FROM base AS run
RUN --mount=type=tmpfs,target=/var/lib/apt/lists apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get -qq install \
    liblmdb0 \
    libprotobuf-lite23 \
    libsecp256k1-0 \
    libsodium23 \
    libtbb12 \
    libzmq5

FROM build AS cmake-bootstrap
RUN --mount=type=tmpfs,target=/var/lib/apt/lists apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get -qq install cmake

FROM download AS cmake-download
ARG CMAKE_VERSION
RUN --mount=type=tmpfs,target=/tmp/download/ cd /tmp/download \
    && wget -O /tmp/download/cmake.tar.gz "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}.tar.gz" \
    && tar -xf cmake.tar.gz \
    && mv "cmake-${CMAKE_VERSION}" /usr/src/cmake

FROM cmake-bootstrap AS cmake
COPY --from=cmake-download /usr/src/cmake /usr/src/cmake
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && /usr/bin/cmake -GNinja /usr/src/cmake \
    && /usr/bin/cmake --build . \
    && /usr/bin/cmake --install .

FROM download AS boost-download
ARG BOOST_MAJOR
ARG BOOST_MINOR
ARG BOOST_PATCH
RUN --mount=type=tmpfs,target=/tmp/download/ cd /tmp/download \
    && wget -O /tmp/download/boost.tar.bz2 "https://boostorg.jfrog.io/artifactory/main/release/${BOOST_MAJOR}.${BOOST_MINOR}.${BOOST_PATCH}/source/boost_${BOOST_MAJOR}_${BOOST_MINOR}_${BOOST_PATCH}.tar.bz2" \
    && tar -xf /tmp/download/boost.tar.bz2 -C /tmp/download \
    && mkdir -p /tmp/download \
    && mv "/tmp/download/boost_${BOOST_MAJOR}_${BOOST_MINOR}_${BOOST_PATCH}" /usr/src/boost

FROM build AS boost
COPY --from=cmake /usr/local /usr/
RUN ldconfig
COPY --from=boost-download /usr/src/boost /usr/src/boost
RUN --mount=type=tmpfs,target=/tmp/build rsync -a /usr/src/boost/ /tmp/build/ \
    && cd /tmp/build \
    && ./bootstrap.sh --prefix=/usr/local --with-libraries=all \
    && ./b2 \
    && ./b2 install

FROM download AS otcommon-download
ARG OTCOMMON_COMMIT_HASH
RUN --mount=type=tmpfs,target=/tmp/download/ cd /tmp/download \
    && wget -O /tmp/download/otcommon.tar.gz "https://github.com/Open-Transactions/otcommon/archive/${OTCOMMON_COMMIT_HASH}.tar.gz" \
    && tar -xf otcommon.tar.gz \
    && mv "otcommon-${OTCOMMON_COMMIT_HASH}" /usr/src/otcommon

FROM build AS otcommon
COPY --from=cmake /usr/local /usr
RUN ldconfig
ARG OTCOMMON_VERSION
COPY --from=otcommon-download /usr/src/otcommon /usr/src/otcommon
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && /usr/bin/cmake \
        -GNinja \
        -Dotcommon_GIT_VERSION=${OTCOMMON_VERSION} \
        /usr/src/otcommon \
    && /usr/bin/cmake --install .

FROM download AS opentxs-download
ARG OPENTXS_REPO
ARG OPENTXS_COMMIT
RUN mkdir -p /usr/src && git clone --recursive "${OPENTXS_REPO}" /usr/src/opentxs && cd /usr/src/opentxs && git reset --hard "${OPENTXS_COMMIT}"

FROM build AS compile
COPY --from=cmake /usr/local /usr
COPY --from=boost /usr/local /usr
COPY --from=otcommon /usr/local /usr
RUN ldconfig
COPY --from=opentxs-download /usr/src/opentxs /usr/src/opentxs
RUN --mount=type=tmpfs,target=/tmp/opentxs /usr/bin/cmake \
        -S /usr/src/opentxs \
        -B /tmp/opentxs \
        --preset prod \
        -DBUILD_SHARED_LIBS=ON \
        -DCMAKE_C_FLAGS="-O2" \
        -DCMAKE_CXX_FLAGS="-O2" \
        -DOT_STORAGE_SQLITE=OFF \
        -DOT_BUNDLED_SECP256K1=OFF \
        -DOT_WITH_QT=OFF \
        -DOT_WITH_QML=OFF \
    && /usr/bin/cmake --build /tmp/opentxs \
    && /usr/bin/cmake --install /tmp/opentxs

FROM run AS opentxs
COPY --from=boost /usr/local/lib /usr/lib
COPY --from=compile /usr/local/lib /usr/lib
RUN ldconfig

FROM build AS opentxs-devel
COPY --from=cmake /usr/local /usr
COPY --from=otcommon /usr/local /usr
COPY --from=boost /usr/local /usr
COPY --from=compile /usr/local /usr
RUN ldconfig
