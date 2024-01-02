variable "ci_39_revision" { default = "6" }
variable "ci_38_revision" { default = "12" }
variable "ci_37_revision" { default = "15" }
variable "android_revision" { default = "28" }

variable "OT_DOCKER_ARCH" {
  default = "amd64"
}
variable "OT_VERSION" {
  default = "develop"
}
variable "OTCOMMON_COMMIT_HASH" {
  default = "a8cb2a10e7eff4fdece28feea681541b70728053"
}
variable "OTCOMMON_VERSION" {
  default = "2.0.0-0-ga8cb2a1"
}
variable "OPENTXS_REPO" {
  default = "https://github.com/open-transactions/opentxs"
}
variable "JOBS" {
  default = "2"
}

variable "BOOST_MAJOR" {
  default = "1"
}

variable "BOOST_MINOR" {
  default = "84"
}

variable "BOOST_PATCH" {
  default = "0"
}

variable "ANDROID_TOOLS" {
  default = "8092744"
}

variable "ANDROID_BUILD_TOOLS" {
  default = "33.0.0"
}

variable "ANDROID_LEVEL" {
  default = "33"
}

variable "ANDROID_LEVEL_TOOLCHAIN" {
  default = "33"
}

variable "NDK_VERSION" {
  default = "26.0.10792818"
}

target "downloader" {
  dockerfile = "common/downloader"
  target = "downloader"
  tags = [
    "temp-opentxs/downloader"
  ]
}

target "cmake-download" {
  dockerfile = "common/download"
  target = "cmake-download"
  contexts = {
    download = "target:downloader"
  }
  args = {
    OTCOMMON_COMMIT_HASH = "${OTCOMMON_COMMIT_HASH}"
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
    OPENTXS_REPO = "${OPENTXS_REPO}"
    OPENTXS_COMMIT = "${OT_VERSION}"
  }
  tags = [
    "temp-opentxs/cmake-download"
  ]
}

target "boost-download" {
  dockerfile = "common/download"
  target = "boost-download"
  contexts = {
    download = "target:downloader"
  }
  args = {
    OTCOMMON_COMMIT_HASH = "${OTCOMMON_COMMIT_HASH}"
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
    OPENTXS_REPO = "${OPENTXS_REPO}"
    OPENTXS_COMMIT = "${OT_VERSION}"
    BOOST_MAJOR = "${BOOST_MAJOR}"
    BOOST_MINOR = "${BOOST_MINOR}"
    BOOST_PATCH = "${BOOST_PATCH}"
  }
  tags = [
    "temp-opentxs/boost-download"
  ]
}

target "otcommon-download" {
  dockerfile = "common/download"
  target = "otcommon-download"
  contexts = {
    download = "target:downloader"
  }
  args = {
    OTCOMMON_COMMIT_HASH = "${OTCOMMON_COMMIT_HASH}"
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
    OPENTXS_REPO = "${OPENTXS_REPO}"
    OPENTXS_COMMIT = "${OT_VERSION}"
  }
  tags = [
    "temp-opentxs/otcommon-download"
  ]
}

target "simpleini-download" {
  dockerfile = "common/download"
  target = "simpleini-download"
  contexts = {
    download = "target:downloader"
  }
  tags = [
    "temp-opentxs/simpleini-download"
  ]
}

target "libguarded-download" {
  dockerfile = "common/download"
  target = "libguarded-download"
  contexts = {
    download = "target:downloader"
  }
  tags = [
    "temp-opentxs/libguarded-download"
  ]
}

group "ci" {
  targets = ["ci_39", "ci_38", "ci_37"]
}

# Fedora 39 CI

target "iwyu-download-39" {
  dockerfile = "common/iwyu-download"
  target = "iwyu-download"
  contexts = {
    download = "target:downloader"
  }
  args = {
    IWYU_COMMIT_HASH = "dbecade3b575678cc3ccc4dd2d0942e8ff3c7527"
  }
  tags = [
    "temp-opentxs/iwyu-download-39"
  ]
}

target "ci-base-39" {
  dockerfile = "runtime/fedora.base"
  target = "base"
  args = {
    BASE_FEDORA_VERSION = "39"
  }
  tags = [
    "temp-opentxs/ci-base-39"
  ]
}

target "ci-baseline-39" {
  dockerfile = "runtime/fedora.build"
  target = "build"
  contexts = {
    base = "target:ci-base-39"
    simpleini-download = "target:simpleini-download"
  }
  tags = [
    "temp-opentxs/ci-baseline-39"
  ]
}

target "ci-cmake-bootstrap-39" {
  dockerfile = "runtime/fedora.cmake-bootstrap"
  target = "cmake-bootstrap"
  contexts = {
    build = "target:ci-baseline-39"
  }
  tags = [
    "temp-opentxs/ci-cmake-bootstrap-39"
  ]
}

target "ci-build-39" {
  dockerfile = "ci/build"
  target = "build"
  contexts = {
    baseline = "target:ci-baseline-39"
  }
  tags = [
    "temp-opentxs/ci-build-39"
  ]
}

target "ci-cmake-39" {
  dockerfile = "common/cmake"
  target = "cmake"
  contexts = {
    cmake-bootstrap = "target:ci-cmake-bootstrap-39"
    cmake-download = "target:cmake-download"
  }
  tags = [
    "temp-opentxs/ci-cmake-39"
  ]
}

target "ci-iwyu-bootstrap-39" {
  dockerfile = "ci/iwyu-bootstrap"
  target = "iwyu-bootstrap"
  contexts = {
    build = "target:ci-build-39"
    cmake = "target:ci-cmake-39"
  }
  tags = [
    "temp-opentxs/ci-iwyu-bootstrap-39"
  ]
}

target "ci-boost-39" {
  dockerfile = "common/boost"
  target = "boost"
  contexts = {
    cmake-bootstrap = "target:ci-cmake-bootstrap-39"
    boost-download = "target:boost-download"
  }
  tags = [
    "temp-opentxs/ci-boost-39"
  ]
}

target "ci-otcommon-39" {
  dockerfile = "common/otcommon"
  target = "otcommon"
  contexts = {
    build = "target:ci-baseline-39"
    cmake = "target:ci-cmake-39"
    otcommon-download = "target:otcommon-download"
  }
  args = {
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
  }
  tags = [
    "temp-opentxs/ci-otcommon-39"
  ]
}

target "ci-libguarded-39" {
  dockerfile = "common/libguarded"
  target = "libguarded"
  contexts = {
    build = "target:ci-baseline-39"
    cmake = "target:ci-cmake-39"
    libguarded-download = "target:libguarded-download"
  }
  tags = [
    "temp-opentxs/ci-libguarded-39"
  ]
}

target "ci-iwyu-39" {
  dockerfile = "common/iwyu"
  target = "iwyu"
  contexts = {
    iwyu-bootstrap = "target:ci-iwyu-bootstrap-39"
    iwyu-download = "target:iwyu-download-39"
  }
  tags = [
    "temp-opentxs/ci-iwyu-39"
  ]
}

target "ci_39" {
  dockerfile = "ci/Dockerfile"
  target = "sdk"
  args = {
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
  }
  contexts = {
    build = "target:ci-build-39"
    cmake = "target:ci-cmake-39"
    boost = "target:ci-boost-39"
    otcommon = "target:ci-otcommon-39"
    libguarded = "target:ci-libguarded-39"
    iwyu = "target:ci-iwyu-39"
    files = "ci/"
  }
  tags = [
    "opentransactions/ci:39_${ci_39_revision}-${OT_DOCKER_ARCH}",
    "opentransactions/ci:latest-${OT_DOCKER_ARCH}"
  ]
}

# Fedora 38 CI

target "iwyu-download-38" {
  dockerfile = "common/iwyu-download"
  target = "iwyu-download"
  contexts = {
    download = "target:downloader"
  }
  args = {
    IWYU_COMMIT_HASH = "35fed15e53d92c8c540f0c00ac10077043126c4d"
  }
  tags = [
    "temp-opentxs/iwyu-download-38"
  ]
}

target "ci-base-38" {
  dockerfile = "runtime/fedora.base"
  target = "base"
  args = {
    BASE_FEDORA_VERSION = "38"
  }
  tags = [
    "temp-opentxs/ci-base-38"
  ]
}

target "ci-baseline-38" {
  dockerfile = "runtime/fedora.build"
  target = "build"
  contexts = {
    base = "target:ci-base-38"
    simpleini-download = "target:simpleini-download"
  }
  tags = [
    "temp-opentxs/ci-baseline-38"
  ]
}

target "ci-cmake-bootstrap-38" {
  dockerfile = "runtime/fedora.cmake-bootstrap"
  target = "cmake-bootstrap"
  contexts = {
    build = "target:ci-baseline-38"
  }
  tags = [
    "temp-opentxs/ci-cmake-bootstrap-38"
  ]
}

target "ci-build-38" {
  dockerfile = "ci/build"
  target = "build"
  contexts = {
    baseline = "target:ci-baseline-38"
  }
  tags = [
    "temp-opentxs/ci-build-38"
  ]
}

target "ci-cmake-38" {
  dockerfile = "common/cmake"
  target = "cmake"
  contexts = {
    cmake-bootstrap = "target:ci-cmake-bootstrap-38"
    cmake-download = "target:cmake-download"
  }
  tags = [
    "temp-opentxs/ci-cmake-38"
  ]
}

target "ci-iwyu-bootstrap-38" {
  dockerfile = "ci/iwyu-bootstrap"
  target = "iwyu-bootstrap"
  contexts = {
    build = "target:ci-build-38"
    cmake = "target:ci-cmake-38"
  }
  tags = [
    "temp-opentxs/ci-iwyu-bootstrap-38"
  ]
}

target "ci-boost-38" {
  dockerfile = "common/boost"
  target = "boost"
  contexts = {
    cmake-bootstrap = "target:ci-cmake-bootstrap-38"
    boost-download = "target:boost-download"
  }
  tags = [
    "temp-opentxs/ci-boost-38"
  ]
}

target "ci-otcommon-38" {
  dockerfile = "common/otcommon"
  target = "otcommon"
  contexts = {
    build = "target:ci-baseline-38"
    cmake = "target:ci-cmake-38"
    otcommon-download = "target:otcommon-download"
  }
  args = {
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
  }
  tags = [
    "temp-opentxs/ci-otcommon-38"
  ]
}

target "ci-libguarded-38" {
  dockerfile = "common/libguarded"
  target = "libguarded"
  contexts = {
    build = "target:ci-baseline-38"
    cmake = "target:ci-cmake-38"
    libguarded-download = "target:libguarded-download"
  }
  tags = [
    "temp-opentxs/ci-libguarded-38"
  ]
}

target "ci-iwyu-38" {
  dockerfile = "common/iwyu"
  target = "iwyu"
  contexts = {
    iwyu-bootstrap = "target:ci-iwyu-bootstrap-38"
    iwyu-download = "target:iwyu-download-38"
  }
  tags = [
    "temp-opentxs/ci-iwyu-38"
  ]
}

target "ci_38" {
  dockerfile = "ci/Dockerfile"
  target = "sdk"
  args = {
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
  }
  contexts = {
    build = "target:ci-build-38"
    cmake = "target:ci-cmake-38"
    boost = "target:ci-boost-38"
    otcommon = "target:ci-otcommon-38"
    libguarded = "target:ci-libguarded-38"
    iwyu = "target:ci-iwyu-38"
    files = "ci/"
  }
  tags = [
    "opentransactions/ci:38_${ci_38_revision}-${OT_DOCKER_ARCH}"
  ]
}

# Fedora 37 CI

target "iwyu-download-37" {
  dockerfile = "common/iwyu-download"
  target = "iwyu-download"
  contexts = {
    download = "target:downloader"
  }
  args = {
    IWYU_COMMIT_HASH = "7f0b6c304acf69c42bb7f6e03c63f836924cb7e0"
  }
  tags = [
    "temp-opentxs/iwyu-download-37"
  ]
}

target "ci-base-37" {
  dockerfile = "runtime/fedora.base"
  target = "base"
  args = {
    BASE_FEDORA_VERSION = "37"
  }
  tags = [
    "temp-opentxs/ci-base-37"
  ]
}

target "ci-baseline-37" {
  dockerfile = "runtime/fedora.build"
  target = "build"
  contexts = {
    base = "target:ci-base-37"
    simpleini-download = "target:simpleini-download"
  }
  tags = [
    "temp-opentxs/ci-baseline-37"
  ]
}

target "ci-cmake-bootstrap-37" {
  dockerfile = "runtime/fedora.cmake-bootstrap"
  target = "cmake-bootstrap"
  contexts = {
    build = "target:ci-baseline-37"
  }
  tags = [
    "temp-opentxs/ci-cmake-bootstrap-37"
  ]
}

target "ci-build-37" {
  dockerfile = "ci/build"
  target = "build"
  contexts = {
    baseline = "target:ci-baseline-37"
  }
  tags = [
    "temp-opentxs/ci-build-37"
  ]
}

target "ci-cmake-37" {
  dockerfile = "common/cmake"
  target = "cmake"
  contexts = {
    cmake-bootstrap = "target:ci-cmake-bootstrap-37"
    cmake-download = "target:cmake-download"
  }
  tags = [
    "temp-opentxs/ci-cmake-37"
  ]
}

target "ci-iwyu-bootstrap-37" {
  dockerfile = "ci/iwyu-bootstrap"
  target = "iwyu-bootstrap"
  contexts = {
    build = "target:ci-build-37"
    cmake = "target:ci-cmake-37"
  }
  tags = [
    "temp-opentxs/ci-iwyu-bootstrap-37"
  ]
}

target "ci-boost-37" {
  dockerfile = "common/boost"
  target = "boost"
  contexts = {
    cmake-bootstrap = "target:ci-cmake-bootstrap-37"
    boost-download = "target:boost-download"
  }
  tags = [
    "temp-opentxs/ci-boost-37"
  ]
}

target "ci-otcommon-37" {
  dockerfile = "common/otcommon"
  target = "otcommon"
  contexts = {
    build = "target:ci-baseline-37"
    cmake = "target:ci-cmake-37"
    otcommon-download = "target:otcommon-download"
  }
  args = {
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
  }
  tags = [
    "temp-opentxs/ci-otcommon-37"
  ]
}

target "ci-libguarded-37" {
  dockerfile = "common/libguarded"
  target = "libguarded"
  contexts = {
    build = "target:ci-baseline-37"
    cmake = "target:ci-cmake-37"
    libguarded-download = "target:libguarded-download"
  }
  tags = [
    "temp-opentxs/ci-libguarded-37"
  ]
}

target "ci-iwyu-37" {
  dockerfile = "common/iwyu"
  target = "iwyu"
  contexts = {
    iwyu-bootstrap = "target:ci-iwyu-bootstrap-37"
    iwyu-download = "target:iwyu-download-37"
  }
  tags = [
    "temp-opentxs/ci-iwyu-37"
  ]
}

target "ci_37" {
  dockerfile = "ci/Dockerfile"
  target = "sdk"
  args = {
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
  }
  contexts = {
    build = "target:ci-build-37"
    cmake = "target:ci-cmake-37"
    boost = "target:ci-boost-37"
    otcommon = "target:ci-otcommon-37"
    libguarded = "target:ci-libguarded-37"
    iwyu = "target:ci-iwyu-37"
    files = "ci/"
  }
  tags = [
    "opentransactions/ci:37_${ci_37_revision}-${OT_DOCKER_ARCH}"
  ]
}

group "runtimes" {
  targets = ["opentxs-fedora", "opentxs-ubuntu", "opentxs-alpine"]
}

target "opentxs-download" {
  dockerfile = "common/download"
  target = "opentxs-download"
  contexts = {
    download = "target:downloader"
  }
  args = {
    OTCOMMON_COMMIT_HASH = "${OTCOMMON_COMMIT_HASH}"
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
    OPENTXS_REPO = "${OPENTXS_REPO}"
    OPENTXS_COMMIT = "${OT_VERSION}"
  }
  tags = [
    "temp-opentxs/opentxs-download"
  ]
}

# Fedora

group "opentxs-fedora" {
  targets = ["fedora-runtime", "fedora-sdk", "downstream-ci"]
}

target "fedora-base" {
  dockerfile = "runtime/fedora.base"
  target = "base"
  tags = [
    "temp-opentxs/fedora-base"
  ]
}

# NOTE simpleini-devel not available in fedora 37 or earlier
target "fedora-build" {
  dockerfile = "runtime/fedora.build"
  target = "build"
  contexts = {
    base = "target:fedora-base"
    simpleini-download = "target:simpleini-download"
  }
  tags = [
    "temp-opentxs/fedora-build"
  ]
}

target "fedora-run" {
  dockerfile = "runtime/fedora.run"
  target = "run"
  contexts = {
    base = "target:fedora-base"
  }
  tags = [
    "temp-opentxs/fedora-run"
  ]
}

target "fedora-cmake-bootstrap" {
  dockerfile = "runtime/fedora.cmake-bootstrap"
  target = "cmake-bootstrap"
  contexts = {
    build = "target:fedora-build"
  }
  tags = [
    "temp-opentxs/fedora-cmake-bootstrap"
  ]
}

target "fedora-cmake" {
  dockerfile = "common/cmake"
  target = "cmake"
  contexts = {
    cmake-download = "target:cmake-download"
    cmake-bootstrap = "target:fedora-cmake-bootstrap"
  }
  tags = [
    "temp-opentxs/fedora-cmake"
  ]
}

target "fedora-boost" {
  dockerfile = "common/boost"
  target = "boost"
  contexts = {
    cmake-bootstrap = "target:fedora-cmake-bootstrap"
    boost-download = "target:boost-download"
  }
  tags = [
    "temp-opentxs/fedora-boost"
  ]
}

target "fedora-otcommon" {
  dockerfile = "common/otcommon"
  target = "otcommon"
  contexts = {
    build = "target:fedora-build"
    cmake = "target:fedora-cmake"
    otcommon-download = "target:otcommon-download"
  }
  args = {
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
  }
  tags = [
    "temp-opentxs/fedora-otcommon"
  ]
}

target "fedora-libguarded" {
  dockerfile = "common/libguarded"
  target = "libguarded"
  contexts = {
    build = "target:fedora-build"
    cmake = "target:fedora-cmake"
    libguarded-download = "target:libguarded-download"
  }
  tags = [
    "temp-opentxs/fedora-libguarded"
  ]
}

target "fedora-opentxs" {
  dockerfile = "common/opentxs"
  target = "opentxs"
  contexts = {
    build = "target:fedora-build"
    cmake = "target:fedora-cmake"
    boost = "target:fedora-boost"
    otcommon = "target:fedora-otcommon"
    opentxs-download = "target:opentxs-download"
    libguarded = "target:fedora-libguarded"
  }
  args = {
    OT_CMAKE_ARGS = "-DOT_WITH_QT=OFF -DOT_WITH_QML=OFF"
  }
  tags = [
    "temp-opentxs/fedora-opentxs"
  ]
}

target "fedora-runtime" {
  dockerfile = "runtime/runtime"
  target = "runtime"
  contexts = {
    build = "target:fedora-build"
    run = "target:fedora-run"
    cmake = "target:fedora-cmake"
    boost = "target:fedora-boost"
    otcommon = "target:fedora-otcommon"
    opentxs = "target:fedora-opentxs"
  }
  args = {
    OT_LIB_DIR = "lib64"
  }
  tags = [
    "opentransactions/fedora-runtime:${OT_VERSION}-${OT_DOCKER_ARCH}",
    "opentransactions/fedora-runtime:latest-${OT_DOCKER_ARCH}"
  ]
}

target "fedora-sdk" {
  dockerfile = "runtime/sdk"
  target = "sdk"
  contexts = {
    build = "target:fedora-build"
    run = "target:fedora-run"
    cmake = "target:fedora-cmake"
    boost = "target:fedora-boost"
    otcommon = "target:fedora-otcommon"
    opentxs = "target:fedora-opentxs"
    libguarded = "target:fedora-libguarded"
  }
  args = {
    OT_LIB_DIR = "lib64"
  }
  tags = [
    "opentransactions/fedora-sdk:${OT_VERSION}-${OT_DOCKER_ARCH}",
    "opentransactions/fedora-sdk:latest-${OT_DOCKER_ARCH}"
  ]
}

# downstream-ci

target "downstream-opentxs" {
  dockerfile = "common/opentxs"
  target = "opentxs"
  contexts = {
    build = "target:ci-build-39"
    cmake = "target:ci-cmake-39"
    boost = "target:ci-boost-39"
    otcommon = "target:ci-otcommon-39"
    libguarded = "target:ci-libguarded-39"
    opentxs-download = "target:opentxs-download"
  }
  args = {
    OT_CMAKE_ARGS = ""
  }
  tags = [
    "temp-opentxs/downstream-opentxs"
  ]
}

target "downstream-ci" {
  dockerfile = "ci/downstream"
  target = "sdk"
  contexts = {
    ci = "target:ci_39"
    opentxs = "target:downstream-opentxs"
  }
  tags = [
    "opentransactions/downstream-ci:${OT_VERSION}-${OT_DOCKER_ARCH}",
    "opentransactions/downstream-ci:latest-${OT_DOCKER_ARCH}"
  ]
}

# Ubuntu

group "opentxs-ubuntu" {
  targets = ["ubuntu-runtime", "ubuntu-sdk"]
}

target "ubuntu-base" {
  dockerfile = "runtime/ubuntu.base"
  target = "base"
  tags = [
    "temp-opentxs/ubuntu-base"
  ]
}

target "ubuntu-build" {
  dockerfile = "runtime/ubuntu.build"
  target = "build"
  contexts = {
    base = "target:ubuntu-base"
  }
  tags = [
    "temp-opentxs/ubuntu-build"
  ]
}

target "ubuntu-run" {
  dockerfile = "runtime/ubuntu.run"
  target = "run"
  contexts = {
    base = "target:ubuntu-base"
  }
  tags = [
    "temp-opentxs/ubuntu-run"
  ]
}

target "ubuntu-cmake-bootstrap" {
  dockerfile = "runtime/ubuntu.cmake-bootstrap"
  target = "cmake-bootstrap"
  contexts = {
    build = "target:ubuntu-build"
  }
  tags = [
    "temp-opentxs/ubuntu-cmake-bootstrap"
  ]
}

target "ubuntu-cmake" {
  dockerfile = "common/cmake"
  target = "cmake"
  contexts = {
    cmake-download = "target:cmake-download"
    cmake-bootstrap = "target:ubuntu-cmake-bootstrap"
  }
  tags = [
    "temp-opentxs/ubuntu-cmake"
  ]
}

target "ubuntu-boost" {
  dockerfile = "common/boost"
  target = "boost"
  contexts = {
    cmake-bootstrap = "target:ubuntu-cmake-bootstrap"
    boost-download = "target:boost-download"
  }
  tags = [
    "temp-opentxs/ubuntu-boost"
  ]
}

target "ubuntu-otcommon" {
  dockerfile = "common/otcommon"
  target = "otcommon"
  contexts = {
    build = "target:ubuntu-build"
    cmake = "target:ubuntu-cmake"
    otcommon-download = "target:otcommon-download"
  }
  args = {
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
  }
  tags = [
    "temp-opentxs/ubuntu-otcommon"
  ]
}

target "ubuntu-libguarded" {
  dockerfile = "common/libguarded"
  target = "libguarded"
  contexts = {
    build = "target:ubuntu-build"
    cmake = "target:ubuntu-cmake"
    libguarded-download = "target:libguarded-download"
  }
  tags = [
    "temp-opentxs/ubuntu-libguarded"
  ]
}

target "ubuntu-opentxs" {
  dockerfile = "common/opentxs"
  target = "opentxs"
  contexts = {
    build = "target:ubuntu-build"
    cmake = "target:ubuntu-cmake"
    boost = "target:ubuntu-boost"
    otcommon = "target:ubuntu-otcommon"
    opentxs-download = "target:opentxs-download"
    libguarded = "target:ubuntu-libguarded"
  }
  args = {
    OT_CMAKE_ARGS = "-DOT_WITH_QT=OFF -DOT_WITH_QML=OFF"
  }
  tags = [
    "temp-opentxs/ubuntu-opentxs"
  ]
}

target "ubuntu-runtime" {
  dockerfile = "runtime/runtime"
  target = "runtime"
  contexts = {
    build = "target:ubuntu-build"
    run = "target:ubuntu-run"
    cmake = "target:ubuntu-cmake"
    boost = "target:ubuntu-boost"
    otcommon = "target:ubuntu-otcommon"
    opentxs = "target:ubuntu-opentxs"
  }
  args = {
    OT_LIB_DIR = "lib"
  }
  tags = [
    "opentransactions/ubuntu-runtime:${OT_VERSION}-${OT_DOCKER_ARCH}",
    "opentransactions/ubuntu-runtime:latest-${OT_DOCKER_ARCH}"
  ]
}

target "ubuntu-sdk" {
  dockerfile = "runtime/sdk"
  target = "sdk"
  contexts = {
    build = "target:ubuntu-build"
    run = "target:ubuntu-run"
    cmake = "target:ubuntu-cmake"
    boost = "target:ubuntu-boost"
    otcommon = "target:ubuntu-otcommon"
    opentxs = "target:ubuntu-opentxs"
    libguarded = "target:ubuntu-libguarded"
  }
  args = {
    OT_LIB_DIR = "lib"
  }
  tags = [
    "opentransactions/ubuntu-sdk:${OT_VERSION}-${OT_DOCKER_ARCH}",
    "opentransactions/ubuntu-sdk:latest-${OT_DOCKER_ARCH}"
  ]
}

# Alpine

group "opentxs-alpine" {
  targets = ["alpine-runtime", "alpine-sdk"]
}

target "alpine-base" {
  dockerfile = "runtime/alpine.base"
  target = "base"
  tags = [
    "temp-opentxs/alpine-base"
  ]
}

target "alpine-build" {
  dockerfile = "runtime/alpine.build"
  target = "build"
  contexts = {
    base = "target:alpine-base"
    simpleini-download = "target:simpleini-download"
  }
  tags = [
    "temp-opentxs/alpine-build"
  ]
}

target "alpine-run" {
  dockerfile = "runtime/alpine.run"
  target = "run"
  contexts = {
    base = "target:alpine-base"
  }
  tags = [
    "temp-opentxs/alpine-run"
  ]
}

target "alpine-cmake-bootstrap" {
  dockerfile = "runtime/alpine.cmake-bootstrap"
  target = "cmake-bootstrap"
  contexts = {
    build = "target:alpine-build"
  }
  tags = [
    "temp-opentxs/alpine-cmake-bootstrap"
  ]
}

target "alpine-cmake" {
  dockerfile = "common/cmake"
  target = "cmake"
  contexts = {
    cmake-download = "target:cmake-download"
    cmake-bootstrap = "target:alpine-cmake-bootstrap"
  }
  tags = [
    "temp-opentxs/alpine-cmake"
  ]
}

target "alpine-boost" {
  dockerfile = "common/boost"
  target = "boost"
  contexts = {
    cmake-bootstrap = "target:alpine-cmake-bootstrap"
    boost-download = "target:boost-download"
  }
  tags = [
    "temp-opentxs/alpine-boost"
  ]
}

target "alpine-otcommon" {
  dockerfile = "common/otcommon"
  target = "otcommon"
  contexts = {
    build = "target:alpine-build"
    cmake = "target:alpine-cmake"
    otcommon-download = "target:otcommon-download"
  }
  args = {
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
  }
  tags = [
    "temp-opentxs/alpine-otcommon"
  ]
}

target "alpine-libguarded" {
  dockerfile = "common/libguarded"
  target = "libguarded"
  contexts = {
    build = "target:alpine-build"
    cmake = "target:alpine-cmake"
    libguarded-download = "target:libguarded-download"
  }
  tags = [
    "temp-opentxs/alpine-libguarded"
  ]
}

target "alpine-opentxs" {
  dockerfile = "common/opentxs"
  target = "opentxs"
  contexts = {
    build = "target:alpine-build"
    cmake = "target:alpine-cmake"
    boost = "target:alpine-boost"
    otcommon = "target:alpine-otcommon"
    opentxs-download = "target:opentxs-download"
    libguarded = "target:alpine-libguarded"
  }
  args = {
    OT_CMAKE_ARGS = "-DOT_WITH_QT=OFF -DOT_WITH_QML=OFF"
  }
  tags = [
    "temp-opentxs/alpine-opentxs"
  ]
}

target "alpine-runtime" {
  dockerfile = "runtime/runtime"
  target = "runtime"
  contexts = {
    build = "target:alpine-build"
    run = "target:alpine-run"
    cmake = "target:alpine-cmake"
    boost = "target:alpine-boost"
    otcommon = "target:alpine-otcommon"
    opentxs = "target:alpine-opentxs"
  }
  args = {
    OT_LIB_DIR = "lib"
  }
  tags = [
    "opentransactions/alpine-runtime:${OT_VERSION}-${OT_DOCKER_ARCH}",
    "opentransactions/alpine-runtime:latest-${OT_DOCKER_ARCH}"
  ]
}

target "alpine-sdk" {
  dockerfile = "runtime/sdk"
  target = "sdk"
  contexts = {
    build = "target:alpine-build"
    run = "target:alpine-run"
    cmake = "target:alpine-cmake"
    boost = "target:alpine-boost"
    otcommon = "target:alpine-otcommon"
    opentxs = "target:alpine-opentxs"
    libguarded = "target:alpine-libguarded"
  }
  args = {
    OT_LIB_DIR = "lib"
  }
  tags = [
    "opentransactions/alpine-sdk:${OT_VERSION}-${OT_DOCKER_ARCH}",
    "opentransactions/alpine-sdk:latest-${OT_DOCKER_ARCH}"
  ]
}

# Android

target "android-run" {
  dockerfile = "android/run"
  target = "run"
  tags = [
    "temp-opentxs/android-run"
  ]
}

target "android-tools" {
  dockerfile = "android/tools"
  target = "tools"
  contexts = {
    run = "target:android-run"
  }
  tags = [
    "temp-opentxs/android-tools"
  ]
}

target "android-cmake-bootstrap" {
  dockerfile = "android/cmake-bootstrap"
  target = "cmake-bootstrap"
  contexts = {
    tools = "target:android-tools"
  }
  tags = [
    "temp-opentxs/android-cmake-bootstrap"
  ]
}

target "android-android" {
  dockerfile = "android/android"
  target = "android"
  contexts = {
    tools = "target:android-tools"
  }
  args = {
    ANDROID_TOOLS = "${ANDROID_TOOLS}"
    ANDROID_LEVEL = "${ANDROID_LEVEL}"
    ANDROID_BUILD_TOOLS = "${ANDROID_BUILD_TOOLS}"
    NDK_VERSION = "${NDK_VERSION}"
  }
  tags = [
    "temp-opentxs/android-android"
  ]
}

target "openssl-download" {
  dockerfile = "common/download"
  target = "openssl-download"
  contexts = {
    download = "target:downloader"
  }
  tags = [
    "temp-opentxs/openssl-download"
  ]
}

target "android-openssl-host" {
  dockerfile = "common/openssl"
  target = "openssl"
  contexts = {
    openssl-bootstrap = "target:android-tools"
    openssl-download = "target:openssl-download"
  }
  args = {
    JOBS = "${JOBS}"
  }
  tags = [
    "temp-opentxs/android-openssl-host"
  ]
}

target "android-cmake" {
  dockerfile = "common/cmake-openssl"
  target = "cmake"
  contexts = {
    openssl = "target:android-openssl-host"
    cmake-bootstrap = "target:android-cmake-bootstrap"
    cmake-download = "target:cmake-download"
  }
  tags = [
    "temp-opentxs/android-cmake"
  ]
}

target "android-build-base" {
  dockerfile = "android/build-base"
  target = "build-base"
  contexts = {
    tools = "target:android-tools"
    android = "target:android-android"
    cmake = "target:android-cmake"
    openssl_host = "target:android-openssl-host"
  }
  args = {
    NDK_VERSION = "${NDK_VERSION}"
  }
  tags = [
    "temp-opentxs/android-build-base"
  ]
}

target "boostforandroid-download" {
  dockerfile = "common/download"
  target = "boostforandroid-download"
  contexts = {
    download = "target:downloader"
  }
  args = {
    BOOST_MAJOR = "${BOOST_MAJOR}"
    BOOST_MINOR = "${BOOST_MINOR}"
    BOOST_PATCH = "${BOOST_PATCH}"
  }
  tags = [
    "temp-opentxs/boostforandroid-download"
  ]
}

target "android-boost" {
  dockerfile = "android/boost"
  target = "boost"
  contexts = {
    build-base = "target:android-build-base"
    boostforandroid-download = "target:boostforandroid-download"
    files = "android/"
  }
  args = {
    ANDROID_LEVEL = "${ANDROID_LEVEL}"
    BOOST_MAJOR = "${BOOST_MAJOR}"
    BOOST_MINOR = "${BOOST_MINOR}"
    BOOST_PATCH = "${BOOST_PATCH}"
  }
  tags = [
    "temp-opentxs/android-boost"
  ]
}

target "sodium-download" {
  dockerfile = "common/download"
  target = "sodium-download"
  contexts = {
    download = "target:downloader"
  }
  tags = [
    "temp-opentxs/sodium-download"
  ]
}

target "android-sodium" {
  dockerfile = "android/sodium"
  target = "sodium"
  contexts = {
    build-base = "target:android-build-base"
    sodium-download = "target:sodium-download"
  }
  args = {
    ANDROID_LEVEL_TOOLCHAIN = "${ANDROID_LEVEL_TOOLCHAIN}"
    JOBS = "${JOBS}"
  }
  tags = [
    "temp-opentxs/android-sodium"
  ]
}

target "secp256k1-download" {
  dockerfile = "common/download"
  target = "secp256k1-download"
  contexts = {
    download = "target:downloader"
  }
  tags = [
    "temp-opentxs/secp256k1-download"
  ]
}

target "android-secp256k1" {
  dockerfile = "android/secp256k1"
  target = "secp256k1"
  contexts = {
    build-base = "target:android-build-base"
    secp256k1-download = "target:secp256k1-download"
  }
  args = {
    ANDROID_LEVEL = "${ANDROID_LEVEL}"
  }
  tags = [
    "temp-opentxs/android-secp256k1"
  ]
}

target "lmdb-download" {
  dockerfile = "common/download"
  target = "lmdb-download"
  contexts = {
    download = "target:downloader"
  }
  tags = [
    "temp-opentxs/lmdb-download"
  ]
}

target "android-lmdb" {
  dockerfile = "android/lmdb"
  target = "lmdb"
  contexts = {
    build-base = "target:android-build-base"
    lmdb-download = "target:lmdb-download"
  }
  args = {
    ANDROID_LEVEL_TOOLCHAIN = "${ANDROID_LEVEL_TOOLCHAIN}"
    JOBS = "${JOBS}"
  }
  tags = [
    "temp-opentxs/android-lmdb"
  ]
}

target "protobuf-download" {
  dockerfile = "common/download"
  target = "protobuf-download"
  contexts = {
    download = "target:downloader"
  }
  tags = [
    "temp-opentxs/protobuf-download"
  ]
}

target "android-protobuf" {
  dockerfile = "android/protobuf"
  target = "protobuf"
  contexts = {
    build-base = "target:android-build-base"
    protobuf-download = "target:protobuf-download"
  }
  args = {
    ANDROID_LEVEL = "${ANDROID_LEVEL}"
    JOBS = "${JOBS}"
  }
  tags = [
    "temp-opentxs/android-protobuf"
  ]
}

target "android-openssl" {
  dockerfile = "android/openssl"
  target = "openssl"
  contexts = {
    build-base = "target:android-build-base"
    openssl-download = "target:openssl-download"
  }
  args = {
    JOBS = "${JOBS}"
    ANDROID_LEVEL = "${ANDROID_LEVEL}"
  }
  tags = [
    "temp-opentxs/android-openssl"
  ]
}

target "zeromq-download" {
  dockerfile = "common/download"
  target = "zeromq-download"
  contexts = {
    download = "target:downloader"
  }
  tags = [
    "temp-opentxs/zeromq-download"
  ]
}

target "android-zeromq" {
  dockerfile = "android/zeromq"
  target = "zeromq"
  contexts = {
    build-base = "target:android-build-base"
    zeromq-download = "target:zeromq-download"
    sodium = "target:android-sodium"
  }
  args = {
    ANDROID_LEVEL = "${ANDROID_LEVEL}"
  }
  tags = [
    "temp-opentxs/android-zeromq"
  ]
}

target "gtest-download" {
  dockerfile = "common/download"
  target = "gtest-download"
  contexts = {
    download = "target:downloader"
  }
  tags = [
    "temp-opentxs/gtest-download"
  ]
}

target "android-gtest" {
  dockerfile = "android/gtest"
  target = "gtest"
  contexts = {
    build-base = "target:android-build-base"
    gtest-download = "target:gtest-download"
    sodium = "target:android-sodium"
  }
  args = {
    ANDROID_LEVEL = "${ANDROID_LEVEL}"
  }
  tags = [
    "temp-opentxs/android-gtest"
  ]
}

target "android-otcommon" {
  dockerfile = "common/otcommon"
  target = "otcommon"
  contexts = {
    build = "target:android-build-base"
    cmake = "target:android-cmake"
    otcommon-download = "target:otcommon-download"
  }
  args = {
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
  }
  tags = [
    "temp-opentxs/android-otcommon"
  ]
}

target "android-libguarded" {
  dockerfile = "common/libguarded"
  target = "libguarded"
  contexts = {
    build = "target:android-build-base"
    cmake = "target:android-cmake"
    libguarded-download = "target:libguarded-download"
  }
  tags = [
    "temp-opentxs/android-libguarded"
  ]
}

target "android-qt-bootstrap" {
  dockerfile = "android/qt-bootstrap"
  target = "qt-bootstrap"
  contexts = {
    build-base = "target:android-build-base"
  }
  tags = [
    "temp-opentxs/android-qt-bootstrap"
  ]
}

target "qt-download" {
  dockerfile = "common/download"
  target = "qt-download"
  contexts = {
    download = "target:downloader"
  }
  tags = [
    "temp-opentxs/qt-download"
  ]
}

target "android-qt-host" {
  dockerfile = "android/qt-host"
  target = "qt-host"
  contexts = {
    qt-bootstrap = "target:android-qt-bootstrap"
    qt-download = "target:qt-download"
  }
  tags = [
    "temp-opentxs/android-qt-host"
  ]
}

target "android-qt" {
  dockerfile = "android/qt"
  target = "qt"
  contexts = {
    qt-host = "target:android-qt-host"
  }
  args = {
    ANDROID_LEVEL_TOOLCHAIN = "${ANDROID_LEVEL_TOOLCHAIN}"
  }
  tags = [
    "temp-opentxs/android-qt"
  ]
}

target "android" {
  dockerfile = "android/Dockerfile"
  target = "final"
  contexts = {
    run = "target:android-run"
    android = "target:android-android"
    qt-host = "target:android-qt-host"
    qt = "target:android-qt"
    boost = "target:android-boost"
    sodium = "target:android-sodium"
    secp256k1 = "target:android-secp256k1"
    lmdb = "target:android-lmdb"
    protobuf = "target:android-protobuf"
    openssl = "target:android-openssl"
    zeromq = "target:android-zeromq"
    gtest = "target:android-gtest"
    cmake = "target:android-cmake"
    otcommon = "target:android-otcommon"
    simpleini-download = "target:simpleini-download"
    libguarded = "target:android-libguarded"
    files = "android/"
  }
  args = {
    NDK_VERSION = "${NDK_VERSION}"
  }
  tags = [
    "opentransactions/android:${android_revision}",
    "opentransactions/android:latest"
  ]
}
