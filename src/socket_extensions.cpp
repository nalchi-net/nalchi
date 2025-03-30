#include "nalchi/socket_extensions.hpp"

namespace nalchi
{

NALCHI_API void socket_extensions::unicast(ISteamNetworkingSockets* sockets, HSteamNetConnection connection,
                                           nalchi::shared_payload payload, int logical_bytes_length, int send_flags,
                                           std::int64_t* out_message_number_or_result, std::uint16_t lane,
                                           std::int64_t user_data)
{
    // TODO: Pool message instead of allocating.
    SteamNetworkingMessage_t* msg = SteamNetworkingUtils()->AllocateMessage(0);

    // Setup the message to send to `conn`.
    payload.add_to_message(msg, logical_bytes_length);
    msg->m_conn = connection;
    msg->m_nFlags = send_flags;
    msg->m_idxLane = lane;
    msg->m_nUserData = user_data;

    // Send the message.
    sockets->SendMessages(1, &msg, reinterpret_cast<int64*>(out_message_number_or_result));
}

NALCHI_API void socket_extensions::multicast(ISteamNetworkingSockets* sockets, unsigned connections_count,
                                             const HSteamNetConnection* connections, nalchi::shared_payload payload,
                                             int logical_bytes_length, int send_flags,
                                             std::int64_t* out_message_number_or_result, std::uint16_t lane,
                                             std::int64_t user_data)
{
    return multicast(sockets, std::span<const HSteamNetConnection>(connections, connections_count), payload,
                     logical_bytes_length, send_flags,
                     std::span<std::int64_t>(out_message_number_or_result, connections_count), lane, user_data);
}

} // namespace nalchi
