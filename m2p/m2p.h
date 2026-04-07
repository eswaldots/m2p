#ifndef M2P_H
#define M2P_H

typedef enum HEADING_FONT_SIZE {
        HEADING_H1_SIZE = 20,
        HEADING_H2_SIZE = 18,
        HEADING_H3_SIZE = 16,
        HEADING_H4_SIZE = 14,
        HEADING_H5_SIZE = 12,
        HEADING_H6_SIZE = 11,
        P_SIZE = 12,
} HEADING_FONT_SIZE;

typedef enum {
        M2P_LOG_WARNING = 1,
        M2P_LOG_INFO = 2,
        M2P_LOG_ERROR = 3,
        M2P_LOG_DEBUG = 4,
} m2p_log_level;
#endif
