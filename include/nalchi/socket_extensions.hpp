#pragma once

#include "nalchi/alloca.hpp"
#include "nalchi/export.hpp"
#include "nalchi/shared_payload.hpp"
#include "nalchi/typed_input_range.hpp"

#include <steam/isteamnetworkingsockets.h>
#include <steam/steamnetworkingtypes.h>

#include <cstdint>
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
    /// @param lane Optional lane index. See <a
    /// href="https://partner.steamgames.com/doc/api/ISteamNetworkingSockets#ConfigureConnectionLanes"
    /// >`ISteamNetworkingSockets::ConfigureConnectionLanes`</a> for details.
    /// @param user_data Optional user data.
    NALCHI_API static void unicast(ISteamNetworkingSockets* sockets, HSteamNetConnection connection,
                                   nalchi::shared_payload payload, int logical_bytes_length, int send_flags,
                                   std::int64_t* out_message_number_or_result, std::uint16_t lane = 0,
                                   std::int64_t user_data = 0);

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
    /// @param lane Optional lane index. See <a
    /// href="https://partner.steamgames.com/doc/api/ISteamNetworkingSockets#ConfigureConnectionLanes"
    /// >`ISteamNetworkingSockets::ConfigureConnectionLanes`</a> for details.
    /// @param user_data Optional user data.
    template <typed_input_range<HSteamNetConnection> ConnectionRange>
    static void multicast(ISteamNetworkingSockets* sockets, ConnectionRange&& connections,
                          nalchi::shared_payload payload, int logical_bytes_length, int send_flags,
                          std::span<std::int64_t> out_message_number_or_result, std::uint16_t lane = 0,
                          std::int64_t user_data = 0)
    {
        const auto connections_count = std::ranges::size(connections);

        // No dynamic allocation vs. Stack overflow safety.
        // I chose the former, because I think it's not common to multicast to more than 10k connections at once.
        auto** messages = static_cast<SteamNetworkingMessage_t**>(
            NALCHI_ALLOCA(connections_count * sizeof(SteamNetworkingMessage_t*)));

        std::remove_const_t<decltype(connections_count)> i = 0;
        for (const auto conn : connections)
        {
            // Pool `CSteamNetworkingMessage` instead of
            // allocating it via `SteamNetworkingUtils()->AllocateMessage(0)`.
            messages[i] = allocate_message();

            // Setup the message to send to `conn`.
            payload.add_to_message(messages[i], logical_bytes_length);
            messages[i]->m_conn = conn;
            messages[i]->m_nFlags = send_flags;
            messages[i]->m_idxLane = lane;
            messages[i]->m_nUserData = user_data;

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
    /// @param lane Optional lane index. See <a
    /// href="https://partner.steamgames.com/doc/api/ISteamNetworkingSockets#ConfigureConnectionLanes"
    /// >`ISteamNetworkingSockets::ConfigureConnectionLanes`</a> for details.
    /// @param user_data Optional user data.
    NALCHI_API static void multicast(ISteamNetworkingSockets* sockets, unsigned connections_count,
                                     const HSteamNetConnection* connections, nalchi::shared_payload payload,
                                     int logical_bytes_length, int send_flags,
                                     std::int64_t* out_message_number_or_result, std::uint16_t lane = 0,
                                     std::int64_t user_data = 0);

private:
    /// @brief Pool `CSteamNetworkingMessage` instead of allocating it
    /// via `SteamNetworkingUtils()->AllocateMessage(0)` in 64-bit environments.
    ///
    /// `SteamNetworkingUtils()->AllocateMessage(0)` will
    /// `new` & `delete` the `CSteamNetworkingMessage`, which is 264 bytes. \n
    /// That's bad, so I provide my own pooled allocation function.
    NALCHI_API static auto allocate_message() -> SteamNetworkingMessage_t*;

    /// @brief Release `SteamNetworkingMessage_t` to the pool.
    static void release_message(SteamNetworkingMessage_t*);
};

} // namespace nalchi
