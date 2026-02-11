/*
 * batman_pkt.h
 * B.A.T.M.A.N. (Better Approach To Mobile Ad-hoc Networking) Packet Definitions
 * Based on RFC draft-openmesh-b-a-t-m-a-n-00
 */

#ifndef __batman_pkt_h__
#define __batman_pkt_h__

#include <packet.h>

/* Protocol Constants */
#define BATMAN_VERSION 4
#define BATMAN_PORT 4305
#define TTL_MIN 2
#define TTL_MAX 255
#define SEQNO_MAX 65535

/* Timing Constants (in seconds) */
#define ORIGINATOR_INTERVAL 1.0
#define ORIGINATOR_INTERVAL_JITTER 0.2
#define WINDOW_SIZE 128
#define PURGE_TIMEOUT (10 * WINDOW_SIZE * ORIGINATOR_INTERVAL)
#define BROADCAST_DELAY_MAX 0.1
#define BI_LINK_TIMEOUT (3 * ORIGINATOR_INTERVAL)

/* Packet Types */
#define BATMANTYPE_OGM 0x01
#define BATMANTYPE_HNA 0x02

/* Flags */
#define BATMAN_FLAG_DIRECTLINK 0x40
#define BATMAN_FLAG_UNIDIRECTIONAL 0x20

/* OGM Header Structure - 12 bytes */
struct hdr_batman_ogm {
    u_int8_t  version_;
    u_int8_t  flags_;
    u_int8_t  ttl_;
    u_int8_t  gw_flags_;
    u_int16_t seqno_;
    u_int16_t gw_port_;
    nsaddr_t  orig_addr_;
    
    static int offset_;
    inline static int& offset() { return offset_; }
    inline static hdr_batman_ogm* access(const Packet* p) {
        return (hdr_batman_ogm*) p->access(offset_);
    }
    
    /* Header field access methods */
    u_int8_t& version() { return version_; }
    u_int8_t& flags() { return flags_; }
    u_int8_t& ttl() { return ttl_; }
    u_int8_t& gw_flags() { return gw_flags_; }
    u_int16_t& seqno() { return seqno_; }
    u_int16_t& gw_port() { return gw_port_; }
    nsaddr_t& orig_addr() { return orig_addr_; }
    
    /* Flag manipulation */
    inline bool is_directlink() { return (flags_ & BATMAN_FLAG_DIRECTLINK); }
    inline bool is_unidirectional() { return (flags_ & BATMAN_FLAG_UNIDIRECTIONAL); }
    inline void set_directlink() { flags_ |= BATMAN_FLAG_DIRECTLINK; }
    inline void set_unidirectional() { flags_ |= BATMAN_FLAG_UNIDIRECTIONAL; }
    inline void clear_directlink() { flags_ &= ~BATMAN_FLAG_DIRECTLINK; }
    inline void clear_unidirectional() { flags_ &= ~BATMAN_FLAG_UNIDIRECTIONAL; }
};

/* HNA Header Structure - 5 bytes */
struct hdr_batman_hna {
    nsaddr_t  network_addr_;
    u_int8_t  netmask_;
    
    static int offset_;
    inline static int& offset() { return offset_; }
    inline static hdr_batman_hna* access(const Packet* p) {
        return (hdr_batman_hna*) p->access(offset_);
    }
    
    nsaddr_t& network_addr() { return network_addr_; }
    u_int8_t& netmask() { return netmask_; }
};

/* Union for complete B.A.T.M.A.N. header */
union hdr_all_batman {
    hdr_batman_ogm ogm_;
    hdr_batman_hna hna_;
};

#endif /* __batman_pkt_h__ */
