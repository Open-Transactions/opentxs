// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/library/openssl/BIO.hpp"  // IWYU pragma: associated

extern "C" {
#include <openssl/bio.h>
}

#include <cstddef>
#include <cstdint>
#include <limits>

#include "internal/core/String.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::crypto::openssl
{
auto BIO::assertBioNotNull(::BIO* pBIO) -> ::BIO*
{
    if (nullptr == pBIO) { LogAbort()().Abort(); }
    return pBIO;
}

BIO::BIO(::BIO* pBIO)
    : bio_(*assertBioNotNull(pBIO))
    , b_cleanup_(true)
    , b_free_only_(false)
{
}

BIO::~BIO()
{
    if (b_cleanup_) {
        if (b_free_only_) {
            BIO_free(&bio_);
        } else {
            BIO_free_all(&bio_);
        }
    }
}

BIO::operator ::BIO*() const { return (&bio_); }

void BIO::release() { b_cleanup_ = false; }

void BIO::setFreeOnly() { b_free_only_ = true; }

void BIO::read_bio(
    const std::size_t amount,
    std::size_t& read,
    std::size_t& total,
    UnallocatedVector<std::byte>& output)
{
    assert_true(std::numeric_limits<int>::max() >= amount);

    output.resize(output.size() + amount);
    read = BIO_read(*this, &output[total], static_cast<int>(amount));
    total += read;
}

auto BIO::ToBytes() -> UnallocatedVector<std::byte>
{
    std::size_t read{0};
    std::size_t total{0};
    UnallocatedVector<std::byte> output{};
    read_bio(read_amount_, read, total, output);

    if (0 == read) {
        LogError()()("Read failed").Flush();

        return {};
    }

    while (read_amount_ == read) {
        read_bio(read_amount_, read, total, output);
    }

    output.resize(total);
    LogInsane()()("Read ")(total)(" bytes").Flush();

    return output;
}

auto BIO::ToString() -> OTString
{
    auto output = String::Factory();
    auto bytes = ToBytes();
    const auto size = bytes.size();

    if (0 < size) {
        bytes.resize(size + 1);
        bytes[size] = static_cast<std::byte>(0x0);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-type-limit-compare"
        assert_true(std::numeric_limits<std::uint32_t>::max() >= bytes.size());
#pragma GCC diagnostic pop

        output->Set(
            reinterpret_cast<const char*>(bytes.data()),
            static_cast<std::uint32_t>(bytes.size()));
    }

    return output;
}
}  // namespace opentxs::crypto::openssl
