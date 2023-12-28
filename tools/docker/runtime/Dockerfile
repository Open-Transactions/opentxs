FROM cmake-bootstrap AS cmake
COPY --from=cmake-download /usr/src/cmake /usr/src/cmake
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && /usr/bin/cmake \
        -GNinja \
        -DCMAKE_INSTALL_PREFIX=/opt/cmake \
        /usr/src/cmake \
    && /usr/bin/cmake --build . \
    && /usr/bin/cmake --install .

FROM cmake-bootstrap AS boost
COPY --from=boost-download /usr/src/boost /usr/src/boost
RUN --mount=type=tmpfs,target=/tmp/build rsync -a /usr/src/boost/ /tmp/build/ \
    && cd /tmp/build \
    && ./bootstrap.sh --prefix=/opt/boost --with-libraries=all \
    && ./b2 \
    && ./b2 --prefix=/opt/boost install

FROM build AS otcommon
COPY --from=cmake /opt/cmake /opt/cmake
ARG OTCOMMON_VERSION
COPY --from=otcommon-download /usr/src/otcommon /usr/src/otcommon
RUN --mount=type=tmpfs,target=/tmp/build cd /tmp/build \
    && /opt/cmake/bin/cmake \
        -GNinja \
        -DCMAKE_INSTALL_PREFIX=/opt/otcommon \
        -Dotcommon_GIT_VERSION=${OTCOMMON_VERSION} \
        /usr/src/otcommon \
    && /opt/cmake/bin/cmake --install .

FROM build AS compile
ENV BOOST_ROOT=/opt/boost
ENV OTCOMMON_ROOT=/opt/otcommon
COPY --from=cmake /opt/cmake /opt/cmake
COPY --from=boost /opt/boost /opt/boost
COPY --from=otcommon /opt/otcommon /opt/otcommon
COPY --from=opentxs-download /usr/src/opentxs /usr/src/opentxs
ARG OT_CMAKE_ARGS
RUN --mount=type=tmpfs,target=/tmp/opentxs /opt/cmake/bin/cmake \
        -S /usr/src/opentxs \
        -B /tmp/opentxs \
        --preset prod \
        -DCMAKE_INSTALL_PREFIX=/opt/opentxs \
        -DBUILD_SHARED_LIBS=ON \
        -DOPENTXS_PEDANTIC_BUILD=OFF \
        -DCMAKE_C_FLAGS="-O2" \
        -DCMAKE_CXX_FLAGS="-O2" \
        -DOT_STORAGE_SQLITE=OFF \
        -DOT_BUNDLED_SECP256K1=OFF \
        ${OT_CMAKE_ARGS} \
    && /opt/cmake/bin/cmake --build /tmp/opentxs \
    && /opt/cmake/bin/cmake --install /tmp/opentxs

FROM run AS runtime
ARG OT_LIB_DIR
ENV LD_LIBRARY_PATH="/opt/boost/lib:/opt/opentxs/${OT_LIB_DIR}"
ENV OPENTXS_ROOT=/opt/opentxs
COPY --from=boost /opt/boost/lib /opt/boost/lib
COPY --from=compile /opt/opentxs/${OT_LIB_DIR} /opt/opentxs/${OT_LIB_DIR}

FROM build AS sdk
ENV BOOST_ROOT=/opt/boost
ENV OTCOMMON_ROOT=/opt/otcommon
ENV OPENTXS_ROOT=/opt/opentxs
COPY --from=cmake /opt/cmake /opt/cmake
COPY --from=otcommon /opt/otcommon /opt/otcommon
COPY --from=boost /opt/boost /opt/boost
COPY --from=compile /opt/opentxs /opt/opentxs
