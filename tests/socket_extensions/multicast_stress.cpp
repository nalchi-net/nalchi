#include "../assert.hpp"
#include "../init_and_kill.hpp"

#include <nalchi/socket_extensions.hpp>

#include <steam/isteamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

#ifndef MC_MESSAGES
#define MC_MESSAGES 5000
#endif

namespace nalchi::tests
{

using msg_t = int;

constexpr auto sleep_duration = std::chrono::milliseconds(16);

int g_clients;
int g_messages;

void do_server(const std::vector<HSteamNetConnection>& conns)
{
    std::vector<std::int64_t> out_message_number_or_result(conns.size());

    for (int i = 0; i < g_messages; ++i)
    {
        // Prepare a payload to send
        auto payload = shared_payload::allocate(sizeof(msg_t));
        NALCHI_TESTS_ASSERT(payload.ptr, "Payload allocation failed");

        int& data = *static_cast<int*>(payload.ptr);
        data = i;

        // Multicast the payload
        socket_extensions::multicast(SteamNetworkingSockets(), conns, payload, sizeof(msg_t),
                                     k_nSteamNetworkingSend_Reliable, out_message_number_or_result);

        // Check the out message numbers
        for (const auto number : out_message_number_or_result)
            NALCHI_TESTS_ASSERT(number >= 0, "Multicast failed with ", -number);
    }
}

void do_client(const HSteamNetConnection conn)
{
    static constexpr std::size_t BATCH_MSGS = 256;
    std::array<SteamNetworkingMessage_t*, BATCH_MSGS> msgs;

    for (int msg_idx = 0; msg_idx < g_messages;)
    {
        const int recv_cnt = SteamNetworkingSockets()->ReceiveMessagesOnConnection(conn, msgs.data(), BATCH_MSGS);
        NALCHI_TESTS_ASSERT(recv_cnt != -1);

        for (int i = 0; i < recv_cnt; ++i)
        {
            NALCHI_TESTS_ASSERT(msgs[i]->m_cbSize == sizeof(msg_t));
            const int data = *((int*)msgs[i]->m_pData);
            NALCHI_TESTS_ASSERT(data == msg_idx + i, "Received ", data, ", expected ", msg_idx + i);

            msgs[i]->Release();
        }

        msg_idx += recv_cnt;
    }
}

void configure_buffer_sizes()
{
    auto& net_utils = *SteamNetworkingUtils();
    std::size_t config_value_size = sizeof(int);

    // Configure send buffer size
    int send_buffer_size;
    auto get_config_result =
        net_utils.GetConfigValue(k_ESteamNetworkingConfig_SendBufferSize, k_ESteamNetworkingConfig_Global, 0, nullptr,
                                 &send_buffer_size, &config_value_size);
    NALCHI_TESTS_ASSERT(get_config_result == k_ESteamNetworkingGetConfigValue_OK ||
                            get_config_result == k_ESteamNetworkingGetConfigValue_OKInherited,
                        "Get send buffer size failed: ", (int)get_config_result);
    NALCHI_TESTS_ASSERT(config_value_size == sizeof(int));

    if (static_cast<std::size_t>(send_buffer_size) < sizeof(msg_t) * g_messages)
    {
        const bool set_config_result =
            net_utils.SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_SendBufferSize, sizeof(msg_t) * g_messages);
        NALCHI_TESTS_ASSERT(set_config_result, "Set send buffer size failed");
        std::cout << "Send buffer size changed: " << sizeof(msg_t) * g_messages << " (was " << send_buffer_size
                  << ")\n";
    }
    else
        std::cout << "Send buffer size: " << send_buffer_size << '\n';

    // Configure recv buffer size
    int recv_buffer_size;
    get_config_result =
        net_utils.GetConfigValue(k_ESteamNetworkingConfig_RecvBufferSize, k_ESteamNetworkingConfig_Global, 0, nullptr,
                                 &recv_buffer_size, &config_value_size);
    NALCHI_TESTS_ASSERT(get_config_result == k_ESteamNetworkingGetConfigValue_OK ||
                            get_config_result == k_ESteamNetworkingGetConfigValue_OKInherited,
                        "Get recv buffer size failed: ", (int)get_config_result);
    NALCHI_TESTS_ASSERT(config_value_size == sizeof(int));

    if (static_cast<std::size_t>(recv_buffer_size) < sizeof(msg_t) * g_messages)
    {
        const bool set_config_result =
            net_utils.SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_RecvBufferSize, sizeof(msg_t) * g_messages);
        NALCHI_TESTS_ASSERT(set_config_result, "Set recv buffer size failed");
        std::cout << "Recv buffer size changed: " << sizeof(msg_t) * g_messages << " (was " << recv_buffer_size
                  << ")\n";
    }
    else
        std::cout << "Recv buffer size: " << recv_buffer_size << '\n';

    // Configure recv messages count
    int recv_msg_count;
    get_config_result =
        net_utils.GetConfigValue(k_ESteamNetworkingConfig_RecvBufferMessages, k_ESteamNetworkingConfig_Global, 0,
                                 nullptr, &recv_msg_count, &config_value_size);
    NALCHI_TESTS_ASSERT(get_config_result == k_ESteamNetworkingGetConfigValue_OK ||
                            get_config_result == k_ESteamNetworkingGetConfigValue_OKInherited,
                        "Get recv messages count failed: ", (int)get_config_result);
    NALCHI_TESTS_ASSERT(config_value_size == sizeof(int));

    if (recv_msg_count < g_messages)
    {
        const bool set_config_result =
            net_utils.SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_RecvBufferMessages, g_messages);
        NALCHI_TESTS_ASSERT(set_config_result, "Set recv msg count failed");
        std::cout << "Max recv msg count changed: " << g_messages << " (was " << recv_msg_count << ")\n";
    }
    else
        std::cout << "Max recv msg count: " << recv_msg_count << '\n';
}

} // namespace nalchi::tests

int main(int argc, char** argv)
{
    if (argc > 2)
    {
        std::cout << "=== Usage ===\n";
        std::cout << "`./test_multicast_stress`\n";
        std::cout << '\t' << "Test multicasting " << MC_MESSAGES << " messages.\n";
        std::cout << "`./test_multicast_stress` <count>\n";
        std::cout << '\t' << "Test multicasting <count> messages.\n";
        return 2;
    }

    using namespace nalchi;
    using namespace nalchi::tests;

    std::cout << "=== multicast stress test ===\n";

    g_messages = (argc == 2) ? std::atoi(argv[1]) : MC_MESSAGES;
    g_clients = std::max(2, static_cast<int>(std::thread::hardware_concurrency()) - 2);

    std::cout << "Prepare to send " << g_messages << " messages to " << g_clients << " client threads...\n";

    NALCHI_TESTS_ASSERT(gns_init(), "GNS init failed");

    std::atomic<bool> stop_callback;

    std::thread callback_worker([&stop_callback]() {
        while (!stop_callback.load(std::memory_order_acquire))
        {
#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
            SteamNetworkingSockets()->RunCallbacks();
#else // Steamworks SDK
            SteamAPI_RunCallbacks();
#endif
            std::this_thread::sleep_for(sleep_duration);
        }
    });

    configure_buffer_sizes();

    // Create connections
    std::vector<HSteamNetConnection> server_side_connections(g_clients);
    std::vector<HSteamNetConnection> client_side_connections(g_clients);

    for (int i = 0; i < g_clients; ++i)
    {
        auto& server = server_side_connections[i];
        auto& client = client_side_connections[i];

        const bool created = SteamNetworkingSockets()->CreateSocketPair(&server, &client, false, nullptr, nullptr);
        NALCHI_TESTS_ASSERT(created, "Connection creation failed");
    }

    // Create worker threads
    std::vector<std::thread> workers;
    workers.reserve(1 + g_clients);

    workers.emplace_back(do_server, server_side_connections);
    for (int i = 0; i < g_clients; ++i)
        workers.emplace_back(do_client, client_side_connections[i]);

    // Wait for the jobs to complete
    for (auto& worker : workers)
        worker.join();

    // Clean up
    for (int i = 0; i < g_clients; ++i)
    {
        SteamNetworkingSockets()->CloseConnection(client_side_connections[i], 0, nullptr, false);
        SteamNetworkingSockets()->CloseConnection(server_side_connections[i], 0, nullptr, false);
    }

    stop_callback.store(true, std::memory_order_release);
    callback_worker.join();

    gns_kill();

    std::cout << "multicast stress test succeeded" << std::endl;
}
