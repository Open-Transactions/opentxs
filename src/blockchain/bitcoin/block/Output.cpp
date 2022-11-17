// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::node::TxoTag

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "blockchain/bitcoin/block/Output.hpp"  // IWYU pragma: associated

#include <BlockchainTransactionOutput.pb.h>
#include <BlockchainWalletKey.pb.h>
#include <algorithm>
#include <cstddef>
#include <iosfwd>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Script.hpp"
#include "internal/core/Amount.hpp"
#include "internal/core/Factory.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BitcoinTransactionOutput(
    const blockchain::Type chain,
    const std::uint32_t index,
    const blockchain::Amount& value,
    std::unique_ptr<const blockchain::bitcoin::block::Script> script,
    const UnallocatedSet<blockchain::crypto::Key>& keys) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Output>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Output;

    try {
        auto keySet = Set<blockchain::crypto::Key>{};
        std::for_each(std::begin(keys), std::end(keys), [&](const auto& key) {
            keySet.emplace(key);
        });

        return std::make_unique<ReturnType>(
            chain, index, value, std::move(script), std::move(keySet));
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinTransactionOutput(
    const blockchain::Type chain,
    const std::uint32_t index,
    const blockchain::Amount& value,
    const blockchain::bitcoin::CompactSize& cs,
    const ReadView script) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Output>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Output;

    try {
        return std::make_unique<ReturnType>(
            chain, index, value, sizeof(value) + cs.Total(), script);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinTransactionOutput(
    const api::crypto::Blockchain& crypto,
    const api::Factory& factory,
    const blockchain::Type chain,
    const proto::BlockchainTransactionOutput& in) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Output>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Output;
    using Position = blockchain::bitcoin::block::Script::Position;

    try {
        auto value = factory::Amount(in.value());
        auto cs = blockchain::bitcoin::CompactSize(in.script().size());
        auto keys = Set<blockchain::crypto::Key>{};
        auto pkh = ReturnType::PubkeyHashes{};
        using Payer = identifier::Generic;
        using Payee = identifier::Generic;
        using Correction = std::pair<Payer, Payee>;
        auto corrections = UnallocatedVector<Correction>{};

        for (const auto& key : in.key()) {
            const auto subchain = static_cast<blockchain::crypto::Subchain>(
                static_cast<std::uint8_t>(key.subchain()));
            auto keyid = blockchain::crypto::Key{
                factory.IdentifierFromBase58(key.subaccount()),
                subchain,
                key.index()};

            if (blockchain::crypto::Subchain::Outgoing == subchain) {
                LogError()("opentxs::factory::")(__func__)(
                    ": invalid key detected in transaction")
                    .Flush();
                auto sender = crypto.SenderContact(keyid);
                auto recipient = crypto.RecipientContact(keyid);

                if (sender.empty() || recipient.empty()) { OT_FAIL; }

                corrections.emplace_back(
                    std::move(sender), std::move(recipient));
            } else {
                keys.emplace(std::move(keyid));
            }
        }

        for (const auto& pattern : in.pubkey_hash()) { pkh.emplace(pattern); }

        auto out = std::make_unique<ReturnType>(
            chain,
            in.version(),
            in.index(),
            value,
            factory::BitcoinScript(chain, in.script(), Position::Output),
            sizeof(value) + cs.Total(),
            std::move(keys),
            // TODO std::move(pkh),
            // TODO (in.has_script_hash()
            //   ? std::make_optional<blockchain::PatternID>(in.script_hash())
            //   : std::nullopt),
            // TODO in.indexed(),
            [&]() -> blockchain::block::Position {
                if (const auto& hash = in.mined_block(); 0 < hash.size()) {

                    return {in.mined_height(), hash};
                } else {

                    return {};
                }
            }(),
            static_cast<blockchain::node::TxoState>(in.state()),
            [&] {
                auto out = UnallocatedSet<blockchain::node::TxoTag>{};

                for (const auto& tag : in.tag()) {
                    out.emplace(static_cast<blockchain::node::TxoTag>(tag));
                }

                return out;
            }());

        for (const auto& payer : in.payer()) {
            if (false == payer.empty()) {
                out->SetPayer([&] {
                    auto id = identifier::Generic{};
                    id.Assign(payer.data(), payer.size());

                    return id;
                }());
            }
        }

        for (const auto& payee : in.payee()) {
            if (false == payee.empty()) {
                out->SetPayee([&] {
                    auto id = identifier::Generic{};
                    id.Assign(payee.data(), payee.size());

                    return id;
                }());
            }
        }

        for (const auto& [payer, payee] : corrections) {
            out->SetPayer(payer);
            out->SetPayee(payee);
        }

        return out;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::bitcoin::block::implementation
{
const VersionNumber Output::default_version_{1};
const VersionNumber Output::key_version_{1};

Output::Output(
    const blockchain::Type chain,
    const VersionNumber version,
    const std::uint32_t index,
    const blockchain::Amount& value,
    std::unique_ptr<const block::Script> script,
    std::optional<std::size_t> size,
    Set<crypto::Key>&& keys,
    block::Position minedPosition,
    node::TxoState state,
    UnallocatedSet<node::TxoTag> tags) noexcept(false)
    : chain_(chain)
    , serialize_version_(version)
    , index_(index)
    , value_(value)
    , script_(std::move(script))
    , cache_(
          std::move(size),
          std::move(keys),
          std::move(minedPosition),
          state,
          std::move(tags))
    , guarded_()
{
    if (false == bool(script_)) {
        throw std::runtime_error("Invalid output script");
    }
}

Output::Output(
    const blockchain::Type chain,
    const std::uint32_t index,
    const blockchain::Amount& value,
    const std::size_t size,
    const ReadView in,
    const VersionNumber version) noexcept(false)
    : Output(
          chain,
          version,
          index,
          value,
          factory::BitcoinScript(chain, in, Script::Position::Output),
          size,
          {},
          block::Position{},
          node::TxoState::Error,
          {})
{
}

Output::Output(
    const blockchain::Type chain,
    const std::uint32_t index,
    const blockchain::Amount& value,
    std::unique_ptr<const block::Script> script,
    Set<crypto::Key>&& keys,
    const VersionNumber version) noexcept(false)
    : Output(
          chain,
          version,
          index,
          value,
          std::move(script),
          {},
          std::move(keys),
          block::Position{},
          node::TxoState::Error,
          {})
{
}

Output::Output(const Output& rhs) noexcept
    : chain_(rhs.chain_)
    , serialize_version_(rhs.serialize_version_)
    , index_(rhs.index_)
    , value_(rhs.value_)
    , script_(rhs.script_->Internal().clone())
    , cache_(rhs.cache_)
    , guarded_()
{
}

auto Output::AssociatedLocalNyms(
    const api::crypto::Blockchain& crypto,
    UnallocatedVector<identifier::Nym>& output) const noexcept -> void
{
    cache_.for_each_key([&](const auto& key) {
        const auto& owner = crypto.Owner(key);

        if (false == owner.empty()) { output.emplace_back(owner); }
    });
}

auto Output::AssociatedRemoteContacts(
    const api::session::Client& api,
    UnallocatedVector<identifier::Generic>& output) const noexcept -> void
{
    const auto hashes = script_->Internal().LikelyPubkeyHashes(api.Crypto());
    std::for_each(std::begin(hashes), std::end(hashes), [&](const auto& hash) {
        auto contacts = api.Crypto().Blockchain().LookupContacts(hash);
        std::move(
            std::begin(contacts),
            std::end(contacts),
            std::back_inserter(output));
    });

    auto payer = cache_.payer();
    auto payee = cache_.payee();

    if (false == payee.empty()) { output.emplace_back(std::move(payee)); }
    if (false == payer.empty()) { output.emplace_back(std::move(payer)); }
}

auto Output::CalculateSize() const noexcept -> std::size_t
{
    return cache_.size([&] {
        const auto scriptCS =
            blockchain::bitcoin::CompactSize(script_->CalculateSize());

        return opentxs::internal::Amount::SerializeBitcoinSize() +
               scriptCS.Total();
    });
}

auto Output::ExtractElements(const cfilter::Type style, Elements& out)
    const noexcept -> void
{
    script_->Internal().ExtractElements(style, out);
}

auto Output::ExtractElements(const cfilter::Type style, alloc::Default alloc)
    const noexcept -> Elements
{
    auto out = Elements{alloc};
    ExtractElements(style, out);
    std::sort(out.begin(), out.end());

    return out;
}

auto Output::FindMatches(
    const api::Session& api,
    const Txid& tx,
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
            log(OT_PRETTY_CLASS())("output ")(index_)(" of transaction ")
                .asHex(tx)(" matches ")(print(keyid))
                .Flush();

            if (crypto::Subchain::Outgoing == subchain) {
                auto sender = crypto.SenderContact(keyid);
                auto recipient = crypto.RecipientContact(keyid);

                if (sender.empty()) { OT_FAIL; }
                if (recipient.empty()) { OT_FAIL; }

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

        const auto scriptHash = script_->ScriptHash();

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
    std::copy(keys.begin(), keys.end(), std::inserter(out, out.end()));
}

auto Output::index_elements(
    const api::Session& api,
    PubkeyHashes& hashes,
    alloc::Default monotonic) const noexcept -> void
{
    const auto patterns = [&] {
        auto out = ElementHashes{monotonic};
        out.clear();
        script_->Internal().IndexElements(api, out);

        return out;
    }();
    LogTrace()(OT_PRETTY_CLASS())(patterns.size())(" pubkey hashes found:")
        .Flush();
    std::for_each(
        std::begin(patterns), std::end(patterns), [&](const auto& id) -> auto{
            hashes.emplace(id);
            LogTrace()("    * ")(id).Flush();
        });
}

auto Output::MergeMetadata(const internal::Output& rhs, const Log& log) noexcept
    -> bool
{
    return cache_.merge(rhs, index_, log);
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
        log(OT_PRETTY_CLASS())("output ")(index_)(" contributes ")(value_)
            .Flush();
    } else {
        log(OT_PRETTY_CLASS())("output ")(index_)(" contributes 0").Flush();
    }

    return output;
}

auto Output::Note(const api::crypto::Blockchain& crypto) const noexcept
    -> UnallocatedCString
{
    auto done{false};
    auto output = UnallocatedCString{};
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

auto Output::Print() const noexcept -> UnallocatedCString
{
    const auto& definition = blockchain::GetDefinition(chain_);

    auto out = std::stringstream{};
    out << "    value: " << definition.Format(value_) << '\n';
    out << "    script: " << '\n';
    out << script_->Print();

    return out.str();
}

auto Output::Script() const noexcept -> const block::Script&
{
    return *script_;
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

        const auto bytes = script_->CalculateSize();
        serialize_compact_size(bytes, buf, "script size");

        if (false == script_->Serialize(buf.Write(bytes))) {

            throw std::runtime_error{"failed to serialize script"};
        }

        check_finished(buf);

        return size;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return std::nullopt;
    }
}

auto Output::Serialize(const api::Session& api, SerializeType& out)
    const noexcept -> bool
{
    out.set_version(std::max(default_version_, serialize_version_));
    out.set_index(index_);
    value_.Serialize(writer(out.mutable_value()));

    if (false == script_->Serialize(writer(*out.mutable_script()))) {
        return false;
    }

    const auto& crypto = api.Crypto();
    const auto& blockchain = crypto.Blockchain();

    cache_.for_each_key([&](const auto& key) {
        const auto& [accountID, subchain, index] = key;
        auto& serializedKey = *out.add_key();
        serializedKey.set_version(key_version_);
        serializedKey.set_chain(
            translate(UnitToClaim(BlockchainToUnit(chain_))));
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

    return true;
}

auto Output::SigningSubscript() const noexcept
    -> std::unique_ptr<internal::Script>
{
    return script_->Internal().SigningSubscript(chain_);
}
}  // namespace opentxs::blockchain::bitcoin::block::implementation
