set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)

# https://github.com/microsoft/vcpkg/issues/14943#issuecomment-738877304
set(VCPKG_OSX_DEPLOYMENT_TARGET 10.15)
