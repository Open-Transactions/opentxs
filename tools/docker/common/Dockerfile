ARG DOWNLOAD_ALPINE_VERSION="3.18.5"
ARG CMAKE_VERSION="3.28.1"
ARG IWYU_COMMIT_HASH="dbecade3b575678cc3ccc4dd2d0942e8ff3c7527"
ARG BOOST_MAJOR="1"
ARG BOOST_MINOR="84"
ARG BOOST_PATCH="0"

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

FROM download AS cmake-download
ARG CMAKE_VERSION
RUN --mount=type=tmpfs,target=/tmp/download/ cd /tmp/download \
    && wget -O /tmp/download/cmake.tar.gz "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}.tar.gz" \
    && tar -xf cmake.tar.gz \
    && mv "cmake-${CMAKE_VERSION}" /usr/src/cmake

FROM download AS iwyu-download
ARG IWYU_COMMIT_HASH
RUN mkdir -p /usr/src && git clone --recursive "https://github.com/include-what-you-use/include-what-you-use" /usr/src/iwyu && cd /usr/src/iwyu && git reset --hard "${IWYU_COMMIT_HASH}"

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
ARG OTCOMMON_COMMIT_HASH
RUN --mount=type=tmpfs,target=/tmp/download/ cd /tmp/download \
    && wget -O /tmp/download/otcommon.tar.gz "https://github.com/Open-Transactions/otcommon/archive/${OTCOMMON_COMMIT_HASH}.tar.gz" \
    && tar -xf otcommon.tar.gz \
    && mv "otcommon-${OTCOMMON_COMMIT_HASH}" /usr/src/otcommon

FROM download AS opentxs-download
ARG OPENTXS_REPO
ARG OPENTXS_COMMIT
RUN mkdir -p /usr/src && git clone --recursive "${OPENTXS_REPO}" /usr/src/opentxs && cd /usr/src/opentxs && git reset --hard "${OPENTXS_COMMIT}"
