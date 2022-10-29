// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace ot = opentxs;

namespace ottest
{
struct OPENTXS_EXPORT ArgonVector {
    std::uint32_t iterations_{};
    std::uint32_t memory_{};
    std::uint32_t threads_{};
    std::string_view input_{};
    std::string_view salt_{};
    std::string_view expected_{};
};

struct OPENTXS_EXPORT HMACVector {
    std::string_view key_{};
    std::string_view data_{};
    std::string_view sha256_{};
    std::string_view sha512_{};
};

struct OPENTXS_EXPORT MurmurVector {
    std::string_view data_{};
    std::uint32_t seed_{};
    std::uint32_t expected_{};
};

struct OPENTXS_EXPORT NistHashVector {
    std::string_view input_{};
    std::string_view sha_1_{};
    std::string_view sha_2_256_{};
    std::string_view sha_2_512_{};
};

struct OPENTXS_EXPORT PBKDFVector {
    std::string_view input_{};
    std::string_view salt_{};
    std::size_t iterations_{};
    std::size_t bytes_{};
    std::string_view expected_{};
};

struct OPENTXS_EXPORT ScryptVector {
    std::string_view input_{};
    std::string_view salt_{};
    std::uint64_t n_{};
    std::uint32_t r_{};
    std::uint32_t p_{};
    std::size_t bytes_{};
    std::string_view expected_{};
};

OPENTXS_EXPORT auto Argon2i() noexcept -> const ot::Vector<ArgonVector>&;
OPENTXS_EXPORT auto Argon2id() noexcept -> const ot::Vector<ArgonVector>&;
OPENTXS_EXPORT auto Murmur() noexcept -> const ot::Vector<MurmurVector>&;
OPENTXS_EXPORT auto NistBasic() noexcept -> const ot::Vector<NistHashVector>&;
OPENTXS_EXPORT auto NistMillion() noexcept -> const NistHashVector&;
OPENTXS_EXPORT auto NistGigabyte() noexcept -> const NistHashVector&;
OPENTXS_EXPORT auto PBKDF_sha1() noexcept -> const ot::Vector<PBKDFVector>&;
OPENTXS_EXPORT auto PBKDF_sha256() noexcept -> const ot::Vector<PBKDFVector>&;
OPENTXS_EXPORT auto PBKDF_sha512() noexcept -> const ot::Vector<PBKDFVector>&;
OPENTXS_EXPORT auto rfc4231() noexcept -> const ot::Vector<HMACVector>&;
OPENTXS_EXPORT auto rfc7914() noexcept -> const ot::Vector<ScryptVector>&;
OPENTXS_EXPORT auto ScryptLitecoin() noexcept
    -> const ot::Vector<ScryptVector>&;
}  // namespace ottest
