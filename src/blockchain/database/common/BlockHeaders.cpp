// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/common/BlockHeaders.hpp"  // IWYU pragma: associated

#include <BlockchainBlockHeader.pb.h>
#include <optional>
#include <stdexcept>
#include <utility>

#include "internal/blockchain/block/Header.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"  // IWYU pragma: keep

namespace opentxs::blockchain::database::common
{
BlockHeader::BlockHeader(storage::lmdb::Database& lmdb) noexcept(false)
    : lmdb_(lmdb)
    , table_(Table::BlockHeaders)
{
}

auto BlockHeader::Exists(const block::Hash& hash) const noexcept -> bool
{
    return lmdb_.Exists(table_, hash.Bytes());
}

auto BlockHeader::Forget(const block::Hash& hash) const noexcept -> bool
{
    return lmdb_.Delete(table_, hash.Bytes());
}

auto BlockHeader::Load(const block::Hash& hash) const noexcept(false)
    -> proto::BlockchainBlockHeader
{
    auto output = std::optional<proto::BlockchainBlockHeader>{std::nullopt};
    auto cb = [&](const auto in) {
        if (valid(in)) {
            output.emplace(proto::Factory<proto::BlockchainBlockHeader>(in));
        }
    };
    lmdb_.Load(table_, hash.Bytes(), cb);

    if (output.has_value()) {

        return std::move(output.value());
    } else {
        const auto error =
            CString{"failed to load serialized block header "}.append(
                hash.asHex());

        throw std::out_of_range(error.c_str());
    }
}

auto BlockHeader::Store(const UpdatedHeader& headers) const noexcept -> bool
{
    try {
        auto tx = lmdb_.TransactionRW();

        for (const auto& [hash, data] : headers) {
            const auto& [header, save] = data;

            assert_true(header.IsValid());

            if (false == save) { continue; }

            // NOLINTBEGIN(clang-analyzer-core.CallAndMessage)
            const auto proto = [&]() {
                auto out = block::internal::Header::SerializedType{};

                if (false == header.Internal().Serialize(out)) {
                    const auto error =
                        CString{
                            "failed to serialize block header to protobuf: "}
                            .append(hash.asHex());

                    throw std::out_of_range(error.c_str());
                }

                out.clear_local();

                return out;
            }();
            // NOLINTEND(clang-analyzer-core.CallAndMessage)
            const auto bytes = [&]() {
                auto out = ByteArray{};

                if (false == proto::write(proto, out.WriteInto())) {
                    const auto error =
                        CString{"failed to serialize block header to bytes: "}
                            .append(hash.asHex());

                    throw std::out_of_range(error.c_str());
                }

                return out;
            }();
            const auto result =
                lmdb_.Store(table_, hash.Bytes(), bytes.Bytes(), tx);

            if (result.first) {
                LogTrace()()("saved block header ").asHex(hash).Flush();
            } else {
                throw std::runtime_error{
                    "Failed to store block header in database"};
            }
        }

        if (tx.Finalize(true)) {

            return true;
        } else {
            throw std::runtime_error{"database update error"};
        }
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::database::common
