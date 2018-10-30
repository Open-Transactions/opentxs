// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace
{
extern "C" {
INTERNAL_PASSWORD_CALLBACK default_pass_cb;
INTERNAL_PASSWORD_CALLBACK souped_up_pass_cb;
}
}  // namespace

namespace opentxs::api::implementation
{
/** \brief Singlton class for providing an interface to process-level resources.
 *  \ingroup native
 */
class Native final : api::internal::Native, Lockable
{
public:
    const api::client::Manager& Client(const int instance) const override;
    std::size_t Clients() const override { return client_.size(); }
    const api::Settings& Config(const std::string& path) const override;
    const api::Crypto& Crypto() const override;
    void HandleSignals(ShutdownCallback* shutdown) const override;
    proto::RPCResponse RPC(const proto::RPCCommand& command) const override;
    const api::server::Manager& Server(const int instance) const override;
    std::size_t Servers() const override { return server_.size(); }
    const api::client::Manager& StartClient(
        const ArgList& args,
        const int instance) const override;
    const api::server::Manager& StartServer(
        const ArgList& args,
        const int instance,
        const bool inproc) const override;
    const api::network::ZAP& ZAP() const override;
    const opentxs::network::zeromq::Context& ZMQ() const override
    {
        return zmq_context_.get();
    }

    INTERNAL_PASSWORD_CALLBACK* GetInternalPasswordCallback() const override;
    OTCaller& GetPasswordCaller() const override;

private:
    friend opentxs::Factory;
    friend class opentxs::OT;

    typedef std::map<std::string, std::unique_ptr<api::Settings>> ConfigMap;

    Flag& running_;
    const std::chrono::seconds gc_interval_{0};
    OTPassword word_list_{};
    OTPassword passphrase_{};
    mutable std::mutex config_lock_;
    mutable std::mutex task_list_lock_;
    mutable std::mutex signal_handler_lock_;
    mutable ConfigMap config_;
    OTZMQContext zmq_context_;
    mutable std::unique_ptr<Signals> signal_handler_;
    std::unique_ptr<api::internal::Log> log_;
    std::unique_ptr<api::Crypto> crypto_;
    std::unique_ptr<api::Legacy> legacy_;
    mutable std::vector<std::unique_ptr<api::client::internal::Manager>>
        client_;
    mutable std::vector<std::unique_ptr<api::server::Manager>> server_;
    std::unique_ptr<api::network::ZAP> zap_;
    const ArgList server_args_;
    mutable ShutdownCallback* shutdown_callback_{nullptr};
    std::unique_ptr<OTCallback> null_callback_{nullptr};
    std::unique_ptr<OTCaller> default_external_password_callback_{nullptr};
    OTCaller* external_password_callback_{nullptr};
    std::unique_ptr<rpc::internal::RPC> rpc_;
    mutable std::unique_ptr<opentxs::PIDFile> pid_;

    static int client_instance(const int count);
    static int server_instance(const int count);
    static std::string get_arg(const ArgList& args, const std::string& argName);

    void init_pid(const Lock& lock) const;
    const ArgList merge_arglist(const ArgList& args) const;
    void start_client(const Lock& lock, const ArgList& args) const;
    void start_server(const Lock& lock, const ArgList& args) const;

    void Init_Crypto();
    void Init_Log();
    void Init_Zap();
    void Init() override;
    void setup_default_external_password_callback();
    void shutdown() override;

    explicit Native(
        Flag& running,
        const ArgList& args,
        const std::chrono::seconds gcInterval,
        OTCaller* externalPasswordCallback = nullptr);
    Native() = delete;
    Native(const Native&) = delete;
    Native(Native&&) = delete;
    Native& operator=(const Native&) = delete;
    Native& operator=(Native&&) = delete;

    ~Native();
};
}  // namespace opentxs::api::implementation
