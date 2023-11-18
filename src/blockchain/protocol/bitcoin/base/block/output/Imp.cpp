// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/protocol/bitcoin/base/block/output/Imp.hpp"  // IWYU pragma: associated

#include <BlockchainTransactionOutput.pb.h>
#include <BlockchainWalletKey.pb.h>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/protocol/bitcoin/base/Bitcoin.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Factory.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Script.hpp"
#include "internal/blockchain/protocol/bitcoin/bitcoincash/token/Types.hpp"
#include "internal/core/Amount.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/cfilter/FilterType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Element.hpp"      // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Subchain.hpp"     // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Pattern.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/Position.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/Script.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::protocol::bitcoin::base::block::implementation
{
const VersionNumber Output::default_version_{1};
const VersionNumber Output::key_version_{1};

Output::Output(
    const blockchain::Type chain,
    const VersionNumber version,
    const std::uint32_t index,
    const Amount& value,
    block::Script script,
    std::optional<std::size_t> size,
    Set<crypto::Key>&& keys,
    block::Position minedPosition,
    node::TxoState state,
    UnallocatedSet<node::TxoTag> tags,
    std::optional<const bitcoincash::token::cashtoken::Value> cashtoken,
    allocator_type alloc) noexcept(false)
    : OutputPrivate(alloc)
    , chain_(chain)
    , serialize_version_(version)
    , index_(index)
    , value_(value)
    , script_(std::move(script), alloc)
    , cashtoken_(std::move(cashtoken))
    , cashtoken_view_([&]() -> decltype(cashtoken_view_) {
        if (cashtoken_.has_value()) {

            return cashtoken_->View();
        } else {

            return {};
        }
    }())
    , cache_(
          std::move(size),
          std::move(keys),
          std::move(minedPosition),
          state,
          std::move(tags))
    , guarded_()
{
    if (false == script_.IsValid()) {
        throw std::runtime_error("Invalid output script");
    }
}

Output::Output(
    const blockchain::Type chain,
    const std::uint32_t index,
    const Amount& value,
    const std::size_t size,
    const ReadView in,
    const VersionNumber version,
    std::optional<const bitcoincash::token::cashtoken::Value> cashtoken,
    allocator_type alloc) noexcept(false)
    : Output(
          chain,
          version,
          index,
          value,
          factory::BitcoinScript(
              chain,
              in,
              script::Position::Output,
              true,
              false,
              alloc),
          size,
          Set<crypto::Key>{alloc},
          block::Position{},
          node::TxoState::Error,
          UnallocatedSet<node::TxoTag>{},
          std::move(cashtoken),
          alloc)
{
}

Output::Output(
    const blockchain::Type chain,
    const std::uint32_t index,
    const Amount& value,
    block::Script script,
    Set<crypto::Key>&& keys,
    const VersionNumber version,
    std::optional<const bitcoincash::token::cashtoken::Value> cashtoken,
    allocator_type alloc) noexcept(false)
    : Output(
          chain,
          version,
          index,
          value,
          std::move(script),
          std::nullopt,
          std::move(keys),
          block::Position{},
          node::TxoState::Error,
          UnallocatedSet<node::TxoTag>{},
          std::move(cashtoken),
          alloc)
{
}

Output::Output(const Output& rhs, allocator_type alloc) noexcept
    : OutputPrivate(rhs, alloc)
    , chain_(rhs.chain_)
    , serialize_version_(rhs.serialize_version_)
    , index_(rhs.index_)
    , value_(rhs.value_)
    , script_(rhs.script_, alloc)
    , cashtoken_(rhs.cashtoken_)
    , cashtoken_view_([&]() -> decltype(cashtoken_view_) {
        if (cashtoken_.has_value()) {

            return cashtoken_->View();
        } else {

            return {};
        }
    }())
    , cache_(rhs.cache_)
    , guarded_()
{
}

auto Output::AssociatedLocalNyms(
    const api::crypto::Blockchain& crypto,
    Set<identifier::Nym>& output) const noexcept -> void
{
    cache_.for_each_key([&](const auto& key) {
        const auto& owner = crypto.Owner(key);

        if (false == owner.empty()) { output.emplace(owner); }
    });
}

auto Output::AssociatedRemoteContacts(
    const api::session::Client& api,
    Set<identifier::Generic>& output) const noexcept -> void
{
    const auto hashes = script_.Internal().LikelyPubkeyHashes(api.Crypto());
    std::ranges::for_each(hashes, [&](const auto& hash) {
        auto contacts = api.Crypto().Blockchain().LookupContacts(hash);
        std::ranges::move(contacts, std::inserter(output, output.end()));
    });

    auto payer = cache_.payer();
    auto payee = cache_.payee();

    if (false == payee.empty()) { output.emplace(std::move(payee)); }
    if (false == payer.empty()) { output.emplace(std::move(payer)); }
}

auto Output::CalculateSize() const noexcept -> std::size_t
{
    using namespace blockchain::protocol::bitcoin::base;

    return cache_.size([&] {
        const auto [total, cashtoken, script] = script_bytes();
        const auto scriptCS = CompactSize(total);

        return opentxs::internal::Amount::SerializeBitcoinSize() +
               scriptCS.Total();
    });
}

auto Output::Cashtoken() const noexcept
    -> const bitcoincash::token::cashtoken::View*
{
    if (cashtoken_.has_value()) {

        return std::addressof(cashtoken_view_);
    } else {

        return nullptr;
    }
}

auto Output::ConfirmMatches(
    const Log& log,
    const api::crypto::Blockchain& api,
    const Matches& candiates) noexcept -> bool
{
    const auto& [_, byKey] = candiates;

    for (const auto& [txid, data1] : byKey) {
        const auto& [index, data2] = data1;
        const auto& [subchain, subaccount] = data2;
        auto alloc = get_allocator();
        auto keyID = crypto::Key{{subaccount, alloc}, subchain, index};
        auto haveKey = false;
        auto haveMatch = false;
        const auto spendable = crypto::Subchain::Outgoing != subchain;
        auto post2 = ScopeGuard{[&] {
            if (haveMatch) {
                const auto confirmed = api.Confirm(keyID, txid);

                assert_true(confirmed);
            }

            if (haveKey) {
                if (spendable) {
                    cache_.add(std::move(keyID));
                } else {
                    cache_.set_payer(api.SenderContact(keyID));
                    cache_.set_payee(api.RecipientContact(keyID));
                }
            }
        }};
        const auto& element = api.GetKey(keyID);

        switch (script_.Type()) {
            using enum script::Pattern;
            case PayToPubkey: {
                const auto& key = element.Key();

                assert_true(key.IsValid());
                assert_true(script_.Pubkey().has_value());

                if (key.PublicKey() == script_.Pubkey().value()) {
                    log()("found P2PK match").Flush();
                    haveMatch = spendable;
                    haveKey = true;

                    return haveMatch;
                }
            } break;
            case PayToPubkeyHash: {
                const auto hash = element.PubkeyHash();

                assert_true(script_.PubkeyHash().has_value());

                if (hash.Bytes() == script_.PubkeyHash().value()) {
                    log()("found P2PKH match").Flush();
                    haveMatch = spendable;
                    haveKey = true;

                    return haveMatch;
                }
            } break;
            case PayToWitnessPubkeyHash: {
                const auto hash = element.PubkeyHash();

                assert_true(script_.PubkeyHash().has_value());

                if (hash.Bytes() == script_.PubkeyHash().value()) {
                    log()("found P2WPKH match").Flush();
                    haveMatch = spendable;
                    haveKey = true;

                    return haveMatch;
                }
            } break;
            case PayToMultisig: {
                const auto required = script_.M();
                const auto of = script_.N();

                assert_true(required.has_value());
                assert_true(of.has_value());

                if (1u != required.value() || (3u != of.value())) {
                    // TODO handle non-payment code multisig eventually

                    continue;
                }

                const auto& key = element.Key();

                assert_true(key.IsValid());

                if (key.PublicKey() == script_.MultisigPubkey(0).value()) {
                    log()(" found ")(required.value())(" of ")(of.value())(
                        " P2MS match")
                        .Flush();
                    haveMatch = spendable;
                    haveKey = true;

                    return haveMatch;
                }
            } break;
            case PayToScriptHash:
            default: {
            }
        }
    }

    return false;
}

auto Output::ExtractElements(const cfilter::Type style, Elements& out)
    const noexcept -> void
{
    script_.Internal().ExtractElements(style, out);
}

auto Output::ExtractElements(const cfilter::Type style, alloc::Default alloc)
    const noexcept -> Elements
{
    auto out = Elements{alloc};
    ExtractElements(style, out);
    using enum cfilter::Type;

    if ((ES == style) && cashtoken_.has_value()) {
        const auto& element = cashtoken_->category_;
        const auto* start = static_cast<const std::byte*>(element.data());
        const auto* end = std::next(start, element.size());
        out.emplace_back(start, end);
    }

    std::ranges::sort(out);

    return out;
}

auto Output::FindMatches(
    const api::Session& api,
    const TransactionHash& tx,
    const cfilter::Type type,
    const ParsedPatterns& patterns,
    const Log& log,
    Matches& out,
    alloc::Default monotonic) const noexcept -> void
{
    blockchain::block::internal::SetIntersection(
        tx.Bytes(),
        patterns,
        ExtractElements(type, monotonic),
        [&](const auto& match) {
            const auto& crypto = api.Crypto().Blockchain();
            const auto& [txid, element] = match;
            const auto& [index, subchainID] = element;
            const auto& [subchain, account] = subchainID;
            auto keyid = crypto::Key{account, subchain, index};
            log()("output ")(index_)(" of transaction ")
                .asHex(tx)(" matches ")(print(keyid, api.Crypto()))
                .Flush();

            if (crypto::Subchain::Outgoing == subchain) {
                auto sender = crypto.SenderContact(keyid);
                auto recipient = crypto.RecipientContact(keyid);

                if (sender.empty()) { LogAbort()().Abort(); }
                if (recipient.empty()) { LogAbort()().Abort(); }

                cache_.set_payer(sender);
                cache_.set_payee(recipient);
            } else {
                cache_.add(std::move(keyid));
            }
        },
        out,
        monotonic);
}

auto Output::get_pubkeys(const api::Session& api, alloc::Default monotonic)
    const noexcept -> const PubkeyHashes&
{
    const auto instance = api.Instance();
    auto handle = guarded_.lock();
    auto& map = *handle;

    if (auto i = map.find(instance); map.end() != i) {

        return i->second.first;
    } else {
        auto& [pubkeys, _] = map[instance];
        index_elements(api, pubkeys, monotonic);

        return pubkeys;
    }
}

auto Output::get_script_hash(const api::Session& api) const noexcept
    -> const std::optional<ElementHash>&
{
    const auto instance = api.Instance();
    auto handle = guarded_.lock();
    auto& map = *handle;

    if (auto i = map.find(instance); map.end() != i) {

        return i->second.second;
    } else {
        auto& [_, sh] = map[instance];

        const auto scriptHash = script_.ScriptHash();

        if (scriptHash.has_value()) {
            sh.emplace(api.Crypto().Blockchain().Internal().IndexItem(
                scriptHash.value()));
        }

        return sh;
    }
}

auto Output::IndexElements(const api::Session& api, ElementHashes& out)
    const noexcept -> void
{
    // TODO monotonic allocator
    const auto& keys = get_pubkeys(api, {});
    std::ranges::copy(keys, std::inserter(out, out.end()));
}

auto Output::index_elements(
    const api::Session& api,
    PubkeyHashes& hashes,
    alloc::Default monotonic) const noexcept -> void
{
    const auto patterns = [&] {
        auto out = ElementHashes{monotonic};
        out.clear();
        script_.Internal().IndexElements(api, out);

        return out;
    }();
    LogTrace()()(patterns.size())(" pubkey hashes found:").Flush();
    std::ranges::for_each(patterns, [&](const auto& id) -> auto {
        hashes.emplace(id);
        LogTrace()("    * ")(id).Flush();
    });
}

auto Output::Keys(alloc::Default alloc) const noexcept -> Set<crypto::Key>
{
    auto out = Set<crypto::Key>{alloc};
    out.clear();
    cache_.keys(out);

    return out;
}

auto Output::Keys(Set<crypto::Key>& out) const noexcept -> void
{
    cache_.keys(out);
}

auto Output::MergeMetadata(
    const api::Crypto& crypto,
    const internal::Output& rhs,
    const Log& log) noexcept -> void
{
    cache_.merge(crypto, rhs, index_, log);
}

auto Output::NetBalanceChange(
    const api::crypto::Blockchain& crypto,
    const identifier::Nym& nym,
    const Log& log) const noexcept -> opentxs::Amount
{
    auto done{false};
    auto output = opentxs::Amount{0};
    cache_.for_each_key([&](const auto& key) {
        if (done) { return; }

        if (nym == crypto.Owner(key)) {
            done = true;
            output = value_;
        }
    });

    if (done) {
        log()("output ")(index_)(" contributes ")(value_).Flush();
    } else {
        log()("output ")(index_)(" contributes 0").Flush();
    }

    return output;
}

auto Output::Note(const api::crypto::Blockchain& crypto) const noexcept
    -> UnallocatedCString
{
    return Note(crypto, {}).c_str();
}

auto Output::Note(const api::crypto::Blockchain& crypto, alloc::Default alloc)
    const noexcept -> CString
{
    auto done{false};
    auto output = CString{alloc};
    output.clear();
    cache_.for_each_key([&](const auto& key) {
        if (done) { return; }

        try {
            const auto& element = crypto.GetKey(key);
            const auto note = element.Label();

            if (false == note.empty()) {
                done = true;
                output = note;
            }
        } catch (...) {
        }
    });

    return output;
}

auto Output::Print(const api::Crypto& api) const noexcept -> UnallocatedCString
{
    return Print(api, {}).c_str();
}

auto Output::Print(const api::Crypto& api, alloc::Default alloc) const noexcept
    -> CString
{
    const auto& definition = blockchain::GetDefinition(chain_);
    // TODO allocator
    auto out = std::stringstream{};
    out << "    value: " << definition.Format(value_) << '\n';
    out << "    script: " << '\n';
    out << script_.Print();
    out << "    associated keys: " << '\n';
    cache_.for_each_key([&](const auto& key) {
        out << "        * " << print(key, api) << '\n';
    });

    return CString{out.str(), alloc};
}

auto Output::RefreshContacts(const api::crypto::Blockchain& api) noexcept
    -> void
{
    auto alloc = get_allocator();
    const auto keys = [&] {
        auto out = Set<crypto::Key>{alloc};
        cache_.keys(out);

        return out;
    }();
    auto data = block::KeyData{alloc};

    for (const auto& key : keys) {
        data.try_emplace(
            key, api.SenderContact(key), api.RecipientContact(key));
    }

    cache_.set(data);
}

auto Output::Script() const noexcept -> const block::Script& { return script_; }

auto Output::script_bytes() const noexcept
    -> std::tuple<std::size_t, std::size_t, std::size_t>
{
    const auto cashtoken = (cashtoken_ ? cashtoken_->Bytes() : 0_uz);
    const auto script = script_.CalculateSize();

    return std::make_tuple(script + cashtoken, cashtoken, script);
}

auto Output::Serialize(Writer&& destination) const noexcept
    -> std::optional<std::size_t>
{
    try {
        const auto size = CalculateSize();
        auto buf = reserve(std::move(destination), size, "output");
        static const auto amount =
            opentxs::internal::Amount::SerializeBitcoinSize();

        if (false == value_.Internal().SerializeBitcoin(buf.Write(amount))) {

            throw std::runtime_error{"failed to serialize amount"};
        }

        const auto [total, cashtoken, script] = script_bytes();
        serialize_compact_size(total, buf, "script size");

        if (cashtoken_.has_value()) {
            cashtoken_->Serialize(buf.Write(cashtoken));
        }

        if (false == script_.Serialize(buf.Write(script))) {

            throw std::runtime_error{"failed to serialize script"};
        }

        check_finished(buf);

        return size;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return std::nullopt;
    }
}

auto Output::Serialize(const api::Session& api, SerializeType& out)
    const noexcept -> bool
{
    out.set_version(std::max(default_version_, serialize_version_));
    out.set_index(index_);
    value_.Serialize(writer(out.mutable_value()));

    if (false == script_.Serialize(writer(*out.mutable_script()))) {
        return false;
    }

    const auto& crypto = api.Crypto();
    const auto& blockchain = crypto.Blockchain();

    cache_.for_each_key([&](const auto& key) {
        const auto& [accountID, subchain, index] = key;
        auto& serializedKey = *out.add_key();
        serializedKey.set_version(key_version_);
        serializedKey.set_chain(
            translate(UnitToClaim(blockchain_to_unit(chain_))));
        serializedKey.set_nym(blockchain.Owner(key).asBase58(crypto));
        serializedKey.set_subaccount(accountID.asBase58(crypto));
        serializedKey.set_subchain(static_cast<std::uint32_t>(subchain));
        serializedKey.set_index(index);
    });

    // TODO monotonic allocator
    for (const auto& id : get_pubkeys(api, {})) { out.add_pubkey_hash(id); }

    if (const auto& sh = get_script_hash(api); sh.has_value()) {
        out.set_script_hash(sh.value());
    }

    out.set_indexed(true);

    if (const auto payer = cache_.payer(); false == payer.empty()) {
        out.add_payer(UnallocatedCString{payer.Bytes()});
    }

    if (const auto payee = cache_.payee(); false == payee.empty()) {
        out.add_payee(UnallocatedCString{payee.Bytes()});
    }

    if (const auto& [height, hash] = cache_.position(); 0 <= height) {
        out.set_mined_height(height);
        out.set_mined_block(hash.data(), hash.size());
    }

    if (auto state = cache_.state(); node::TxoState::Error != state) {
        out.set_state(static_cast<std::uint32_t>(state));
    }

    for (const auto tag : cache_.tags()) {
        out.add_tag(static_cast<std::uint32_t>(tag));
    }

    if (cashtoken_.has_value()) { cashtoken_->Serialize(out); }

    return true;
}

auto Output::SigningSubscript(alloc::Default alloc) const noexcept
    -> block::Script
{
    return script_.Internal().SigningSubscript(chain_, alloc);
}

Output::~Output() = default;
}  // namespace
   // opentxs::blockchain::protocol::bitcoin::base::block::implementation
