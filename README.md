## Open-Transactions Library Project

[![Stories in Ready](https://badge.waffle.io/open-transactions/opentxs.svg?label=ready&title=Ready)](http://waffle.io/open-transactions/opentxs)
[![License](http://img.shields.io/:License-MPLv2-yellow.svg)](LICENSE)


The Open-Transactions project is a collaborative effort to develop
a robust, commercial-grade, fully-featured, free-software toolkit
implementing the OTX protocol as well as a full-strength financial
cryptography library, API, CLI, and prototype server. The project
is managed by a worldwide community of volunteers that use the
Internet to communicate, plan, and develop the Open-Transactions
toolkit and its related documentation.

### Official Wiki

http://opentransactions.org/

### About

Open-Transactions democratizes financial and monetary actions. You
can use it for issuing currencies/stock, paying dividends, creating
asset accounts, sending/receiving digital cash, writing/depositing
cheques, cashier's cheques, creating basket currencies, trading on
markets, scripting custom agreements, recurring payments, escrow,
etc.

Open-Transactions uses strong crypto. The balances are unchangeable
(even by a malicious server.) The receipts are destructible and
redundant. The transactions are unforgeable. The cash is untraceable.
The cheques are non-repudiable. Etc.

This product includes software developed by Ben Laurie for use in
the Lucre project.

### Contributing

All development goes in develop branch - please don't submit pull requests to
master.

Please do *NOT* use an editor that automatically reformats.

As part of our Continuous Integration system
we run [cppcheck](https://github.com/danmar/cppcheck/) and
[clang-format](http://clang.llvm.org/docs/ClangFormat.html). The build will fail
if either of them finds problems.

#### Running the tests

Run `make test`, after `make`. Optionally to see test logging: `make test args -V`. To output the information of failed tests: `env CTEST_OUTPUT_ON_FAILURE=1 make test`

Tests may run in parallel passing `-j` to make.

BE ADVISED: The OT directory in `~/.ot` is deleted on every `make test` in the `build` directory. Run `make test` in *development only*.
  
#### CppCheck and clang-format Git hooks

For convenience please enable the git hooks which will trigger cppcheck and
clang-format each time you push or commit. To do so type in the repo directory:

    cd .git/hooks
    ln -s ../../scripts/git_hooks/pre-push
    ln -s ../../scripts/git_hooks/pre-commit

To check your code without pushing the following command can be used:

    git push -n
### Dependencies

* [opentxs-proto](https://github.com/open-transactions/opentxs-proto)

### Optional Features

* SQLite driver for new storage engine
  * Default: enabled
  * Adds dependency: [SQLite 3](https://www.sqlite.org)
  * CMake symbol: OT_STORAGE_SQLITE
* Filesystem driver for new storage engine
  * Default: disabled
  * Adds dependency: [Boost::Filesystem](http://www.boost.org)
  * CMake symbol: OT_STORAGE_FS

* OpenDHT network driver
  * Default: enabled
  * Adds dependency: [OpenDHT](https://github.com/savoirfairelinux/opendht)
  * CMake symbol: OT_DHT

### Build Instructions

OpenTransactions uses the CMake build system. The basic steps are

    mkdir build
    cd build
    cmake ..
    make
    make install

More detailed instructions are listed below

#### Docker

You can get started with opentxs cli and build setup quickly. It is possible to build this in a docker container, once you have docker installed on your system, just run the following commands;

        cd in your git dir
        cd opentxs
        docker build --rm=true .
        docker run -i -t <image ID> /bin/bash

Don't forget to "Detach" correctly or you will lose the state of the container. e.g Ctrl + P, I

More detailed instructions are listed below

#### Linux

 * [Generic Linux](docs/INSTALL-MEMO-Linux.txt)
 * [Debian and Ubuntu](docs/INSTALL-Debian_Ubuntu.txt)
 * [Fedora](docs/INSTALL-Fedora.txt)
 * [OpenSUSE](docs/INSTALL-openSUSE.txt)

#### OSX

 * [OSX Homebrew](docs/INSTALL-OSX-Homebrew.txt)

#### Windows

 * [Windows (Vista SP2+)](docs/INSTALL-Windows.txt)
