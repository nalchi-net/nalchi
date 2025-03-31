#include "nalchi/socket_extensions.hpp"

#if defined(NALCHI_POOL_MSGS)
#define NB_OBJ_POOL_CHECK false
#include "CSteamNetworkingMessage.hpp"
#include <NetBuff/LockfreeObjectPool.hpp>
#include <cassert>
#else
#include <steam/isteamnetworkingutils.h>
#endif

namespace nalchi
{

#if defined(NALCHI_POOL_MSGS)
static nb::LockfreeObjectPool<SteamNetworkingSocketsLib::CSteamNetworkingMessage, true> g_msg_pool;
#endif

NALCHI_API void socket_extensions::unicast(ISteamNetworkingSockets* sockets, HSteamNetConnection connection,
                                           nalchi::shared_payload payload, int logical_bytes_length, int send_flags,
                                           std::int64_t* out_message_number_or_result, std::uint16_t lane,
                                           std::int64_t user_data)
{
    // Pool `CSteamNetworkingMessage` instead of
    // allocating it via `SteamNetworkingUtils()->AllocateMessage(0)`.
    SteamNetworkingMessage_t* msg = allocate_message();

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

NALCHI_API auto socket_extensions::allocate_message() -> SteamNetworkingMessage_t*
{
#if defined(NALCHI_POOL_MSGS)
    SteamNetworkingSocketsLib::CSteamNetworkingMessage& msg = g_msg_pool.construct();

    msg.m_cbSize = 0;
    msg.m_pData = nullptr;
    msg.m_pfnFreeData = nullptr;

    // Clear identity
    // msg.m_conn = k_HSteamNetConnection_Invalid;  // will be set on uni/multicast
    msg.m_identityPeer.m_eType = k_ESteamNetworkingIdentityType_Invalid;
    msg.m_identityPeer.m_cbSize = 0;

    // Set the release function
    msg.m_pfnRelease = release_message;

    // Clear these fields
    msg.m_nConnUserData = 0;
    msg.m_usecTimeReceived = 0;
    msg.m_nMessageNumber = 0;
    msg.m_nChannel = -1;
    // msg.m_nFlags = 0;   // will be set on uni/multicast
    // msg.m_idxLane = 0;  // will be set on uni/multicast

    msg.m_links.Clear();
    msg.m_linksSecondaryQueue.Clear();

    return &msg;
#else
    return SteamNetworkingUtils()->AllocateMessage(0);
#endif
}

void socket_extensions::release_message(SteamNetworkingMessage_t* msg)
{
#if defined(NALCHI_POOL_MSGS)
    SteamNetworkingSocketsLib::CSteamNetworkingMessage* c_msg =
        static_cast<SteamNetworkingSocketsLib::CSteamNetworkingMessage*>(msg);

    // Free up the buffer, if we have one
    if (c_msg->m_pData && c_msg->m_pfnFreeData)
        (*c_msg->m_pfnFreeData)(c_msg);
    c_msg->m_pData = nullptr; // Just for grins

    // We must not currently be in any queue.  In fact, our parent
    // might have been destroyed.
    assert(!c_msg->m_links.m_pQueue);
    assert(!c_msg->m_links.m_pPrev);
    assert(!c_msg->m_links.m_pNext);
    assert(!c_msg->m_linksSecondaryQueue.m_pQueue);
    assert(!c_msg->m_linksSecondaryQueue.m_pPrev);
    assert(!c_msg->m_linksSecondaryQueue.m_pNext);

    // Self destruct
    g_msg_pool.destroy(*static_cast<SteamNetworkingSocketsLib::CSteamNetworkingMessage*>(msg));
#else
    msg->Release();
#endif
}

} // namespace nalchi
