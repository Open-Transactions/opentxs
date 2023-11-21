// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/internal.factory.hpp"  // IWYU pragma: associated

#include <exception>
#include <stdexcept>
#include <utility>

#include "opentxs/api/session/NotaryPrivate.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "otx/common/OTStorage.hpp"

namespace opentxs::factory
{
auto NotarySession(
    const api::Context& parent,
    Flag& running,
    Options&& args,
    const api::Crypto& crypto,
    const api::Settings& config,
    const opentxs::network::zeromq::Context& context,
    const std::filesystem::path& dataFolder,
    const int instance) -> std::shared_ptr<api::session::internal::Notary>
{
    using ReturnType = api::session::NotaryPrivate;

    try {
        auto output = std::make_shared<ReturnType>(
            parent,
            running,
            std::move(args),
            crypto,
            config,
            context,
            dataFolder,
            instance);

        if (output) {
            try {
                output->Init(output);

                return output;
            } catch (const std::invalid_argument& e) {
                LogError()()(
                    "There was a problem creating the server. The "
                    "server contract will be deleted. Error: ")(e.what())
                    .Flush();
                const auto datafolder = output->DataFolder().string();
                OTDB::EraseValueByKey(
                    output->Self(),
                    datafolder,
                    ".",
                    "NEW_SERVER_CONTRACT.otc",
                    "",
                    "");
                OTDB::EraseValueByKey(
                    output->Self(),
                    datafolder,
                    ".",
                    "notaryServer.xml",
                    "",
                    "");
                OTDB::EraseValueByKey(
                    output->Self(),
                    datafolder,
                    ".",
                    "seed_backup.json",
                    "",
                    "");
                std::rethrow_exception(std::current_exception());
            }
        } else {

            throw std::runtime_error{"failed to instantiate notary"};
        }
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory
