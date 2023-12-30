FROM run AS runtime
ARG OT_LIB_DIR
ENV LD_LIBRARY_PATH="/opt/boost/lib:/opt/opentxs/${OT_LIB_DIR}"
ENV OPENTXS_ROOT=/opt/opentxs
COPY --from=boost /opt/boost/lib /opt/boost/lib
COPY --from=opentxs /opt/opentxs/${OT_LIB_DIR} /opt/opentxs/${OT_LIB_DIR}

FROM build AS sdk
ENV BOOST_ROOT=/opt/boost
ENV OTCOMMON_ROOT=/opt/otcommon
ENV OPENTXS_ROOT=/opt/opentxs
RUN git config --global --add safe.directory '*'
COPY --from=cmake /opt/cmake /opt/cmake
COPY --from=otcommon /opt/otcommon /opt/otcommon
COPY --from=boost /opt/boost /opt/boost
COPY --from=opentxs /opt/opentxs /opt/opentxs
