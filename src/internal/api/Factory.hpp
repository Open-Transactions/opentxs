// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/api/Context.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace internal
{
class Context;
class Log;
}  // namespace internal

namespace session
{
class Storage;
}  // namespace session

class Crypto;
class Legacy;
class Settings;
}  // namespace api

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

class Flag;
class Options;
class PasswordCaller;
class String;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto Context(
    Flag& running,
    const Options& args,
    PasswordCaller* externalPasswordCallback = nullptr) noexcept
    -> std::unique_ptr<api::internal::Context>;
auto Legacy(const UnallocatedCString& home) noexcept
    -> std::unique_ptr<api::Legacy>;
auto Log(
    const network::zeromq::Context& zmq,
    const UnallocatedCString& endpoint) noexcept
    -> std::unique_ptr<api::internal::Log>;
auto FactoryAPI(const api::Crypto& crypto) noexcept
    -> std::unique_ptr<api::Factory>;
auto Settings(const api::Legacy& legacy, const String& path) noexcept
    -> std::unique_ptr<api::Settings>;
}  // namespace opentxs::factory
