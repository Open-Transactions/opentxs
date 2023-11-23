# Open-Transactions Application Base Images (Fedora version)

These images provide Fedora-based development and runtime environments to be used by applications which use libopentxs.

## Usage

### Building the image

To create the development image:

```
docker image build --build-arg "OPENTXS_COMMIT=<tag or commit hash>" --target opentxs-devel .

```

This image is available on Docker Hub as [opentransactions/fedora-devel
](https://hub.docker.com/r/opentransactions/fedora-devel)


To create the runtime image:

```
docker image build --build-arg "OPENTXS_COMMIT=<tag or commit hash>" --target opentxs .
```

This image is available on Docker Hub as [opentransactions/fedora](https://hub.docker.com/r/opentransactions/fedora)

### Using the images

When compiling an application, use [opentransactions/fedora-devel
](https://hub.docker.com/r/opentransactions/fedora-devel). All required header files, cmake files, and libraries are installed in /usr.

When executing an application, use [opentransactions/fedora](https://hub.docker.com/r/opentransactions/fedora). This provides all the necessary libraries in /usr without any headers or build-only dependencies.
