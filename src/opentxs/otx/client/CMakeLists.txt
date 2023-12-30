# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("otx/client" "Depositability.hpp")
libopentxs_add_public_header("otx/client" "Messagability.hpp")
libopentxs_add_public_header("otx/client" "PaymentType.hpp")
libopentxs_add_public_header("otx/client" "PaymentWorkflowState.hpp")
libopentxs_add_public_header("otx/client" "PaymentWorkflowType.hpp")
libopentxs_add_public_header("otx/client" "RemoteBoxType.hpp")
libopentxs_add_public_header("otx/client" "SendResult.hpp")
libopentxs_add_public_header("otx/client" "StorageBox.hpp")
libopentxs_add_public_header("otx/client" "ThreadStatus.hpp")
libopentxs_add_public_header("otx/client" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "Depositability.cpp"
    "Messagability.cpp"
    "PaymentType.cpp"
    "PaymentWorkflowState.cpp"
    "PaymentWorkflowType.cpp"
    "RemoteBoxType.cpp"
    "SendResult.cpp"
    "StorageBox.cpp"
    "ThreadStatus.cpp"
    "Types.cpp"
)
