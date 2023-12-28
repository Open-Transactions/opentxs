variable "OT_DOCKER_ARCH" { }

group "ci" {
  targets = ["ci_39", "ci_38", "ci_37"]
}

target "ci_39" {
  dockerfile = "Dockerfile"
  args = {
    BASE_FEDORA_VERSION = "39",
    IWYU_COMMIT_HASH= "dbecade3b575678cc3ccc4dd2d0942e8ff3c7527"
  }
  tags = [
    "opentransactions/ci:39_4-${OT_DOCKER_ARCH}",
    "opentransactions/ci:latest-${OT_DOCKER_ARCH}"
  ]
}

target "ci_38" {
  dockerfile = "Dockerfile"
  args = {
    BASE_FEDORA_VERSION = "38",
    IWYU_COMMIT_HASH= "35fed15e53d92c8c540f0c00ac10077043126c4d"
  }
  tags = ["opentransactions/ci:38_10-${OT_DOCKER_ARCH}"]
}

target "ci_37" {
  dockerfile = "Dockerfile"
  args = {
    BASE_FEDORA_VERSION = "37",
    IWYU_COMMIT_HASH= "7f0b6c304acf69c42bb7f6e03c63f836924cb7e0"
  }
  tags = ["opentransactions/ci:37_13-${OT_DOCKER_ARCH}"]
}
