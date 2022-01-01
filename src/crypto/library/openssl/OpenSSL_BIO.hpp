// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

extern "C" {
#include <openssl/bio.h>
#include <openssl/ossl_typ.h>
}

#include <cstddef>
#include <vector>

#include "opentxs/core/String.hpp"

namespace opentxs::crypto::implementation
{
class OpenSSL_BIO
{
private:
    BIO& m_refBIO;
    bool bCleanup;
    bool bFreeOnly;

    static auto assertBioNotNull(BIO* pBIO) -> BIO*;

    void read_bio(
        const std::size_t amount,
        std::size_t& read,
        std::size_t& total,
        std::pmr::vector<std::byte>& output);

public:
    OpenSSL_BIO(BIO* pBIO);

    ~OpenSSL_BIO();

    operator BIO*() const;

    void release();
    void setFreeOnly();

    auto ToBytes() -> std::pmr::vector<std::byte>;
    auto ToString() -> OTString;
};
}  // namespace opentxs::crypto::implementation
