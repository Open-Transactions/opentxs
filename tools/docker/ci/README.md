# Open-Transactions Docker image for Continuous Integration

These images construct build environments for compiling and testing Open-Transactions

## Usage

### Building the image

Run the following command in the parent directory to this README.md file:

```
docker buildx bake ci
```

### Compiling opentxs

The script for compiling opentxs accepts three mandatory arguments.

The first argument specifies the path inside the image where the source tree is mounted. Formerly this was hardcoded to /home/src.

The second argument specifies the compiler. The allowed values are: gcc clang

The third argument specifies a preset from CMakePresets.json

#### Example

```
docker run \
    --read-only \
    --mount readonly,type=bind,src=/path/to/opentxs,dst=/home/src \
    --mount type=bind,src=/path/to/build/directory,dst=/home/output \
    -it opentransactions/ci:latest \
    /home/src \
    gcc \
    full
```

### Check formatting

The script for formatting opentxs accepts one mandatory argument.

The argument specifies the path inside the image where the source tree is mounted. Formerly this was hardcoded to /home/src.

The source tree must be mounted as writable.

#### Example

```
docker run \
    --read-only \
    --mount type=bind,src=/path/to/opentxs,dst=/home/src \
    --entrypoint /usr/bin/check-formatting.sh \
    -it opentransactions/ci:latest \
    /home/src
```

### Check includes

The script for normalizing includes accepts two mandatory arguments

The first argument specifies the path inside the image where the source tree is mounted. Formerly this was hardcoded to /home/src.

The second argument specifies a preset from CMakePresets.json.

The source tree must be mounted as writable.

#### Example

```
docker run \
    --read-only \
    --mount type=bind,src=/path/to/opentxs,dst=/home/src \
    --mount type=bind,src=/path/to/build/directory,dst=/home/output \
    --entrypoint /usr/bin/run-iwyu.sh \
    -it opentransactions/ci:latest \
    /home/src \
    iwyu
```
