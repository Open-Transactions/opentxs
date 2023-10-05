// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/params/Types.hpp"  // IWYU pragma: associated

#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>
#include <boost/json.hpp>
#include <cstdint>
#include <fstream>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>

#include "blockchain/params/ChainDataPrivate.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/cfilter/FilterType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/cfilter/Header.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"      // IWYU pragma: keep
#include "opentxs/crypto/Bip44Type.hpp"                    // IWYU pragma: keep
#include "opentxs/network/blockchain/Protocol.hpp"         // IWYU pragma: keep
#include "opentxs/network/blockchain/Subchain.hpp"         // IWYU pragma: keep
#include "opentxs/network/blockchain/bitcoin/Service.hpp"  // IWYU pragma: keep
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::blockchain::params
{
auto WriteCheckpoints(const std::filesystem::path& outputDirectory) noexcept
    -> bool
{
    auto output{true};

    for (const auto chain : chains()) {
        if (false == WriteCheckpoint(outputDirectory, chain)) {
            output = false;
        }
    }

    return output;
}

auto WriteCheckpoint(
    const std::filesystem::path& outputDirectory,
    blockchain::Type chain) noexcept -> bool
{
    try {
        const auto& data = get(chain);

        return WriteCheckpoint(
            outputDirectory,
            data.CheckpointPosition(),
            data.CheckpointPrevious(),
            data.CheckpointCfheader(),
            chain,
            data.DefaultCfilterType());
    } catch (const std::exception& e) {
        LogError()(__func__)(": ")(e.what()).Flush();

        return false;
    }
}

auto WriteCheckpoint(
    const std::filesystem::path& outputDirectory,
    const block::Position& current,
    const block::Position& prior,
    const cfilter::Header& cfheader,
    blockchain::Type chain,
    cfilter::Type type) noexcept -> bool
{
    try {
        std::filesystem::create_directories(outputDirectory);
        const auto id = std::to_string(static_cast<std::uint32_t>(chain));
        auto json = boost::json::object{};
        auto& chainData = json[id];
        auto& root = chainData.emplace_object();

        {
            auto& out = root["checkpoint"].emplace_object();
            {
                auto& position = out["position"].emplace_object();
                position["height"] = current.height_;
                position["hash"] = current.hash_.asHex();
            }
            {
                auto& position = out["previous"].emplace_object();
                position["height"] = prior.height_;
                position["hash"] = prior.hash_.asHex();
            }
            {
                out["cfheader"] = cfheader.asHex();
            }
        }
        {
            auto& out = root["predefined"].emplace_array();
            auto handle = get(chain).imp_->cfheaders_.lock();
            {
                auto& [block, map] = (*handle)[current.height_];
                block = current.hash_;
                map[type] = cfheader;
            }

            for (const auto& [height, data] : *handle) {
                const auto& [block, map] = data;
                auto& heightData =
                    out.emplace_back(boost::json::object{}).emplace_object();
                heightData["height"] = height;
                heightData["block"] = block.asHex();

                for (const auto& [fType, cpHeader] : map) {
                    const auto serializedType =
                        std::to_string(static_cast<std::uint32_t>(fType));
                    heightData[serializedType] = cpHeader.asHex();
                }
            }
        }

        const auto filename =
            std::filesystem::path{outputDirectory / ticker_symbol(chain)}
                .replace_extension("json");
        auto file = std::filebuf{};
        constexpr auto mode =
            std::ios::binary | std::ios::out | std::ios::trunc;

        if (nullptr == file.open(filename.c_str(), mode)) {
            throw std::runtime_error{"failed to open output file"};
        }

        {
            auto stream = std::ostream{std::addressof(file)};
            opentxs::print(json, stream);
        }

        file.close();

        return true;
    } catch (const std::exception& e) {
        LogError()(__func__)(": ")(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::params
