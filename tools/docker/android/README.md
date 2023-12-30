# Open-Transactions Android Build Image

This image creates an Android NDK development environment and compiles all opentxs dependencies. A build script is included which will compile opentxs for a specified architecture and bindings.

## Contents

* Android SDK
  * Platform (android-33)
  * Android NDK (26.0.10792818)
* Boost (1.84)
* Googletest (1.14.0)
* LMDB (8d0cbbc936091eb85972501a9b31a8f86d4c51a7)
* OpenSSL (3.1.4)
* otcommon (2.0.0)
* Libsodium (1.0.18)
* Protocol Buffers (23.4)
* Qt (6.6.1)
* secp256k1 (0.4.1)
* ZeroMQ (4.3.4)

## Usage

### Building the image

Run the following command in the parent directory to this README.md file:

```
docker buildx bake android
```

This image is available on Docker Hub as [opentransactions/android](https://hub.docker.com/r/opentransactions/android)


### Compiling opentxs

Valid architectures (first argument, mandatory): arm64 arm x64 x86

Valid bindings (second argument, optional): none qt all


#### Example

```
docker run \
    --read-only \
    --tmpfs /tmp/build:rw,nosuid,size=2g \
    --mount type=bind,src=/path/to/opentxs,dst=/home/src \
    --mount type=bind,src=/desired/output/directory,dst=/home/output \
    -it opentransactions/android:latest \
    arm64 \
    all
```
