/// @file
/// @brief Socket extensions flat API.

#pragma once

#include "nalchi/socket_extensions.hpp"

/// @brief Unicasts a `shared_payload` to a connection.
///
/// This function is pretty similar to the <a
/// href="https://partner.steamgames.com/doc/api/ISteamNetworkingSockets#SendMessageToConnection"
/// >`ISteamNetworkingSockets::SendMessageToConnection()`</a>, but it's for the `nalchi::shared_payload`. \n
/// But, it uses <a
/// href="https://partner.steamgames.com/doc/api/ISteamNetworkingSockets#SendMessages"
/// >`ISteamNetworkingSockets::SendMessages()`</a> under the hood, so the result is returns with @p
/// out_message_number_or_result instead of a return value.
/// @param connection Connection to send to.
/// @param payload Payload to send.
/// @param logical_bytes_length Logical number of bytes of the payload.
/// @param send_flags Send flags. See <a
/// href="https://partner.steamgames.com/doc/api/steamnetworkingtypes#message_sending_flags" >message sending
/// flags</a> on the Steamworks docs.
/// @param out_message_number_or_result Optional pointer to receive the message number if successful,
/// or a negative `EResult` value if failed.
/// @param lane Optional lane index. See <a
/// href="https://partner.steamgames.com/doc/api/ISteamNetworkingSockets#ConfigureConnectionLanes"
/// >`ISteamNetworkingSockets::ConfigureConnectionLanes`</a> for details.
/// @param user_data Optional user data.
NALCHI_FLAT_API void nalchi_socket_extensions_unicast(ISteamNetworkingSockets* sockets, HSteamNetConnection connection,
                                                      nalchi::shared_payload payload, int logical_bytes_length,
                                                      int send_flags, std::int64_t* out_message_number_or_result,
                                                      std::uint16_t lane, std::int64_t user_data);

/// @brief Multicasts a `shared_payload` to the connections.
///
/// This function uses <a
/// href="https://partner.steamgames.com/doc/api/ISteamNetworkingSockets#SendMessages"
/// >`ISteamNetworkingSockets::SendMessages()`</a> under the hood, but it shares the payload between them. \n
/// So, it's more efficient if you send a same message to a lot of connections with this.
/// @param connections_count Number of @p connections.
/// @param connections Connections to multicast to.
/// @param payload Payload to send.
/// @param logical_bytes_length Logical number of bytes of the payload.
/// @param send_flags Send flags. See <a
/// href="https://partner.steamgames.com/doc/api/steamnetworkingtypes#message_sending_flags" >message sending
/// flags</a> on the Steamworks docs.
/// @param out_message_number_or_result Optional pointer to receive the message number if successful,
/// or a negative `EResult` value if failed.
/// @param lane Optional lane index. See <a
/// href="https://partner.steamgames.com/doc/api/ISteamNetworkingSockets#ConfigureConnectionLanes"
/// >`ISteamNetworkingSockets::ConfigureConnectionLanes`</a> for details.
/// @param user_data Optional user data.
NALCHI_FLAT_API void nalchi_socket_extensions_multicast(ISteamNetworkingSockets* sockets, unsigned connections_count,
                                                        const HSteamNetConnection* connections,
                                                        nalchi::shared_payload payload, int logical_bytes_length,
                                                        int send_flags, std::int64_t* out_message_number_or_result,
                                                        std::uint16_t lane, std::int64_t user_data);
