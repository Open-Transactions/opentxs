FROM run AS final
ARG NDK_VERSION
COPY --from=android /opt/android-sdk-linux/ndk /opt/android-sdk-linux/ndk
RUN ln -s "/opt/android-sdk-linux/ndk/${NDK_VERSION}" /opt/ndk
COPY --from=qt-host /opt/qt /opt/qt
COPY --from=qt /usr/qt-for-android /usr/qt-for-android
COPY --from=boost /usr/android /usr/android
COPY --from=sodium /usr/android /usr/android
COPY --from=secp256k1 /usr/android /usr/android
COPY --from=lmdb /usr/android /usr/android
COPY --from=protobuf /usr/android /usr/android
COPY --from=protobuf /opt/protobuf /opt/protobuf
COPY --from=openssl /usr/android /usr/android
COPY --from=zeromq /usr/android /usr/android
COPY --from=gtest /usr/android /usr/android
COPY --from=cmake /opt/cmake /opt/cmake
COPY --from=otcommon /opt/otcommon /opt/otcommon
COPY --from=simpleini-download /usr/src/simpleini/SimpleIni.h /usr/android/arm/include/
COPY --from=simpleini-download /usr/src/simpleini/SimpleIni.h /usr/android/arm64/include/
COPY --from=simpleini-download /usr/src/simpleini/SimpleIni.h /usr/android/x64/include/
COPY --from=simpleini-download /usr/src/simpleini/SimpleIni.h /usr/android/x86/include/
COPY --from=files build /usr/bin
ENTRYPOINT [ "/usr/bin/build" ]
CMD []
