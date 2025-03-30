#include "nalchi/socket_extensions_flat.hpp"

NALCHI_FLAT_API void nalchi_socket_extensions_unicast(ISteamNetworkingSockets* sockets, HSteamNetConnection connection,
                                                      nalchi::shared_payload payload, int logical_bytes_length,
                                                      int send_flags, std::int64_t* out_message_number_or_result,
                                                      std::uint16_t lane, std::int64_t user_data)
{
    return nalchi::socket_extensions::unicast(sockets, connection, payload, logical_bytes_length, send_flags,
                                              out_message_number_or_result, lane, user_data);
}

NALCHI_FLAT_API void nalchi_socket_extensions_multicast(ISteamNetworkingSockets* sockets, unsigned connections_count,
                                                        const HSteamNetConnection* connections,
                                                        nalchi::shared_payload payload, int logical_bytes_length,
                                                        int send_flags, std::int64_t* out_message_number_or_result,
                                                        std::uint16_t lane, std::int64_t user_data)
{
    return nalchi::socket_extensions::multicast(sockets, connections_count, connections, payload, logical_bytes_length,
                                                send_flags, out_message_number_or_result, lane, user_data);
}
