// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/Proto.tpp"

namespace opentxs::proto
{
std::string ToString(const ProtobufType& input)
{
    std::string output{};
    output.resize(input.ByteSize());
    input.SerializeToArray(output.data(), output.size());

    return output;
}
}  // namespace opentxs::proto