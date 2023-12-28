ARG BASE_FEDORA_VERSION="39"
ARG DOWNLOAD_ALPINE_VERSION="3.18.5"
ARG OTCOMMON_COMMIT_HASH="a8cb2a10e7eff4fdece28feea681541b70728053"
ARG OTCOMMON_VERSION="2.0.0-0-ga8cb2a1"
ARG OPENTXS_REPO="https://github.com/open-transactions/opentxs"
ARG IWYU_COMMIT_HASH="dbecade3b575678cc3ccc4dd2d0942e8ff3c7527"
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

FROM fedora:${BASE_FEDORA_VERSION} AS base

FROM base AS build
RUN --mount=type=tmpfs,target=/var/cache/dnf dnf install -y \
    gcc \
    gcc-c++ \
    git \
    libsecp256k1-devel \
    libsodium-devel \
    llvm \
    lmdb-devel \
    ninja-build \
    openssl-devel \
    protobuf-compiler \
    protobuf-lite-devel \
    qt6-qtbase-devel \
    qt6-qtdeclarative-devel \
    qt6-qtsvg-devel \
    rsync \
    tbb-devel \
    zeromq-devel

FROM base AS run
RUN --mount=type=tmpfs,target=/var/cache/dnf dnf install -y \
    libsecp256k1 \
    libsodium \
    lmdb \
    openssl-libs \
    protobuf-lite \
    qt6-qtbase \
    qt6-qtdeclarative \
    qt6-qtsvg \
    tbb \
    zeromq

FROM build AS cmake-bootstrap
RUN --mount=type=tmpfs,target=/var/cache/dnf dnf install -y cmake

FROM build AS iwyu-bootstrap
RUN --mount=type=tmpfs,target=/var/cache/dnf dnf install -y \
    clang-devel \
    llvm-devel

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

FROM download AS iwyu-download
ARG IWYU_COMMIT_HASH
RUN mkdir -p /usr/src && git clone --recursive "https://github.com/include-what-you-use/include-what-you-use" /usr/src/iwyu && cd /usr/src/iwyu && git reset --hard "${IWYU_COMMIT_HASH}"

FROM iwyu-bootstrap AS iwyu
COPY --from=cmake /usr/local /usr/
RUN ldconfig
COPY --from=iwyu-download /usr/src/iwyu /usr/src/iwyu
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && /usr/bin/cmake -GNinja /usr/src/iwyu \
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
    && /usr/bin/cmake --build /tmp/opentxs \
    && /usr/bin/cmake --install /tmp/opentxs

FROM run AS opentxs
COPY --from=boost /usr/local/lib /usr/lib
COPY --from=compile /usr/local/lib64 /usr/lib64
RUN ldconfig

FROM opentransactions/ci:39_0 AS ci

FROM build AS opentxs-devel
COPY --from=cmake /usr/local /usr
COPY --from=iwyu /usr/local /usr
COPY --from=otcommon /usr/local /usr
COPY --from=boost /usr/local /usr
COPY --from=compile /usr/local /usr
RUN ldconfig
COPY --from=ci /usr/bin/run-iwyu.sh /usr/bin/run-iwyu.sh
COPY --from=ci /usr/bin/build-opentxs.sh /usr/bin/build-opentxs.sh
