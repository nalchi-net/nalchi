#pragma once

#include "nalchi/alloca.hpp"
#include "nalchi/export.hpp"
#include "nalchi/shared_payload.hpp"
#include "nalchi/typed_input_range.hpp"

#include <steam/isteamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingtypes.h>

#include <span>
#include <type_traits>

namespace nalchi
{

/// @brief Extensions for `ISteamNetworkingSockets`.
class socket_extensions
{
public:
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
    NALCHI_API static void unicast(ISteamNetworkingSockets* sockets, HSteamNetConnection connection,
                                   nalchi::shared_payload payload, int logical_bytes_length, int send_flags,
                                   std::int64_t* out_message_number_or_result);

    /// @brief Multicasts a `shared_payload` to the connections.
    ///
    /// This function uses <a
    /// href="https://partner.steamgames.com/doc/api/ISteamNetworkingSockets#SendMessages"
    /// >`ISteamNetworkingSockets::SendMessages()`</a> under the hood, but it shares the payload between them. \n
    ///  So, it's more efficient if you send a same message to a lot of connections with this.
    /// @tparam ConnectionRange Connection range type that can take any iterable range of `HSteamNetConnection`.
    /// @param connections Connections to multicast to.
    /// @param payload Payload to send.
    /// @param logical_bytes_length Logical number of bytes of the payload.
    /// @param send_flags Send flags. See <a
    /// href="https://partner.steamgames.com/doc/api/steamnetworkingtypes#message_sending_flags" >message sending
    /// flags</a> on the Steamworks docs.
    /// @param out_message_number_or_result Optional pointer to receive the message number if successful,
    /// or a negative `EResult` value if failed.
    template <typed_input_range<HSteamNetConnection> ConnectionRange>
    static void multicast(ISteamNetworkingSockets* sockets, ConnectionRange&& connections,
                          nalchi::shared_payload payload, int logical_bytes_length, int send_flags,
                          std::span<std::int64_t> out_message_number_or_result)
    {
        const auto connections_count = std::ranges::size(connections);

        // No dynamic allocation vs. Stack overflow safety.
        // I chose the former, because I think it's not common to multicast to more than 10k connections at once.
        auto** messages = static_cast<SteamNetworkingMessage_t**>(
            NALCHI_ALLOCA(connections_count * sizeof(SteamNetworkingMessage_t*)));

        std::remove_const_t<decltype(connections_count)> i = 0;
        for (const auto conn : connections)
        {
            // TODO: Pool message instead of allocating.
            messages[i] = SteamNetworkingUtils()->AllocateMessage(0);

            // Setup the message to send to `conn`.
            payload.add_to_message(messages[i], logical_bytes_length);
            messages[i]->m_conn = conn;
            messages[i]->m_nFlags = send_flags;

            ++i;
        }

        // Send all messages.
        sockets->SendMessages(static_cast<int>(connections_count), messages,
                              reinterpret_cast<int64*>(out_message_number_or_result.data()));
    }

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
    NALCHI_API static void multicast(ISteamNetworkingSockets* sockets, unsigned connections_count,
                                     const HSteamNetConnection* connections, nalchi::shared_payload payload,
                                     int logical_bytes_length, int send_flags,
                                     std::int64_t* out_message_number_or_result);
};

} // namespace nalchi
