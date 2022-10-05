# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(verify)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/serialization/protobuf/Basic.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/serialization/protobuf/Check.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/serialization/protobuf/Contact.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/serialization/protobuf/Proto.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/serialization/protobuf/Proto.tpp"
    "Proto.cpp"
)

target_link_libraries(opentxs-common PRIVATE messages)
