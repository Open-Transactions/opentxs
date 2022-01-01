// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
#include "1_Internal.hpp"                          // IWYU pragma: associated
#include "crypto/library/openssl/OpenSSL_BIO.hpp"  // IWYU pragma: associated

extern "C" {
#include <openssl/bio.h>
}

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

#include "internal/util/LogMacros.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

#define READ_AMOUNT 256

namespace opentxs::crypto::implementation
{
// OpenSSL_BIO
auto OpenSSL_BIO::assertBioNotNull(BIO* pBIO) -> BIO*
{
    if (nullptr == pBIO) OT_FAIL;
    return pBIO;
}

OpenSSL_BIO::OpenSSL_BIO(BIO* pBIO)
    : m_refBIO(*assertBioNotNull(pBIO))
    , bCleanup(true)
    , bFreeOnly(false)
{
}

OpenSSL_BIO::~OpenSSL_BIO()
{
    if (bCleanup) {
        if (bFreeOnly) {
            BIO_free(&m_refBIO);
        } else {
            BIO_free_all(&m_refBIO);
        }
    }
}

OpenSSL_BIO::operator BIO*() const { return (&m_refBIO); }

void OpenSSL_BIO::release() { bCleanup = false; }

void OpenSSL_BIO::setFreeOnly() { bFreeOnly = true; }

void OpenSSL_BIO::read_bio(
    const std::size_t amount,
    std::size_t& read,
    std::size_t& total,
    std::pmr::vector<std::byte>& output)
{
    OT_ASSERT(std::numeric_limits<int>::max() >= amount);

    output.resize(output.size() + amount);
    read = BIO_read(*this, &output[total], static_cast<int>(amount));
    total += read;
}

auto OpenSSL_BIO::ToBytes() -> std::pmr::vector<std::byte>
{
    std::size_t read{0};
    std::size_t total{0};
    std::pmr::vector<std::byte> output{};
    read_bio(READ_AMOUNT, read, total, output);

    if (0 == read) {
        LogError()(OT_PRETTY_CLASS())("Read failed").Flush();

        return {};
    }

    while (READ_AMOUNT == read) { read_bio(READ_AMOUNT, read, total, output); }

    output.resize(total);
    LogInsane()(OT_PRETTY_CLASS())("Read ")(total)(" bytes").Flush();

    return output;
}

auto OpenSSL_BIO::ToString() -> OTString
{
    auto output = String::Factory();
    auto bytes = ToBytes();
    const auto size = bytes.size();

    if (0 < size) {
        bytes.resize(size + 1);
        bytes[size] = static_cast<std::byte>(0x0);

        OT_ASSERT(std::numeric_limits<std::uint32_t>::max() >= bytes.size());

        output->Set(
            reinterpret_cast<const char*>(bytes.data()),
            static_cast<std::uint32_t>(bytes.size()));
    }

    return output;
}
}  // namespace opentxs::crypto::implementation
