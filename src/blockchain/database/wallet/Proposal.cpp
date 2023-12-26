// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/wallet/Proposal.hpp"  // IWYU pragma: associated

#include <BlockchainTransactionProposal.pb.h>
#include <cs_plain_guarded.h>
#include <stdexcept>

#include "internal/blockchain/database/Types.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::database::wallet
{
using Direction = storage::lmdb::Dir;
constexpr auto table_{Table::Proposals};

struct Proposal::Imp {
    auto CompletedProposals() const noexcept
        -> UnallocatedSet<identifier::Generic>
    {
        return data_.lock()->finished_;
    }
    auto Exists(const identifier::Generic& id) const noexcept -> bool
    {
        return lmdb_.Exists(table_, id.Bytes());
    }
    auto LoadProposal(const identifier::Generic& id) const noexcept
        -> std::optional<proto::BlockchainTransactionProposal>
    {
        return load_proposal(id);
    }
    auto LoadProposals() const noexcept
        -> UnallocatedVector<proto::BlockchainTransactionProposal>
    {
        auto output = UnallocatedVector<proto::BlockchainTransactionProposal>{};
        lmdb_.Read(
            table_,
            [&](const auto key, const auto value) -> bool {
                output.emplace_back(
                    proto::Factory<proto::BlockchainTransactionProposal>(
                        value.data(), value.size()));

                return true;
            },
            Direction::Forward);

        return output;
    }

    auto AddProposal(
        const identifier::Generic& id,
        const proto::BlockchainTransactionProposal& tx) noexcept -> bool
    {
        try {
            const auto bytes = [&] {
                auto out = ByteArray{};

                if (false == proto::write(tx, out.WriteInto())) {
                    throw std::runtime_error{"failed to serialize proposal"};
                }

                return out;
            }();
            const auto cb = [&](const auto) {
                auto out = Space{};
                copy(bytes.Bytes(), writer(out));

                return out;
            };

            if (lmdb_.StoreOrUpdate(table_, id.Bytes(), cb).first) {
                LogVerbose()()("proposal ")(id, crypto_)(" added ").Flush();

                return true;
            } else {
                throw std::runtime_error{"failed to store proposal"};
            }
        } catch (const std::exception& e) {
            LogError()()(e.what()).Flush();

            return false;
        }
    }
    auto CancelProposal(
        storage::lmdb::Transaction& tx,
        const identifier::Generic& id) noexcept -> bool
    {
        return cancel(*data_.lock(), tx, id);
    }
    auto FinishProposal(
        storage::lmdb::Transaction& tx,
        const identifier::Generic& id) noexcept -> bool
    {
        auto handle = data_.lock();
        auto& data = *handle;
        const auto out = cancel(data, tx, id);
        data.finished_.emplace(id);

        return out;
    }
    auto ForgetProposals(
        const UnallocatedSet<identifier::Generic>& ids) noexcept -> bool
    {
        auto handle = data_.lock();
        auto& data = *handle;

        for (const auto& id : ids) { data.finished_.erase(id); }

        return true;
    }

    Imp(const api::Crypto& crypto, const storage::lmdb::Database& lmdb) noexcept
        : crypto_(crypto)
        , lmdb_(lmdb)
        , data_()
    {
    }
    Imp() = delete;
    Imp(const Imp&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

private:
    using Proposals = UnallocatedSet<identifier::Generic>;

    struct Data {
        Proposals finished_{};
        Proposals cancelled_{};
    };

    using Guarded = libguarded::plain_guarded<Data>;

    const api::Crypto& crypto_;
    const storage::lmdb::Database& lmdb_;
    mutable Guarded data_;

    auto cancel(
        Data& data,
        storage::lmdb::Transaction& tx,
        const identifier::Generic& id) noexcept -> bool
    {
        if (data.cancelled_.contains(id)) {

            return true;
        } else if (lmdb_.Delete(table_, id.Bytes(), tx)) {
            LogError()()("proposal ")(id, crypto_)(" cancelled").Flush();
            data.cancelled_.emplace(id);

            return true;
        } else if (lmdb_.Exists(table_, id.Bytes())) {
            LogError()()("failed to cancel proposal ")(id, crypto_).Flush();

            return false;
        } else {
            LogError()()("proposal ")(id, crypto_)(" already cancelled ")
                .Flush();
            data.cancelled_.emplace(id);

            return true;
        }
    }
    auto load_proposal(const identifier::Generic& id) const noexcept
        -> std::optional<proto::BlockchainTransactionProposal>
    {
        auto out = std::optional<proto::BlockchainTransactionProposal>{};
        lmdb_.Load(Table::Proposals, id.Bytes(), [&](const auto bytes) {
            out = proto::Factory<proto::BlockchainTransactionProposal>(
                bytes.data(), bytes.size());
        });

        return out;
    }
};

Proposal::Proposal(
    const api::Crypto& crypto,
    const storage::lmdb::Database& lmdb) noexcept
    : imp_(std::make_unique<Imp>(crypto, lmdb))
{
    assert_false(nullptr == imp_);
}

auto Proposal::AddProposal(
    const identifier::Generic& id,
    const proto::BlockchainTransactionProposal& tx) noexcept -> bool
{
    return imp_->AddProposal(id, tx);
}

auto Proposal::CancelProposal(
    storage::lmdb::Transaction& tx,
    const identifier::Generic& id) noexcept -> bool
{
    return imp_->CancelProposal(tx, id);
}

auto Proposal::CompletedProposals() const noexcept
    -> UnallocatedSet<identifier::Generic>
{
    return imp_->CompletedProposals();
}

auto Proposal::Exists(const identifier::Generic& id) const noexcept -> bool
{
    return imp_->Exists(id);
}

auto Proposal::FinishProposal(
    storage::lmdb::Transaction& tx,
    const identifier::Generic& id) noexcept -> bool
{
    return imp_->FinishProposal(tx, id);
}

auto Proposal::ForgetProposals(
    const UnallocatedSet<identifier::Generic>& ids) noexcept -> bool
{
    return imp_->ForgetProposals(ids);
}

auto Proposal::LoadProposal(const identifier::Generic& id) const noexcept
    -> std::optional<proto::BlockchainTransactionProposal>
{
    return imp_->LoadProposal(id);
}

auto Proposal::LoadProposals() const noexcept
    -> UnallocatedVector<proto::BlockchainTransactionProposal>
{
    return imp_->LoadProposals();
}

Proposal::~Proposal() = default;
}  // namespace opentxs::blockchain::database::wallet
