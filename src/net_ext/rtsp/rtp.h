#ifndef NET_RTP_H
#define NET_RTP_H

#include <stdint.h>

namespace net
{

constexpr int kRtpMaxPacketSize = 1400;

enum class RtpPayloadType
{
    H264 = 96
};

/**
 *
 *    0                   1                   2                   3
 *    7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |V=2|P|X|  CC   |M|     PT      |       sequence number         |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                           timestamp                           |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |           synchronization source (SSRC) identifier            |
 *   +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *   |            contributing source (CSRC) identifiers             |
 *   :                             ....                              :
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */
struct RtpHeader
{
    /* byte 0 */
    uint8_t csrcLen; // 4 bit
    uint8_t extension; // 1 bit
    uint8_t padding; // 1 bit
    uint8_t version; // 2 bit

    /* byte 1 */
    RtpPayloadType payloadType; // 7 bit
    uint8_t marker; // 1 bit

    /* bytes 2,3 */
    uint16_t seq;

    /* bytes 4-7 */
    uint32_t timestamp;

    /* bytes 8-11 */
    uint32_t ssrc;
};


struct RtpInfo
{
    bool isOverUdp;

    union
    {
        struct
        {
            uint8_t rtpChannel;
            uint8_t rtcpChannel;
        };
        struct
        {
            uint16_t clientRtpPort;
            uint16_t clientRtcpPort;
        };
    };
};

} // namespace net

#endif // NET_RTP_H
