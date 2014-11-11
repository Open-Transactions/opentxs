## Open-Transactions Library Project

[![Stories in Ready](https://badge.waffle.io/open-transactions/opentxs.svg?label=ready&title=Ready)](http://waffle.io/open-transactions/opentxs) 
[![License](http://img.shields.io/:License-LAGPL-yellow.svg)](docs/LICENSE-AND-CREDITS.txt)


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

#### CppCheck and clang-format Git hooks

For convenience please enable the git hooks which will trigger cppcheck and
clang-format each time you push or commit. To do so type in the repo directory:

    cd .git/hooks
    ln -s ../../scripts/git_hooks/pre-push
    ln -s ../../scripts/git_hooks/pre-commit

To check your code without pushing the following command can be used:

    git push -n

### Build Instructions

OpenTransactions uses the CMake build system. The basic steps are

    mkdir build
    cd build
    cmake ..
    make
    make install

#### Docker

You can get started with opentxs cli very quickly with a docker container, once you have docker installed on your system, just run the following commands;

	cd in your repo dir
	cd opentxs
	docker build --rm=true .
	docker run -i -t <image ID> /bin/bash 

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
