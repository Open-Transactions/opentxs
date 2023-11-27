# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(network)
add_subdirectory(session)

libopentxs_add_public_header("api" "Context.hpp")
libopentxs_add_public_header("api" "Factory.hpp")
libopentxs_add_public_header("api" "Network.hpp")
libopentxs_add_public_header("api" "Periodic.hpp")
libopentxs_add_public_header("api" "Session.hpp")
libopentxs_add_public_header("api" "Settings.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "Context.cpp"
    "Context.factory.cpp"
    "Context.internal.cpp"
    "Context.internal.hpp"
    "ContextPrivate.cpp"
    "ContextPrivate.hpp"
    "Factory.cpp"
    "Factory.factory.cpp"
    "Factory.internal.cpp"
    "Factory.internal.hpp"
    "FactoryPrivate.cpp"
    "FactoryPrivate.hpp"
    "Log.factory.cpp"
    "Log.internal.cpp"
    "Log.internal.hpp"
    "LogPrivate.cpp"
    "LogPrivate.hpp"
    "Network.cpp"
    "Network.factory.cpp"
    "Network.internal.cpp"
    "Network.internal.hpp"
    "NetworkPrivate.cpp"
    "NetworkPrivate.hpp"
    "Paths.factory.cpp"
    "Paths.internal.cpp"
    "Paths.internal.hpp"
    "PathsPrivate.cpp"
    "PathsPrivate.hpp"
    "Periodic.cpp"
    "PeriodicPrivate.cpp"
    "PeriodicPrivate.hpp"
    "Session.cpp"
    "Session.internal.cpp"
    "Session.internal.hpp"
    "SessionPrivate.cpp"
    "SessionPrivate.hpp"
    "Settings.cpp"
    "Settings.factory.cpp"
    "Settings.internal.cpp"
    "Settings.internal.hpp"
    "SettingsPrivate.cpp"
    "SettingsPrivate.hpp"
    "internal.factory.hpp"
)

libopentxs_add_platform_specific("ContextPrivate")
libopentxs_add_platform_specific("LogPrivate")
libopentxs_add_platform_specific("PathsPrivate")

if(OPENTXS_DEBUG_BUILD)
  target_sources(opentxs-common PRIVATE "ContextPrivate.debug.cpp")
else()
  libopentxs_add_platform_specific("ContextPrivate.release")
endif()

if(OT_ENABLE_RPC)
  target_sources(opentxs-common PRIVATE "ContextPrivate.rpc.cpp")
else()
  target_sources(opentxs-common PRIVATE "ContextPrivate.no-rpc.cpp")
endif()

if(OT_QT_EXPORT)
  target_sources(opentxs-common PRIVATE "ContextPrivate.qt.cpp")
  libopentxs_link_internal(Qt::Core)
  libopentxs_link_external(Qt::Core)
else()
  target_sources(opentxs-common PRIVATE "ContextPrivate.no-qt.cpp")
endif()
