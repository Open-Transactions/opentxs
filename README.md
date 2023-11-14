## Open-Transactions Library Project

[![License: MPL 2.0](https://img.shields.io/badge/License-MPL%202.0-brightgreen.svg)](https://opensource.org/licenses/MPL-2.0)

[![Android](https://github.com/open-transactions/opentxs/workflows/Android/badge.svg)](https://github.com/Open-Transactions/opentxs/actions)
[![Linux](https://github.com/open-transactions/opentxs/workflows/linux/badge.svg)](https://github.com/Open-Transactions/opentxs/actions)
[![MacOS / Windows](https://github.com/open-transactions/opentxs/workflows/vcpkg/badge.svg)](https://github.com/Open-Transactions/opentxs/actions)

The Open-Transactions project is a collaborative effort to develop a robust,
commercial-grade, fully-featured, free-software toolkit implementing the OTX
protocol as well as a full-strength financial cryptography library, API, CLI,
and prototype server. The project is managed by a worldwide community of
volunteers that use the Internet to communicate, plan, and develop the
Open-Transactions toolkit and its related documentation.

### Official Wiki

http://opentransactions.org/

### About

Open-Transactions democratizes financial and monetary actions. You can use it
for issuing currencies/stock, paying dividends, creating asset accounts,
sending/receiving digital cash, writing/depositing cheques, cashier's cheques,
creating basket currencies, trading on markets, scripting custom agreements,
recurring payments, escrow, etc.

Open-Transactions uses strong crypto. The balances are unchangeable (even by a
malicious server.) The receipts are destructible and redundant. The transactions
are unforgeable. The cash is untraceable. The cheques are non-repudiable. Etc.

This product includes software developed by Ben Laurie for use in the Lucre
project.

### Contributing

Configuration files for clang-format and cmake-format are available in
[otcommon](https://github.com/Open-Transactions/otcommon).

### Build Instructions

Ensure opentxs is fully checked out prior to running cmake:

    git submodule update --init --recursive

Install dependencies, including [otcommon](https://github.com/Open-Transactions/otcommon) or use vcpkg.

Basic build instructions:

    mkdir build
    cd build
    cmake -GNinja -DBUILD_SHARED_LIBS=ON ..
    cmake --build .
    ctest -j4
    sudo cmake --install .

#### Minimum Supported Compiler Versions

- gcc-12.2
- clang-16
- xcode 15
- msvc-19.33
- ndk r26
