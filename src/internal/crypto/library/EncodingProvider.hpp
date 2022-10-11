// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::crypto
{
class EncodingProvider
{
public:
    virtual UnallocatedCString Base58CheckEncode(
        const std::uint8_t* inputStart,
        const std::size_t& inputSize) const = 0;
    virtual bool Base58CheckDecode(
        const UnallocatedCString&& input,
        RawData& output) const = 0;

    virtual ~EncodingProvider() = default;

protected:
    EncodingProvider() = default;

private:
    EncodingProvider(const EncodingProvider&) = delete;
    EncodingProvider(EncodingProvider&&) = delete;
    EncodingProvider& operator=(const EncodingProvider&) = delete;
    EncodingProvider& operator=(EncodingProvider&&) = delete;
};
}  // namespace opentxs::crypto
