/* =======================================================
** Lumasys Optical Modem network/UDP Comm header file
**
** History:
**     Date      Who    Description
**    -------    ---    ----------------------------------
**   01/01/2010  SL     Create
**   03/06/2010  SL     Added debug_level to IdMappingList
** =======================================================
*/
#ifndef COMMS_INC
#define COMMS_INC
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "om4u_config.hpp"

//Define Process IDs
#define MAX_PIDS 19

#define SOA    0
#define MOA    1
#define UCMD   2
#define DPROC  3
#define MAA    4
#define SAA    5
#define MXA    6
#define SXA    7
#define RCMD   8
#define XTD    9
#define GUI    10
#define SOAA   11
#define SOAF   12
#define MOAA   13
#define MOAF   14
#define LIST1  15
#define LIST2  16
#define NAV    17
#define WEB    18
//#define VSURF  17

//#define MALL  22
//#define RALL  23
//#define BCMD  24


//#define JLOG  22
//#define MOA1  23
//#define MOA2  24
//#define MOA3  25


//Define Process ID Mappings - ID,Name,UDP_Port,IP_Addr,Block,Timeout,Debug_Level
// debug_level: ERR,WNG,MSG,DBG1,DBG2

/*#define IdMappingList \
              {"0  SOA  2200 127.0.0.1 1 0 MSG", \
               "1  MOA  3200 127.0.0.1 1 0 MSG", \
               "2  SOAA  2210 127.0.0.1 0 0 WNG", \
               "3  SOAF  2220 127.0.0.1 0 0 WNG", \
               "4  MOAA  3210 127.0.0.1 0 0 WNG", \
               "5  MOAF  3220 127.0.0.1 0 0 WNG", \
               "6  UCMD 2011 127.0.0.1 0 0 MSG", \
               "7  DPROC 2012 127.0.0.1 0 0 WNG", \
               "8  MAA 3400 127.0.0.1 0 0 MSG", \
               "9  RAA 2400 127.0.0.1 0 0 WNG", \
               "10 MXA 2500 127.0.0.1 0 0 WNG", \
               "11 SXA 3500 127.0.0.1 0 0 WNG", \
               "12 RCMD 3011 127.0.0.1 0 0 WNG", \
               "13 XTD  2700 127.0.0.1 1 0 WNG", \
               "14 GUI  2800 127.0.0.1 0 0 WNG", \
               "15 LIST1  3600 127.0.0.1 0 0 WNG", \
               "16 LIST2 3600 127.0.0.1 0 0 WNG"};
*/

#define IdMappingList \
              {"0  SOA   2200 127.0.0.1 1 0 MSG", \
               "1  MOA   3200 127.0.0.1 1 0 MSG", \
               "2  UCMD  2011 127.0.0.1 0 0 MSG", \
               "3  DPROC 2012 127.0.0.1 0 0 WNG", \
               "4  MAA   3400 127.0.0.1 0 0 MSG", \
               "5  RAA   2400 127.0.0.1 0 0 WNG", \
               "6  MXA   2500 127.0.0.1 0 0 WNG", \
               "7  SXA   3500 127.0.0.1 0 0 WNG", \
               "8  RCMD  3011 127.0.0.1 0 0 WNG", \
               "9  XTD   2700 127.0.0.1 1 0 WNG", \
               "10 GUI   2800 127.0.0.1 0 0 WNG", \
               "11 SOAA  2210 127.0.0.1 0 0 WNG", \
               "12 SOAF  2220 127.0.0.1 0 0 WNG", \
               "13 MOAA  3210 127.0.0.1 0 0 WNG", \
               "14 MOAF  3220 127.0.0.1 0 0 WNG", \
               "15 LIST1 2600 127.0.0.1 0 0 WNG", \
               "16 LIST2 3600 127.0.0.1 0 0 WNG", \
               "17 NAV   3700 127.0.0.1 0 0 WNG", \
               "18 WEB   3800 127.0.0.1 0 0 WNG"};


//#define MAX_BUF_SIZE 1024
#define MAX_BUF_SIZE            1400
#define MAX_RAWBUF_SIZE         2048
#define MAX_MESSAGE_SIZE        1024
#define MSG_OK                  0
#define MSG_ERR                -1
#define MSG_WOULD_BLOCK        -2
#define MSG_LL                  1
#define MSG_TIMEOUT            -3
#define MSG_RESET              -4

//Define message IDs here
#define NUM_MESSAGE_TYPES       170
#define ACK                     0
#define NACK                    1
#define PING                    2
#define PING_RET                3
#define QUIT                    4
#define XFR_STOP                5
#define XFR_RATE                6  //rate (bps)
#define XFR_PSIZE               7  //packet size (bytes)
#define XFR_LIMIT               8  //number of bytes
#define MXA_XFR_STATUS          9

#define STATUS_RATE             10  //update rate every nsec
#define OM_TXRTREQ              11
#define OM_TXRTCHNG             12
#define OM_RXRTREQ              13
#define OM_RXRTCHNG             14
#define OM_RDPWR                15
#define UMOD_RANGEREQ           16
#define UMOD_PING               17
#define UMOD_SETSRC             18
#define UMOD_OMRESET            19

#define OM_AGCSET               20
#define OM_EMITLEV              21
#define OM_SOASTAT              22
#define OM_MOASTAT              23
#define OM_SOADAT               24
#define OM_MOADAT               25
#define OM_RXKICK               26
#define UMOD_MAADAT             27
#define UMOD_RAADAT             28
#define SYSLOG                  29

#define DPROC_DAT               30
#define OM_XFRTXFLAG            31
#define BAM_GETRNG              32
#define BAM_FWD                 33
#define BAM_SETTOUT             34
#define AC_RSCSTAT              35
#define BAM_RETRNG              36
#define AC_RSCDAT               37
#define BAM_WAKE                38  // maybe delete - replaced by gosbc?
#define BAM_RETWAKE             39  // maybe delete - replaced by gosbc?

#define FTP_LDIR                40  // deprecated
#define FTP_RDIR                41  // deprecated
#define FTP_RDIR_ACK            42  // deprecated
#define FTP_PWD                 43  // deprecated
#define FTP_CD                  44  // deprecated
#define FTP_GET_DIR             45  // deprecated
#define FTP_GET_FILE            46  // deprecated
#define FTP_GET_NEXT_FILE       47  // deprecated
#define FTP_GET_FILE_ACK        48  // deprecated
#define FTP_PUT_FILE            49

#define FTP_PUT_FILE_ACK        50
#define FTP_RESEND_REQ          51  // deprecated
#define FTP_RESEND_PKT          52  // deprecated
#define SXA_XFR_STATUS          53
#define XTD_DAT                 54
#define OM_EMITSET              55
#define OM_EMITSTAT             56
#define OM_EMITTMP              57
#define BAM_CMD                 58
#define BAM_ACK                 59

#define OM_HALT                 60
#define BAM_ADDR                61
#define OM_SHUTDOWN             62
#define PIC_XFER_STOP           63
#define PIC_XFER_ACK            64
#define PIC_XFER_STATUS         65
#define UMOD_FWD                66
#define UMOD_WAKE               67
#define UMOD_GETRNG             68
#define UMOD_RETRNG             69

#define RSC_FILECPY             70
#define OM_COMBLOCK_RST         71
#define OM_NETPKT_RD            72
#define OM_VBATREAD             73
#define OM_SAMPLE               74
#define OM_TEST_SAMPLE          75
#define OM_TEST_MODE            76  // can we remove ??
#define OM_TXERCREQ             77
#define OM_TXERCCHNG            78
#define OM_RXERCREQ             79

#define OM_RXERCCHNG            80
#define OM_TESTDAT              81
#define OM_TXCARREQ             82
#define OM_RXCARREQ             83
#define BAM_DIRCMD              84
#define BAM_DIRRESP             85
#define STATE                   86
#define STATE_RET               87
#define XFR_COMPLETE            88
#define BAM_DAT                 89

#define OM_STATE                90
#define OM_RCVENA               91
#define OM_CAPTURE              92
#define OM_CONFIG               93
#define FTP_SET_ARG             94  // deprecated
#define ERROR_MSG               95
#define FTP_SEND_REQ            96  // deprecated
#define FTP_SEND_ACK            97  // deprecated
#define BAM_SIM                 98
#define BAM_PKT_ACK             99

#define OM_CLEAR                100
#define OM_REMMSG               101
#define BAM_REMMSG              102
#define BAM_CONFIG              103
#define BAM_CONFIRM             104
#define BAM_COMP                105
#define OM_SOABIDIR             106
#define OM_MOABIDIR             107
#define OM_SOAEMIT              108
#define OM_MOAEMIT              109

#define OM_MODEREQ              110
#define OM_MODECHNG             111
#define OM_MODECONFIRM          112
#define OM_SETREQ               113
#define OM_SETPARAMS            114
#define OM_SETTINGS             115
#define OM_MODELIST             116
#define OM_MODES                117
#define OM_RESET                118
#define OM_TEST                 119

#define GUI_ADDRESS             120
#define OM_POWER                121
#define CMDEXEC                 122
#define OXFRSTART               123
#define OXFRPROG                124
#define SENTSTAT                125
#define OM_SOALRDAT             126
#define OM_MOALRDAT             127
#define SYNCTX_RATIO            128
#define VERSION                 129

#define FIRMWARE_UPDATE         130
#define SELFTEST                131
#define OM_SOAPTLG              132
#define OM_MOAPTLG              133
#define OM_STATUSREQ            134
#define OM_STATUS               135
#define OM_ENABLE               136
#define VERSIONREQ              137
#define APP_ADDRESS             138
#define OM_UPDATE               139

#define OM_FACT_RESET           140
#define REMUS_DATA              141
#define CMDFWD                  142
#define OM_BANDWIDTH_REQ        143
#define OM_BANDWIDTH            144
#define OM_WAKE_REQ             145
#define OM_WAKE_RESP            146
#define RESERVED147             147
#define RESERVED148             148
#define OM_NAVDATA              149

#define OM_NAVSTATUS            150
#define OM_NAVID_REQ            151
#define OM_NAVID_RESP           152
#define OM_NAVID_SAVE_REQ       153
#define OM_NAVID_SAVE_RESP      154
#define OM_NAVID_MSG            155
#define RESERVED156             156
#define WEB_UPDATE_REQ          157
#define WEB_UPDATE_RSP          158
#define UPDATE_STATUS           159

#define OM_SETTINGS_SAVE_REQ    160
#define OM_SETTINGS_SAVE_RESP   161
#define OM_PRNG_PAIRED          162
#define OM_PRNG_UNPAIRED        163
#define OM_PRNG_ERROR           164
#define RESERVED165             165
#define RESERVED166             166
#define RESERVED167             167
#define RESERVED168             168
#define RESERVED169             169

//#define OM_TEST_RUN   89
//#define OM_TEST_ERROR   73
//#define OM_TEST_CONFIG    85
//#define OM_TXRTSTAT      31
//#define OM_RXRTSTAT      32
//#define OM_TXREVERT      33
//#define OM_RXREVERT      34
//#define OM_STATREQ       26  
//#define OM_STATUS        27
//#define OM_LOG           28
//#define OM_UPSOADRT  20
//#define OM_DWNSOADRT     21
//#define XFR_DAT          10
//#define XFR_TX_DAT       11
//#define XFR_COMPLETE     13
//#define XFR_START         5


/* depreciated commands 
//                "XFR_START",     \
//                "XFR_DAT",       \
//                "XFR_TX_DAT",    \
//         "OM_UPSOADRT",   \
//         "OM_DWNSOADRT",  \
//         "OM_STATREQ",    \
//         "OM_STATUS",     \
//         "OM_LOG",        \
//         "OM_TXRTSTAT",   \
//         "OM_RXRTSTAT",   \
//         "OM_TXREVERT",   \
//         "OM_RXREVERT",   \
//    "OM_TEST_CONFIG", \
//    "OM_TEST_ERROR", \
//    "OM_TEST_RUN", \
//  "OM_ARPCLEAR", \ */

// Define message Names here
#define MsgNameList           \
    {"ACK",                   \
     "NACK",                  \
     "PING",                  \
     "PING_RET",              \
     "QUIT",                  \
     "XFR_STOP",              \
     "XFR_RATE",              \
     "XFR_PSIZE",             \
     "XFR_LIMIT",             \
     "MXA_XFR_STATUS",        \
     "STATUS_RATE",           \
     "OM_TXRTREQ",            \
     "OM_TXRTCHNG",           \
     "OM_RXRTREQ",            \
     "OM_RXRTCHNG",           \
     "OM_RDPWR",              \
     "UMOD_RANGEREQ",         \
     "UMOD_PING",             \
     "UMOD_SETSRC",           \
     "UMOD_OMRESET",          \
     "OM_AGCSET",             \
     "OM_EMITLEV",            \
     "OM_SOASTAT",            \
     "OM_MOASTAT",            \
     "OM_SOADAT",             \
     "OM_MOADAT",             \
     "OM_RXKICK",             \
     "UMOD_MAADAT",           \
     "UMOD_RAADAT",           \
     "SYSLOG",                \
     "DPROC_DAT",             \
     "OM_XFRTXFLAG",          \
     "BAM_GETRNG",            \
     "BAM_FWD",               \
     "BAM_SETTOUT",           \
     "AC_RSCSTAT",            \
     "BAM_RETRNG",            \
     "AC_RSCDAT",             \
     "BAM_WAKE",              \
     "BAM_RETWAKE",           \
     "FTP_LDIR",              \
     "FTP_RDIR",              \
     "FTP_RDIR_ACK",          \
     "FTP_PWD",               \
     "FTP_CD",                \
     "FTP_GET_DIR",           \
     "FTP_GET_FILE",          \
     "FTP_GET_NEXT_FILE",     \
     "FTP_GET_FILE_ACK",      \
     "FTP_PUT_FILE",          \
     "FTP_PUT_FILE_ACK",      \
     "FTP_RESEND_REQ",        \
     "FTP_RESEND_PKT",        \
     "SXA_XFR_STATUS",        \
     "XTD_DAT",               \
     "OM_EMITSET",            \
     "OM_EMITSTAT",           \
     "OM_EMITTMP",            \
     "BAM_CMD",               \
     "BAM_ACK",               \
     "OM_HALT",               \
     "BAM_ADDR",              \
     "OM_SHUTDOWN",           \
     "PIC_XFER_STOP",         \
     "PIC_XFER_ACK",          \
     "PIC_XFER_STATUS",       \
     "UMOD_FWD",              \
     "UMOD_WAKE",             \
     "UMOD_GETRNG",           \
     "UMOD_RETRNG",           \
     "RSC_FILECPY",           \
     "OM_COMBLOCK_RST",       \
     "OM_NETPKT_RD",          \
     "OM_VBATREAD",           \
     "OM_SAMPLE",             \
     "OM_TEST_SAMPLE",        \
     "OM_TEST_MODE",          \
     "OM_TXERCREQ",           \
     "OM_TXERCCHNG",          \
     "OM_RXERCREQ",           \
     "OM_RXERCCHNG",          \
     "OM_TESTDAT",            \
     "OM_TXCARREQ",           \
     "OM_RXCARREQ",           \
     "BAM_DIRCMD",            \
     "BAM_DIRRESP",           \
     "STATE",                 \
     "STATE_RET",             \
     "XFR_COMPLETE",          \
     "BAM_DAT",               \
     "OM_STATE",              \
     "OM_RCVENA",             \
     "OM_CAPTURE",            \
     "OM_CONFIG",             \
     "FTP_SET_ARG",           \
     "ERROR_MSG",             \
     "FTP_SEND_REQ",          \
     "FTP_SEND_ACK",          \
     "BAM_SIM",               \
     "BAM_PKT_ACK",           \
     "OM_CLEAR",              \
     "OM_REMMSG",             \
     "BAM_REMMSG",            \
     "BAM_CONFIG",            \
     "BAM_CONFIRM",           \
     "BAM_COMP",              \
     "OM_SOABIDIR",           \
     "OM_MOABIDIR",           \
     "OM_SOAEMIT",            \
     "OM_MOAEMIT",            \
     "OM_MODEREQ",            \
     "OM_MODECHNG",           \
     "OM_MODECONFIRM",        \
     "OM_SETREQ",             \
     "OM_SETPARAMS",          \
     "OM_SETTINGS",           \
     "OM_MODELIST",           \
     "OM_MODES",              \
     "OM_RESET",              \
     "OM_TEST",               \
     "GUI_ADDRESS",           \
     "OM_POWER",              \
     "CMDEXEC",               \
     "OXFRSTART",             \
     "OXFRPROG",              \
     "SENTSTAT",              \
     "OM_SOALRDAT",           \
     "OM_MOALRDAT",           \
     "SYNCTX_RATIO",          \
     "VERSION",               \
     "FIRMWARE_UPDATE",       \
     "SELFTEST",              \
     "OM_SOAPTLG",            \
     "OM_MOAPTLG",            \
     "OM_STATUSREQ",          \
     "OM_STATUS",             \
     "OM_ENABLE",             \
     "VERSIONREQ",            \
     "APP_ADDRESS",           \
     "OM_UPDATE",             \
     "OM_FACT_RESET",         \
     "REMUS_DATA",            \
     "CMDFWD",                \
     "OM_BANDWIDTH_REQ",      \
     "OM_BANDWIDTH",          \
     "OM_WAKE_REQ",           \
     "OM_WAKE_RESP",          \
     "RESERVED147",           \
     "RESERVED148",           \
     "OM_NAVDATA",            \
     "OM_NAVSTATUS",          \
     "OM_NAVID_REQ",          \
     "OM_NAVID_RESP",         \
     "OM_NAVID_SAVE_REQ",     \
     "OM_NAVID_SAVE_RESP",    \
     "OM_NAVID_MSG",          \
     "RESERVED156",           \
     "WEB_UPDATE_REQ",        \
     "WEB_UPDATE_RSP",        \
     "UPDATE_STATUS",         \
     "OM_SETTINGS_SAVE_REQ",  \
     "OM_SETTINGS_SAVE_RESP", \
     "OM_PRNG_PAIRED",        \
     "OM_PRNG_UNPAIRED",      \
     "OM_PRNG_ERROR",         \
     "__End_MSG_NameList__"};

// Define message help here - order does not matter
#define MsgHelpList         \
    {"PING",                \
     "QUIT",                \
     "XFR_START",           \
     "XFR_STOP",            \
     "XFR_RATE <bps>",      \
     "XFR_PSIZE <bytes>",   \
     "XFR_LIMIT <bytes>",   \
     "STATUS_RATE <nsec>",  \
     "OM_EXPRUN",           \
     "FTP_GET_FILE <file>", \
     "FTP_PUT_FILE <file>", \
     "__End_MSG_Help__"};

typedef struct
{
    int dest;
    int src;
    int msg_type;
    int msg_len;
} message_hdr_t;

typedef struct
{
    int pid;
    char pid_name[25];
    int udp_port;
    char ip_addr[25];
    int block;
    int timeout;
    int sock_in;
    int sock_out;
    struct sockaddr_in src_addr;
    unsigned int src_addr_len;
    struct sockaddr_in dest_addr;
    unsigned int dest_addr_len;
    char debug_level_str[10];
} proc_comm_t;

#endif //COMMS_INC


int initialize_network_comms(om4u::om4uConfigStruct_t *conf_ptr);
extern int   gui_address_set(char *gui_address);
int             app_address_set(const char *id_str, const char *app_address);
unsigned int    pair_address_set(int my_id, uint16_t pair_id);
void            my_ip_address_set(const char* iface_name, const char* ip_address, const char* netmask);
extern void  close_comms();
extern int   get_msg(int my_id, message_hdr_t *msg_hdr, char *msg_data, int max_msg_size);
extern int   get_msg_raw(int my_id, message_hdr_t *msg_hdr, char *msg_data, int max_msg_size, char *raw_msg);
extern int   get_msg_repeat(int my_id, message_hdr_t *msg_hdr, char *msg_data, int max_msg_size);
extern int   get_msg_from_sentry(int my_id, message_hdr_t *msg_hdr, char *msg_data, int max_msg_size);
extern int   get_msg2(int my_id2, char *msg_data, int max_msg_size);
extern int   get_msg3(int my_id2, char *msg, int max_msg_size, int *byte_rcv);
extern int   get_msg3_select(int my_id2, char *msg, int max_msg_size, int *byte_rcv);
extern int   get_msg_select(int my_id, int my_id2, message_hdr_t *msg_hdr, char *msg_data, int max_msg_size);
extern int   send_msg(int dest, int src, int msg_type, int msg_len, char msg[]);
extern int   send_raw_msg(int dest, int src, int msg_type, int msg_len, char msg[]);
extern int   send_msg_lowlat(int dest, int src, int msg_len, char msg[]);
extern int   send_jason_msg(int dest, int msg_len, char msg[]);
extern int send_msg3(int dest, int src, int msg_len, char msg[]);
extern int send_msg_repeat(int dest, int src, int msg_type, int msg_len, char msg[], int rep_dest);
extern int send_msg_prefix(char *prefix, int dest, int src, int msg_type, int msg_len, char msg[]);
void         send_msg_to_listeners(int status1hz_enable, int src, int msg_type, size_t msg_len, char msg[]);
extern void *ip_detect_thread (void *arg);
extern int get_raw_msg(int my_id );
extern int   PIDstr2ID(const char *str);
extern char *ID2PIDstr(int id);
extern char *MsgID2Str(int id);
extern int   MsgName2ID(char *str);
extern void  str2Upper(char *str);
extern void  show_msg_help();
extern void  show_net_cfg();



