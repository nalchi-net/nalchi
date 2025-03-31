/// @file
/// @brief Copy-pasted `CSteamNetworkingMessage` declaration from GNS.

#pragma once

#include <steam/steamnetworkingtypes.h>

#include <limits>

#ifdef STEAMNETWORKINGSOCKETS_ENABLE_MEM_OVERRIDE
#define STEAMNETWORKINGSOCKETS_DECLARE_CLASS_OPERATOR_NEW \
    static void* operator new(size_t s) noexcept \
    { \
        return malloc(s); \
    } \
    static void* operator new[](size_t) = delete; \
    static void operator delete(void* p) noexcept \
    { \
        free(p); \
    } \
    static void operator delete[](void*) = delete;
#else
#define STEAMNETWORKINGSOCKETS_DECLARE_CLASS_OPERATOR_NEW
#endif

typedef unsigned char byte;

namespace SteamNetworkingSocketsLib
{

struct SteamNetworkingMessageQueue;

/// We implement priority groups using Weighted Fair Queueing.
/// https://en.wikipedia.org/wiki/Weighted_fair_queueing
/// The idea is to assign a virtual "timestamp" when the message
/// would finish sending, and each time we have an opportunity to
/// send, we select the group with the earliest finish time.
/// Virtual time is essentially an arbitrary counter that increases
/// at a fixed rate per outbound byte sent.
typedef int64 VirtualSendTime;
static constexpr VirtualSendTime k_virtSendTime_Infinite = std::numeric_limits<VirtualSendTime>::max();

/// Actual implementation of SteamNetworkingMessage_t, which is the API
/// visible type.  Has extra fields needed to put the message into intrusive
/// linked lists.
class CSteamNetworkingMessage : public SteamNetworkingMessage_t
{
public:
    STEAMNETWORKINGSOCKETS_DECLARE_CLASS_OPERATOR_NEW
    static CSteamNetworkingMessage* New(uint32 cbSize);
    static void DefaultFreeData(SteamNetworkingMessage_t* pMsg);

    /// OK to delay sending this message until this time.  Set to zero to explicitly force
    /// Nagle timer to expire and send now (but this should behave the same as if the
    /// timer < usecNow).  If the timer is cleared, then all messages with lower message numbers
    /// are also cleared.
    // NOTE: Intentionally reusing the m_usecTimeReceived field, which is not used on outbound messages
    inline SteamNetworkingMicroseconds SNPSend_UsecNagle() const
    {
        return m_usecTimeReceived;
    }
    inline void SNPSend_SetUsecNagle(SteamNetworkingMicroseconds x)
    {
        m_usecTimeReceived = x;
    }

    /// "Virtual finish time".  This is the "virtual time" when we
    /// wound have finished sending the current message, if any,
    /// if all priority groups were busy and we got our proportionate
    /// share.
    inline VirtualSendTime SNPSend_VirtualFinishTime() const
    {
        return m_nConnUserData;
    }
    inline void SNPSend_SetVirtualFinishTime(VirtualSendTime x)
    {
        m_nConnUserData = x;
    }

    /// Offset in reliable stream of the header byte.  0 if we're not reliable.
    inline int SNPSend_ReliableStreamSize() const;

    inline bool SNPSend_IsReliable() const;

    inline int64 SNPSend_ReliableStreamPos() const;
    inline void SNPSend_SetReliableStreamPos(int64 x);

    // Working data for reliable messages.
    struct ReliableSendInfo_t
    {
        int64 m_nStreamPos;

        // Number of reliable segments that refer to this message.
        // Also while we are in the queue waiting to be sent the queue holds a reference
        int m_nSentReliableSegRefCount;
        int m_cbHdr;
        byte m_hdr[16];
    };
    const ReliableSendInfo_t& ReliableSendInfo() const;
    ReliableSendInfo_t& ReliableSendInfo();

    /// Remove it from queues
    void Unlink();

    struct Links
    {
        SteamNetworkingMessageQueue* m_pQueue;
        CSteamNetworkingMessage* m_pPrev;
        CSteamNetworkingMessage* m_pNext;

        inline void Clear()
        {
            m_pQueue = nullptr;
            m_pPrev = nullptr;
            m_pNext = nullptr;
        }
    };

    /// Intrusive links for the "primary" list we are in
    Links m_links;

    /// Intrusive links for any secondary list we may be in.  (Same listen socket or
    /// P2P channel, depending on message type)
    Links m_linksSecondaryQueue;

    void LinkBefore(CSteamNetworkingMessage* pSuccessor, Links CSteamNetworkingMessage::* pMbrLinks,
                    SteamNetworkingMessageQueue* pQueue);
    void LinkToQueueTail(Links CSteamNetworkingMessage::* pMbrLinks, SteamNetworkingMessageQueue* pQueue);
    void UnlinkFromQueue(Links CSteamNetworkingMessage::* pMbrLinks);

//private:
    // Use New and Release()!!
    inline CSteamNetworkingMessage()
    {
    }
    inline ~CSteamNetworkingMessage()
    {
    }
    static void ReleaseFunc(SteamNetworkingMessage_t* pIMsg);
};

} // namespace SteamNetworkingSocketsLib
