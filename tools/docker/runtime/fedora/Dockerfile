ARG OTCOMMON_VERSION="1.0.2"
ARG OPENTXS_REPO="https://github.com/open-transactions/opentxs"
ARG IWYU_COMMIT="dbecade3b575678cc3ccc4dd2d0942e8ff3c7527"
ARG BOOST_MAJOR="1"
ARG BOOST_MINOR="83"
ARG BOOST_PATCH="0"

FROM fedora:39 AS base

FROM base AS download
RUN --mount=type=tmpfs,target=/var/cache/dnf dnf install -y \
    bzip2 \
    git \
    lzo \
    p7zip \
    rsync \
    wget \
    xz

FROM download AS opentxs-download
ARG OPENTXS_REPO
ARG OPENTXS_COMMIT
RUN mkdir -p /usr/src && git clone --recursive "${OPENTXS_REPO}" /usr/src/opentxs && cd /usr/src/opentxs && git reset --hard "${OPENTXS_COMMIT}"

FROM download AS iwyu-download
ARG IWYU_COMMIT
RUN mkdir -p /usr/src && git clone --recursive "https://github.com/include-what-you-use/include-what-you-use" /usr/src/iwyu && cd /usr/src/iwyu && git reset --hard "${IWYU_COMMIT}"

FROM download AS boost-download
ARG BOOST_MAJOR
ARG BOOST_MINOR
ARG BOOST_PATCH
RUN --mount=type=tmpfs,target=/tmp/download/ cd /tmp/download \
    && wget -O /tmp/download/boost.tar.bz2 "https://boostorg.jfrog.io/artifactory/main/release/${BOOST_MAJOR}.${BOOST_MINOR}.${BOOST_PATCH}/source/boost_${BOOST_MAJOR}_${BOOST_MINOR}_${BOOST_PATCH}.tar.bz2" \
    && tar -xf /tmp/download/boost.tar.bz2 -C /tmp/download \
    && mkdir -p /tmp/download \
    && mv "/tmp/download/boost_${BOOST_MAJOR}_${BOOST_MINOR}_${BOOST_PATCH}" /usr/src/boost

FROM download AS otcommon-download
ARG OTCOMMON_VERSION
RUN --mount=type=tmpfs,target=/tmp/download/ cd /tmp/download \
    && wget -O /tmp/download/otcommon.tar.gz "https://github.com/Open-Transactions/otcommon/archive/refs/tags/${OTCOMMON_VERSION}.tar.gz" \
    && tar -xf otcommon.tar.gz \
    && mv "otcommon-${OTCOMMON_VERSION}" /usr/src/otcommon

FROM download AS build
RUN --mount=type=tmpfs,target=/var/cache/dnf dnf install -y \
    clang-devel \
    cmake \
    gcc \
    gcc-c++ \
    libsodium-devel \
    llvm-devel \
    lmdb-devel \
    ninja-build \
    openssl-devel \
    protobuf-compiler \
    protobuf-lite-devel \
    qt6-qtbase-devel \
    qt6-qtdeclarative-devel \
    qt6-qtsvg-devel \
    tbb-devel \
    zeromq-devel

FROM build AS boost
COPY --from=boost-download /usr/src/boost /usr/src/boost
RUN --mount=type=tmpfs,target=/tmp/build rsync -a /usr/src/boost/ /tmp/build/ \
    && cd /tmp/build \
    && ./bootstrap.sh --prefix=/usr/local --with-libraries=all \
    && ./b2 \
    && ./b2 --prefix=/usr/local install

FROM build AS otcommon
ARG OTCOMMON_VERSION
COPY --from=otcommon-download /usr/src/otcommon /usr/src/otcommon
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && cmake \
        -GNinja \
        -Dotcommon_GIT_VERSION=${OTCOMMON_VERSION} \
        /usr/src/otcommon \
    && cmake --install .

FROM build AS compile
COPY --from=boost /usr/local /usr
COPY --from=otcommon /usr/local /usr
RUN ldconfig
COPY --from=opentxs-download /usr/src/opentxs /usr/src/opentxs
RUN --mount=type=tmpfs,target=/tmp/opentxs cmake \
        -S /usr/src/opentxs \
        -B /tmp/opentxs \
        --preset prod \
        -DBUILD_SHARED_LIBS=ON \
        -DCMAKE_C_FLAGS="-O2" \
        -DCMAKE_CXX_FLAGS="-O2" \
        -DOT_STORAGE_SQLITE=OFF \
    && cmake --build /tmp/opentxs \
    && cmake --install /tmp/opentxs

FROM base AS run
RUN --mount=type=tmpfs,target=/var/cache/dnf dnf install -y \
    libsodium \
    lmdb \
    openssl-libs \
    protobuf-lite \
    qt6-qtbase \
    qt6-qtdeclarative \
    qt6-qtsvg \
    tbb \
    zeromq

FROM run AS opentxs
COPY --from=boost /usr/local/lib /usr/lib
COPY --from=compile /usr/local/lib64 /usr/lib64
RUN ldconfig

FROM build AS iwyu
COPY --from=iwyu-download /usr/src/iwyu /usr/src/iwyu
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && cmake -GNinja /usr/src/iwyu \
    && cmake --build . \
    && cmake --install .

FROM opentransactions/ci:39_0 AS ci

FROM build AS opentxs-devel
COPY --from=boost /usr/local /usr
COPY --from=compile /usr/local /usr
COPY --from=iwyu /usr/local /usr
RUN ldconfig
COPY --from=ci /usr/bin/run-iwyu.sh /usr/bin/run-iwyu.sh
COPY --from=ci /usr/bin/build-opentxs.sh /usr/bin/build-opentxs.sh
