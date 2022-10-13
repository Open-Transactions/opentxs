// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/serialization/protobuf/Proto.hpp"  // IWYU pragma: associated

#include <cassert>
#include <cstddef>
#include <iostream>
#include <limits>
#include <memory>

#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::proto
{
auto ToString(const ProtobufType& input) -> UnallocatedCString;

template <typename Output>
auto Factory(const void* input, const std::size_t size) -> Output;
template <typename Output, typename Input>
auto Factory(const Pimpl<Input>& input) -> Output;
template <typename Output, typename Input>
auto Factory(const Input& input) -> Output;

template <typename Output>
auto Factory(const void* input, const std::size_t size) -> Output
{
    static_assert(sizeof(int) <= sizeof(std::size_t));
    static constexpr auto max =
        static_cast<std::size_t>(std::numeric_limits<int>::max());

    if (size > max) {
        LogAbort()("opentxs::proto::")(__func__)(
            ": attempted to construct protobuf from ")(size)(" byte array")
            .Abort();
    }

    auto serialized = Output{};
    serialized.ParseFromArray(input, static_cast<int>(size));

    return serialized;
}

template <typename Output, typename Input>
auto Factory(const Pimpl<Input>& input) -> Output
{
    return Factory<Output>(input.get());
}

template <typename Output, typename Input>
auto Factory(const Input& input) -> Output
{
    return Factory<Output>(input.data(), input.size());
}

template <typename Output>
auto DynamicFactory(const void* input, const std::size_t size)
    -> std::unique_ptr<Output>
{
    if (std::numeric_limits<int>::max() < size) {
        std::cerr << __func__ << ": input too large\n";

        return {};
    }

    auto output = std::make_unique<Output>();

    if (output) { output->ParseFromArray(input, static_cast<int>(size)); }

    return output;
}

template <typename Output, typename Input>
auto DynamicFactory(const Pimpl<Input>& input) -> std::unique_ptr<Output>
{
    return DynamicFactory<Output>(input.get());
}

template <typename Output, typename Input>
auto DynamicFactory(const Input& input) -> std::unique_ptr<Output>
{
    return DynamicFactory<Output>(input.data(), input.size());
}

template <typename Output>
auto StringToProto(const String& input) -> Output
{
    auto armored = Armored::Factory();
    OTString unconstInput = String::Factory(input.Get());

    if (!armored->LoadFromString(unconstInput)) {
        std::cerr << __func__ << ": failed to decode armored protobuf\n";

        return Output();
    } else {

        return Factory<Output>(ByteArray{armored});
    }
}
}  // namespace opentxs::proto
