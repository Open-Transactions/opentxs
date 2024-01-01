// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/StoragePaymentWorkflows.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/StoragePaymentWorkflows.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/StorageItemHash.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/StorageWorkflowIndex.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/StorageWorkflowType.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyStorage.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const StoragePaymentWorkflows& input, const Log& log) -> bool
{
    CHECK_SUBOBJECTS(workflow, StoragePaymentWorkflowsAllowedStorageItemHash());
    CHECK_SUBOBJECTS(
        items, StoragePaymentWorkflowsAllowedStorageWorkflowIndex());
    CHECK_SUBOBJECTS(
        accounts, StoragePaymentWorkflowsAllowedStorageWorkflowIndex());
    CHECK_SUBOBJECTS(
        units, StoragePaymentWorkflowsAllowedStorageWorkflowIndex());
    CHECK_IDENTIFIERS(archived);
    CHECK_SUBOBJECTS(
        types, StoragePaymentWorkflowsAllowedStoragePaymentWorkflowType());

    if (input.workflow_size() != input.types_size()) {
        FAIL_4(
            "Wrong number of index objects. Workflows: ",
            input.workflow_size(),
            " Index objects: ",
            input.types_size());
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
