// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/crypto/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <compare>
#include <cstring>
#include <iterator>
#include <sstream>
#include <string_view>
#include <utility>
#include <variant>

#include "opentxs/blockchain/crypto/AddressStyle.hpp"    // IWYU pragma: keep
#include "opentxs/blockchain/crypto/HDProtocol.hpp"      // IWYU pragma: keep
#include "opentxs/blockchain/crypto/SubaccountType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Subchain.hpp"        // IWYU pragma: keep
#include "opentxs/blockchain/token/Descriptor.hpp"
#include "opentxs/blockchain/token/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::blockchain::crypto
{
using namespace std::literals;

auto base_chain(const Target& in) noexcept -> blockchain::Type
{
    struct Visitor {
        auto operator()(blockchain::Type in) const noexcept { return in; }
        auto operator()(const token::Descriptor& in) const noexcept
        {
            return in.host_;
        }
    };

    return std::visit(Visitor{}, in);
}

auto is_notification(Subchain in) noexcept -> bool
{
    switch (in) {
        case Subchain::NotificationV1:
        case Subchain::NotificationV2:
        case Subchain::NotificationV3:
        case Subchain::NotificationV4: {

            return true;
        }
        default: {

            return false;
        }
    }
}

auto is_token(const Target& in) noexcept -> bool
{
    return std::holds_alternative<token::Descriptor>(in);
}

auto operator==(const Key& lhs, const Key& rhs) noexcept -> bool
{
    const auto& [lAccount, lSubchain, lIndex] = lhs;
    const auto& [rAccount, rSubchain, rIndex] = rhs;

    if (lAccount != rAccount) { return false; }

    if (lSubchain != rSubchain) { return false; }

    return lIndex == rIndex;
}

auto operator==(const Target& lhs, const Target& rhs) noexcept -> bool
{
    struct Visitor {
        auto operator()(blockchain::Type lhs, blockchain::Type rhs)
            const noexcept
        {
            return lhs == rhs;
        }
        auto operator()(blockchain::Type lhs, const token::Descriptor& rhs)
            const noexcept
        {
            return false;
        }
        auto operator()(
            const token::Descriptor& lhs,
            const token::Descriptor& rhs) const noexcept
        {
            return lhs == rhs;
        }
        auto operator()(const token::Descriptor& lhs, blockchain::Type rhs)
            const noexcept
        {
            return false;
        }
    };

    return std::visit(Visitor{}, lhs, rhs);
}

auto operator<=>(const Key& lhs, const Key& rhs) noexcept
    -> std::strong_ordering
{
    const auto& [lAccount, lSubchain, lIndex] = lhs;
    const auto& [rAccount, rSubchain, rIndex] = rhs;
    constexpr auto equal = std::strong_ordering::equal;

    if (auto out = lAccount <=> rAccount; equal != out) {

        return out;
    } else if (out = lSubchain <=> rSubchain; equal != out) {

        return out;
    } else {

        return lIndex <=> rIndex;
    }
}

auto operator<=>(const Target& lhs, const Target& rhs) noexcept
    -> std::strong_ordering
{
    static constexpr auto equal = std::strong_ordering::equal;
    static constexpr auto greater = std::strong_ordering::greater;
    static constexpr auto less = std::strong_ordering::less;

    struct Visitor {
        auto operator()(blockchain::Type lhs, blockchain::Type rhs)
            const noexcept
        {
            return lhs <=> rhs;
        }
        auto operator()(blockchain::Type lhs, const token::Descriptor& rhs)
            const noexcept
        {
            if (auto out = lhs <=> rhs.host_; equal != out) {

                return out;
            } else {

                return less;
            }
        }
        auto operator()(
            const token::Descriptor& lhs,
            const token::Descriptor& rhs) const noexcept
        {
            return lhs <=> rhs;
        }
        auto operator()(const token::Descriptor& lhs, blockchain::Type rhs)
            const noexcept
        {
            if (auto out = lhs.host_ <=> rhs; equal != out) {

                return out;
            } else {

                return greater;
            }
        }
    };

    return std::visit(Visitor{}, lhs, rhs);
}

auto print(AddressStyle value) noexcept -> std::string_view
{
    using enum AddressStyle;
    static constexpr auto map =
        frozen::make_unordered_map<AddressStyle, std::string_view>({
            {Unknown, "Unknown"sv},
            {P2PKH, "P2PKH"sv},
            {P2SH, "P2SH"sv},
            {P2WPKH, "P2WPKH"sv},
            {P2WSH, "P2WSH"sv},
            {P2TR, "P2TR"sv},
        });

    if (const auto* i = map.find(value); map.end() != i) {

        return i->second;
    } else {

        return map.at(Unknown);
    }
}

auto print(HDProtocol value) noexcept -> std::string_view
{
    using enum HDProtocol;
    static constexpr auto map =
        frozen::make_unordered_map<HDProtocol, std::string_view>({
            {Error, "invalid"sv},
            {BIP_32, "BIP-32"sv},
            {BIP_44, "BIP-44"sv},
            {BIP_49, "BIP-49"sv},
            {BIP_84, "BIP-84"sv},
        });

    if (const auto* i = map.find(value); map.end() != i) {

        return i->second;
    } else {

        return map.at(Error);
    }
}

auto print(SubaccountType type) noexcept -> std::string_view
{
    using enum SubaccountType;
    static constexpr auto map =
        frozen::make_unordered_map<SubaccountType, std::string_view>({
            {Error, "invalid"sv},
            {HD, "HD"sv},
            {PaymentCode, "payment code"sv},
            {Imported, "single key"sv},
            {Notification, "payment code notification"sv},
        });

    if (const auto* i = map.find(type); map.end() != i) {

        return i->second;
    } else {

        return map.at(Error);
    }
}
auto print(Subchain value) noexcept -> std::string_view
{
    using enum Subchain;
    static constexpr auto map =
        frozen::make_unordered_map<Subchain, std::string_view>({
            {Error, "invalid"sv},
            {Internal, "internal"sv},
            {External, "external"sv},
            {Incoming, "incoming"sv},
            {Outgoing, "outgoing"sv},
            {NotificationV3, "version 3"sv},
            {NotificationV1, "version 1"sv},
            {NotificationV2, "version 2"sv},
            {NotificationV4, "version 4"sv},
            {None, "none"sv},
        });

    if (const auto* i = map.find(value); map.end() != i) {

        return i->second;
    } else {

        return map.at(Error);
    }
}

static auto print_key(
    const Key& key,
    const api::Crypto& api,
    alloc::Strategy alloc) noexcept -> std::stringstream
{
    const auto& [account, subchain, index] = key;
    auto out = std::stringstream{};  // TODO c++20
    out << account.asBase58(api);
    out << " / ";
    out << print(subchain);
    out << " / ";
    out << std::to_string(index);

    return out;
}

auto print(
    const Key& key,
    const api::Crypto& api,
    alloc::Strategy alloc) noexcept -> CString
{
    const auto out = print_key(key, api, alloc.WorkOnly());

    return CString{out.str().c_str(), alloc.result_};  // TODO c++20
}

auto print(const Key& key, const api::Crypto& api) noexcept
    -> UnallocatedCString
{
    const auto out = print_key(key, api, {});

    return out.str();
}

static auto print_target(const Target& target, alloc::Strategy alloc) noexcept
    -> std::stringstream
{
    auto out = std::stringstream{};  // TODO c++20

    struct Visitor {
        std::stringstream& out_;
        alloc::Strategy& alloc_;

        auto operator()(blockchain::Type in) noexcept { out_ << print(in); }
        auto operator()(const token::Descriptor& in) noexcept
        {
            const auto id = [&] {
                auto hex = in.id_.asHex(alloc_.work_);
                const auto before = hex.size();

                while ((false == hex.empty()) && (hex.back() == '0')) {
                    hex.pop_back();
                }

                if ((before > hex.size()) && (0 != hex.size() % 2)) {
                    hex.push_back('0');
                }

                return hex;
            }();

            out_ << print(token_to_unit(in));
            out_ << " on ";
            out_ << print(in.host_);
            out_ << " (";
            out_ << print(in.type_);
            out_ << " 0x";
            out_ << in.id_.asHex();
            out_ << ")";
        }
    };

    std::visit(Visitor{out, alloc}, target);

    return out;
}

auto print(const Target& target, alloc::Strategy alloc) noexcept -> CString
{
    const auto out = print_target(target, alloc.WorkOnly());

    return CString{out.str().c_str(), alloc.result_};  // TODO c++20
}

auto print(const Target& target) noexcept -> UnallocatedCString
{
    const auto out = print_target(target, {});

    return out.str();
}

auto target_to_unit(const Target& target) noexcept -> opentxs::UnitType
{
    struct Visitor {
        auto operator()(blockchain::Type in) const noexcept
        {
            return blockchain_to_unit(in);
        }
        auto operator()(const token::Descriptor& in) const noexcept
        {
            return token_to_unit(in);
        }
    };

    return std::visit(Visitor{}, target);
}
}  // namespace opentxs::blockchain::crypto

namespace opentxs
{
auto deserialize(const ReadView in) noexcept -> blockchain::crypto::Key
{
    auto out = blockchain::crypto::Key{};
    auto& [id, subchain, index] = out;

    if (in.size() < (sizeof(subchain) + sizeof(index))) { return out; }

    const auto idbytes = in.size() - sizeof(subchain) - sizeof(index);

    const auto* i = in.data();
    id.Assign(ReadView{i, idbytes});
    std::advance(i, idbytes);
    std::memcpy(&subchain, i, sizeof(subchain));
    std::advance(i, sizeof(subchain));
    std::memcpy(&index, i, sizeof(index));
    std::advance(i, sizeof(index));

    return out;
}

auto serialize(const blockchain::crypto::Key& in) noexcept -> Space
{
    const auto& [id, subchain, index] = in;
    auto out = space(id.size() + sizeof(subchain) + sizeof(index));
    auto* i = out.data();
    std::memcpy(i, id.data(), id.size());
    std::advance(i, id.size());
    std::memcpy(i, &subchain, sizeof(subchain));
    std::advance(i, sizeof(subchain));
    std::memcpy(i, &index, sizeof(index));
    std::advance(i, sizeof(index));

    return out;
}
}  // namespace opentxs
