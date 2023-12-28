variable "OT_DOCKER_ARCH" { }
variable "OT_VERSION" { }
variable "OTCOMMON_COMMIT_HASH" {
  default = "a8cb2a10e7eff4fdece28feea681541b70728053"
}
variable "OTCOMMON_VERSION" {
  default = "2.0.0-0-ga8cb2a1"
}
variable "OPENTXS_REPO" {
  default = "https://github.com/open-transactions/opentxs"
}

group "runtimes" {
  targets = ["opentxs-fedora", "opentxs-ubuntu", "opentxs-alpine"]
}

target "cmake-download" {
  dockerfile = "../common/Dockerfile"
  target = "cmake-download"
  args = {
    OTCOMMON_COMMIT_HASH = "${OTCOMMON_COMMIT_HASH}"
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
    OPENTXS_REPO = "${OPENTXS_REPO}"
    OPENTXS_COMMIT = "${OT_VERSION}"
  }
}

target "iwyu-download" {
  dockerfile = "../common/Dockerfile"
  target = "iwyu-download"
  args = {
    OTCOMMON_COMMIT_HASH = "${OTCOMMON_COMMIT_HASH}"
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
    OPENTXS_REPO = "${OPENTXS_REPO}"
    OPENTXS_COMMIT = "${OT_VERSION}"
  }
}

target "boost-download" {
  dockerfile = "../common/Dockerfile"
  target = "boost-download"
  args = {
    OTCOMMON_COMMIT_HASH = "${OTCOMMON_COMMIT_HASH}"
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
    OPENTXS_REPO = "${OPENTXS_REPO}"
    OPENTXS_COMMIT = "${OT_VERSION}"
  }
}

target "otcommon-download" {
  dockerfile = "../common/Dockerfile"
  target = "otcommon-download"
  args = {
    OTCOMMON_COMMIT_HASH = "${OTCOMMON_COMMIT_HASH}"
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
    OPENTXS_REPO = "${OPENTXS_REPO}"
    OPENTXS_COMMIT = "${OT_VERSION}"
  }
}

target "opentxs-download" {
  dockerfile = "../common/Dockerfile"
  target = "opentxs-download"
  args = {
    OTCOMMON_COMMIT_HASH = "${OTCOMMON_COMMIT_HASH}"
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
    OPENTXS_REPO = "${OPENTXS_REPO}"
    OPENTXS_COMMIT = "${OT_VERSION}"
  }
}

# Fedora

group "opentxs-fedora" {
  targets = ["fedora-runtime", "fedora-sdk"]
}

target "fedora-build" {
  dockerfile = "fedora.baseline"
  target = "build"
}

target "fedora-run" {
  dockerfile = "fedora.baseline"
  target = "run"
}

target "fedora-cmake-bootstrap" {
  dockerfile = "fedora.baseline"
  target = "cmake-bootstrap"
}

target "fedora-cmake" {
  dockerfile = "Dockerfile"
  target = "cmake"
  contexts = {
    cmake-download = "target:cmake-download"
    cmake-bootstrap = "target:fedora-cmake-bootstrap"
  }
}

target "fedora-base-runtime" {
  dockerfile = "Dockerfile"
  target = "runtime"
  contexts = {
    cmake-download = "target:cmake-download"
    boost-download = "target:boost-download"
    otcommon-download = "target:otcommon-download"
    opentxs-download = "target:opentxs-download"
    build = "target:fedora-build"
    run = "target:fedora-run"
    cmake-bootstrap = "target:fedora-cmake-bootstrap"
  }
  args = {
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
    OT_LIB_DIR = "lib64"
    OT_CMAKE_ARGS = ""
  }
}

target "fedora-base-sdk" {
  dockerfile = "Dockerfile"
  target = "sdk"
  contexts = {
    cmake-download = "target:cmake-download"
    boost-download = "target:boost-download"
    otcommon-download = "target:otcommon-download"
    opentxs-download = "target:opentxs-download"
    build = "target:fedora-build"
    run = "target:fedora-run"
    cmake-bootstrap = "target:fedora-cmake-bootstrap"
  }
  args = {
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
    OT_LIB_DIR = "lib64"
    OT_CMAKE_ARGS = ""
  }
}

target "fedora-runtime" {
  dockerfile = "fedora.final"
  target = "runtime"
  contexts = {
    iwyu-download = "target:iwyu-download"
    cmake = "target:fedora-cmake"
    build = "target:fedora-build"
    base-runtime = "target:fedora-base-runtime"
    base-sdk = "target:fedora-base-sdk"
  }
  tags = [
    "opentransactions/fedora-runtime:${OT_VERSION}-${OT_DOCKER_ARCH}",
    "opentransactions/fedora-runtime:latest-${OT_DOCKER_ARCH}"
  ]
}

target "fedora-sdk" {
  dockerfile = "fedora.final"
  target = "sdk"
  contexts = {
    iwyu-download = "target:iwyu-download"
    cmake = "target:fedora-cmake"
    build = "target:fedora-build"
    base-runtime = "target:fedora-base-runtime"
    base-sdk = "target:fedora-base-sdk"
  }
  tags = [
    "opentransactions/fedora-runtime:${OT_VERSION}-${OT_DOCKER_ARCH}",
    "opentransactions/fedora-runtime:latest-${OT_DOCKER_ARCH}"
  ]
}

# Ubuntu

group "opentxs-ubuntu" {
  targets = ["ubuntu-runtime", "ubuntu-sdk"]
}

target "ubuntu-build" {
  dockerfile = "ubuntu.baseline"
  target = "build"
}

target "ubuntu-run" {
  dockerfile = "ubuntu.baseline"
  target = "run"
}

target "ubuntu-cmake-bootstrap" {
  dockerfile = "ubuntu.baseline"
  target = "cmake-bootstrap"
}

target "ubuntu-cmake" {
  dockerfile = "Dockerfile"
  target = "cmake"
  contexts = {
    cmake-download = "target:cmake-download"
    cmake-bootstrap = "target:ubuntu-cmake-bootstrap"
  }
}

target "ubuntu-base-runtime" {
  dockerfile = "Dockerfile"
  target = "runtime"
  contexts = {
    cmake-download = "target:cmake-download"
    boost-download = "target:boost-download"
    otcommon-download = "target:otcommon-download"
    opentxs-download = "target:opentxs-download"
    build = "target:ubuntu-build"
    run = "target:ubuntu-run"
    cmake-bootstrap = "target:ubuntu-cmake-bootstrap"
  }
  args = {
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
    OT_LIB_DIR = "lib"
    OT_CMAKE_ARGS = "-DOT_WITH_QT=OFF -DOT_WITH_QML=OFF"
  }
}

target "ubuntu-base-sdk" {
  dockerfile = "Dockerfile"
  target = "sdk"
  contexts = {
    cmake-download = "target:cmake-download"
    boost-download = "target:boost-download"
    otcommon-download = "target:otcommon-download"
    opentxs-download = "target:opentxs-download"
    build = "target:ubuntu-build"
    run = "target:ubuntu-run"
    cmake-bootstrap = "target:ubuntu-cmake-bootstrap"
  }
  args = {
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
    OT_LIB_DIR = "lib"
    OT_CMAKE_ARGS = "-DOT_WITH_QT=OFF -DOT_WITH_QML=OFF"
  }
}

target "ubuntu-runtime" {
  dockerfile = "ubuntu.final"
  target = "runtime"
  contexts = {
    iwyu-download = "target:iwyu-download"
    cmake = "target:ubuntu-cmake"
    build = "target:ubuntu-build"
    base-runtime = "target:ubuntu-base-runtime"
    base-sdk = "target:ubuntu-base-sdk"
  }
  tags = [
    "opentransactions/ubuntu-runtime:${OT_VERSION}-${OT_DOCKER_ARCH}",
    "opentransactions/ubuntu-runtime:latest-${OT_DOCKER_ARCH}"
  ]
}

target "ubuntu-sdk" {
  dockerfile = "ubuntu.final"
  target = "sdk"
  contexts = {
    iwyu-download = "target:iwyu-download"
    cmake = "target:ubuntu-cmake"
    build = "target:ubuntu-build"
    base-runtime = "target:ubuntu-base-runtime"
    base-sdk = "target:ubuntu-base-sdk"
  }
  tags = [
    "opentransactions/ubuntu-runtime:${OT_VERSION}-${OT_DOCKER_ARCH}",
    "opentransactions/ubuntu-runtime:latest-${OT_DOCKER_ARCH}"
  ]
}

# Alpine

group "opentxs-alpine" {
  targets = ["alpine-runtime", "alpine-sdk"]
}

target "alpine-build" {
  dockerfile = "alpine.baseline"
  target = "build"
}

target "alpine-run" {
  dockerfile = "alpine.baseline"
  target = "run"
}

target "alpine-cmake-bootstrap" {
  dockerfile = "alpine.baseline"
  target = "cmake-bootstrap"
}

target "alpine-cmake" {
  dockerfile = "Dockerfile"
  target = "cmake"
  contexts = {
    cmake-download = "target:cmake-download"
    cmake-bootstrap = "target:alpine-cmake-bootstrap"
  }
}

target "alpine-base-runtime" {
  dockerfile = "Dockerfile"
  target = "runtime"
  contexts = {
    cmake-download = "target:cmake-download"
    boost-download = "target:boost-download"
    otcommon-download = "target:otcommon-download"
    opentxs-download = "target:opentxs-download"
    build = "target:alpine-build"
    run = "target:alpine-run"
    cmake-bootstrap = "target:alpine-cmake-bootstrap"
  }
  args = {
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
    OT_LIB_DIR = "lib"
    OT_CMAKE_ARGS = "-DOT_WITH_QT=OFF -DOT_WITH_QML=OFF -DOT_USE_PSTL=OFF -DOT_WITH_TBB=OFF"
  }
}

target "alpine-base-sdk" {
  dockerfile = "Dockerfile"
  target = "sdk"
  contexts = {
    cmake-download = "target:cmake-download"
    boost-download = "target:boost-download"
    otcommon-download = "target:otcommon-download"
    opentxs-download = "target:opentxs-download"
    build = "target:alpine-build"
    run = "target:alpine-run"
    cmake-bootstrap = "target:alpine-cmake-bootstrap"
  }
  args = {
    OTCOMMON_VERSION = "${OTCOMMON_VERSION}"
    OT_LIB_DIR = "lib"
    OT_CMAKE_ARGS = "-DOT_WITH_QT=OFF -DOT_WITH_QML=OFF -DOT_USE_PSTL=OFF -DOT_WITH_TBB=OFF"
  }
}

target "alpine-runtime" {
  dockerfile = "alpine.final"
  target = "runtime"
  contexts = {
    iwyu-download = "target:iwyu-download"
    cmake = "target:alpine-cmake"
    build = "target:alpine-build"
    base-runtime = "target:alpine-base-runtime"
    base-sdk = "target:alpine-base-sdk"
  }
  tags = [
    "opentransactions/alpine-runtime:${OT_VERSION}-${OT_DOCKER_ARCH}",
    "opentransactions/alpine-runtime:latest-${OT_DOCKER_ARCH}"
  ]
}

target "alpine-sdk" {
  dockerfile = "alpine.final"
  target = "sdk"
  contexts = {
    iwyu-download = "target:iwyu-download"
    cmake = "target:alpine-cmake"
    build = "target:alpine-build"
    base-runtime = "target:alpine-base-runtime"
    base-sdk = "target:alpine-base-sdk"
  }
  tags = [
    "opentransactions/alpine-runtime:${OT_VERSION}-${OT_DOCKER_ARCH}",
    "opentransactions/alpine-runtime:latest-${OT_DOCKER_ARCH}"
  ]
}
