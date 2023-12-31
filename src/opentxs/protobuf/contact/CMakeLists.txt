# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common PRIVATE "Types.internal.hash.cpp" "Types.internal.hpp"
)

add_library(
  opentxs-serialization-protobuf-contact OBJECT
  "AllowedItemAttributes.cpp"
  "AllowedItemTypes.cpp"
  "AllowedSectionNames.cpp"
  "AllowedSubtypes.cpp"
  "ContactItemAttributes.cpp"
  "ContactItemTypes.cpp"
  "ContactSectionNames.cpp"
  "RelationshipMap.cpp"
)
target_include_directories(
  opentxs-serialization-protobuf-contact SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/external/frozen/include"
)
target_link_libraries(
  opentxs-serialization-protobuf-contact PRIVATE opentxs-common
                                                 opentxs-protobuf
)
libopentxs_configure_constexpr_target(opentxs-serialization-protobuf-contact)
