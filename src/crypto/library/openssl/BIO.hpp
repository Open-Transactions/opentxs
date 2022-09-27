// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

extern "C" {
#include <openssl/bio.h>
#if __has_include(<openssl/types.h>)
// TODO openssl-3
#include <openssl/types.h>  // IWYU pragma: keep
#elif __has_include(<openssl/ossl_typ.h>)
#include <openssl/ossl_typ.h>  // IWYU pragma: keep
#endif
}

#include <cstddef>

#include "internal/util/P0330.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::crypto::openssl
{
class BIO
{
private:
    static constexpr auto read_amount_ = 256_uz;

    ::BIO& bio_;
    bool b_cleanup_;
    bool b_free_only_;

    static auto assertBioNotNull(::BIO* pBIO) -> ::BIO*;

    void read_bio(
        const std::size_t amount,
        std::size_t& read,
        std::size_t& total,
        UnallocatedVector<std::byte>& output);

public:
    BIO(::BIO* pBIO);

    ~BIO();

    operator ::BIO*() const;

    void release();
    void setFreeOnly();

    auto ToBytes() -> UnallocatedVector<std::byte>;
    auto ToString() -> OTString;
};
}  // namespace opentxs::crypto::openssl
