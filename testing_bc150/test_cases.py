# test_cases.py

# Define the chronological test sequence with response validation
# Based on net_comms.h from Lumasys Optical Modem

# Process ID constants
PROCESS_IDS = {
    'SOA': 0,
    'MOA': 1,
    'UCMD': 2,
    'DPROC': 3,
    'MAA': 4,
    'SAA': 5,
    'MXA': 6,
    'SXA': 7,
    'RCMD': 8,
    'XTD': 9,
    'GUI': 10,
    'SOAA': 11,
    'SOAF': 12,
    'MOAA': 13,
    'MOAF': 14,
    'LIST1': 15,
    'LIST2': 16,
    'NAV': 17,
    'WEB': 18
}

# Complete message type constants from net_comms.h
MESSAGE_TYPES = {
    # Basic communication
    'ACK': 0,
    'NACK': 1,
    'PING': 2,
    'PING_RET': 3,
    'QUIT': 4,

    # Transfer control
    'XFR_STOP': 5,
    'XFR_RATE': 6,
    'XFR_PSIZE': 7,
    'XFR_LIMIT': 8,
    'MXA_XFR_STATUS': 9,

    # Status and control (10-19)
    'STATUS_RATE': 10,
    'OM_TXRTREQ': 11,
    'OM_TXRTCHNG': 12,
    'OM_RXRTREQ': 13,
    'OM_RXRTCHNG': 14,
    'OM_RDPWR': 15,
    'UMOD_RANGEREQ': 16,
    'UMOD_PING': 17,
    'UMOD_SETSRC': 18,
    'UMOD_OMRESET': 19,

    # OM control (20-29)
    'OM_AGCSET': 20,
    'OM_EMITLEV': 21,
    'OM_SOASTAT': 22,
    'OM_MOASTAT': 23,
    'OM_SOADAT': 24,
    'OM_MOADAT': 25,
    'OM_RXKICK': 26,
    'UMOD_MAADAT': 27,
    'UMOD_RAADAT': 28,
    'SYSLOG': 29,

    # Data processing and BAM (30-39)
    'DPROC_DAT': 30,
    'OM_XFRTXFLAG': 31,
    'BAM_GETRNG': 32,
    'BAM_FWD': 33,
    'BAM_SETTOUT': 34,
    'AC_RSCSTAT': 35,
    'BAM_RETRNG': 36,
    'AC_RSCDAT': 37,
    'BAM_WAKE': 38,  # maybe deprecated - replaced by gosbc?
    'BAM_RETWAKE': 39,  # maybe deprecated - replaced by gosbc?

    # FTP commands (40-52) - many deprecated
    'FTP_LDIR': 40,  # deprecated
    'FTP_RDIR': 41,  # deprecated
    'FTP_RDIR_ACK': 42,  # deprecated
    'FTP_PWD': 43,  # deprecated
    'FTP_CD': 44,  # deprecated
    'FTP_GET_DIR': 45,  # deprecated
    'FTP_GET_FILE': 46,  # deprecated
    'FTP_GET_NEXT_FILE': 47,  # deprecated
    'FTP_GET_FILE_ACK': 48,  # deprecated
    'FTP_PUT_FILE': 49,
    'FTP_PUT_FILE_ACK': 50,
    'FTP_RESEND_REQ': 51,  # deprecated
    'FTP_RESEND_PKT': 52,  # deprecated

    # Transfer and system (53-69)
    'SXA_XFR_STATUS': 53,
    'XTD_DAT': 54,
    'OM_EMITSET': 55,
    'OM_EMITSTAT': 56,
    'OM_EMITTMP': 57,
    'BAM_CMD': 58,
    'BAM_ACK': 59,
    'OM_HALT': 60,
    'BAM_ADDR': 61,
    'OM_SHUTDOWN': 62,
    'PIC_XFER_STOP': 63,
    'PIC_XFER_ACK': 64,
    'PIC_XFER_STATUS': 65,
    'UMOD_FWD': 66,
    'UMOD_WAKE': 67,
    'UMOD_GETRNG': 68,
    'UMOD_RETRNG': 69,

    # System operations (70-89)
    'RSC_FILECPY': 70,
    'OM_COMBLOCK_RST': 71,
    'OM_NETPKT_RD': 72,
    'OM_VBATREAD': 73,
    'OM_SAMPLE': 74,
    'OM_TEST_SAMPLE': 75,
    'OM_TEST_MODE': 76,  # can we remove ??
    'OM_TXERCREQ': 77,
    'OM_TXERCCHNG': 78,
    'OM_RXERCREQ': 79,
    'OM_RXERCCHNG': 80,
    'OM_TESTDAT': 81,
    'OM_TXCARREQ': 82,
    'OM_RXCARREQ': 83,
    'BAM_DIRCMD': 84,
    'BAM_DIRRESP': 85,
    'STATE': 86,
    'STATE_RET': 87,
    'XFR_COMPLETE': 88,
    'BAM_DAT': 89,

    # OM operations (90-109)
    'OM_STATE': 90,
    'OM_RCVENA': 91,
    'OM_CAPTURE': 92,
    'OM_CONFIG': 93,
    'FTP_SET_ARG': 94,  # deprecated
    'ERROR_MSG': 95,
    'FTP_SEND_REQ': 96,  # deprecated
    'FTP_SEND_ACK': 97,  # deprecated
    'BAM_SIM': 98,
    'BAM_PKT_ACK': 99,
    'OM_CLEAR': 100,
    'OM_REMMSG': 101,
    'BAM_REMMSG': 102,
    'BAM_CONFIG': 103,
    'BAM_CONFIRM': 104,
    'BAM_COMP': 105,
    'OM_SOABIDIR': 106,
    'OM_MOABIDIR': 107,
    'OM_SOAEMIT': 108,
    'OM_MOAEMIT': 109,

    # Mode and settings (110-119)
    'OM_MODEREQ': 110,
    'OM_MODECHNG': 111,
    'OM_MODECONFIRM': 112,
    'OM_SETREQ': 113,
    'OM_SETPARAMS': 114,
    'OM_SETTINGS': 115,
    'OM_MODELIST': 116,
    'OM_MODES': 117,
    'OM_RESET': 118,
    'OM_TEST': 119,

    # GUI and system (120-129)
    'GUI_ADDRESS': 120,
    'OM_POWER': 121,
    'CMDEXEC': 122,
    'OXFRSTART': 123,
    'OXFRPROG': 124,
    'SENTSTAT': 125,
    'OM_SOALRDAT': 126,  # Status message - filtered out
    'OM_MOALRDAT': 127,  # Status message - filtered out
    'SYNCTX_RATIO': 128,
    'VERSION': 129,

    # Firmware and status (130-139)
    'FIRMWARE_UPDATE': 130,
    'SELFTEST': 131,
    'OM_SOAPTLG': 132,
    'OM_MOAPTLG': 133,
    'OM_STATUSREQ': 134,
    'OM_STATUS': 135,
    'OM_ENABLE': 136,
    'VERSIONREQ': 137,
    'APP_ADDRESS': 138,
    'OM_UPDATE': 139,

    # System control (140-149)
    'OM_FACT_RESET': 140,
    'REMUS_DATA': 141,
    'CMDFWD': 142,
    'OM_BANDWIDTH_REQ': 143,
    'OM_BANDWIDTH': 144,
    'OM_WAKE_REQ': 145,
    'OM_WAKE_RESP': 146,
    'RESERVED147': 147,
    'RESERVED148': 148,
    'OM_NAVDATA': 149,

    # Navigation (150-159)
    'OM_NAVSTATUS': 150,
    'OM_NAVID_REQ': 151,
    'OM_NAVID_RESP': 152,
    'OM_NAVID_SAVE_REQ': 153,
    'OM_NAVID_SAVE_RESP': 154,
    'OM_NAVID_MSG': 155,
    'RESERVED156': 156,
    'WEB_UPDATE_REQ': 157,
    'WEB_UPDATE_RSP': 158,
    'UPDATE_STATUS': 159,

    # Settings and PRNG (160-169)
    'OM_SETTINGS_SAVE_REQ': 160,
    'OM_SETTINGS_SAVE_RESP': 161,
    'OM_PRNG_PAIRED': 162,
    'OM_PRNG_UNPAIRED': 163,
    'OM_PRNG_ERROR': 164,
    'RESERVED165': 165,
    'RESERVED166': 166,
    'RESERVED167': 167,
    'RESERVED168': 168,
    'RESERVED169': 169
}

# Status message types that are typically filtered out
STATUS_MESSAGE_TYPES = {
    MESSAGE_TYPES['OM_SOALRDAT'],  # 126
    MESSAGE_TYPES['OM_MOALRDAT']  # 127
}

# Test sequence definition
test_sequence = [
    {
        'desc': "Change GUI address",
        'dest': 1,
        'msg_type': MESSAGE_TYPES['GUI_ADDRESS'],
        'body': '192.168.2.10',
        'expect_response': False
    },
    {
        'desc': "Save settings",
        'dest': 1,
        'msg_type': MESSAGE_TYPES['OM_SETTINGS_SAVE_REQ'],
        'body': '',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['OM_SETTINGS_SAVE_RESP']],
        'expected_body_content': None,  # Any response body is acceptable
        'timeout': 2.0  # Allow more time for save operation
    },
    {
        'desc': "Basic ping test",
        'dest': 1,
        'msg_type': MESSAGE_TYPES['PING'],
        'body': '',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['PING_RET']],
        'expected_body_content': None
    },
    {
        'desc': "Request version info",
        'dest': 1,  # SOA
        'msg_type': MESSAGE_TYPES['VERSIONREQ'],
        'body': '',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['OM_UPDATE']],  # Based on header pattern
        'expected_body_content': None  # Version string can vary
    },
    {
        'desc': "Request settings",
        'dest': 1,
        'msg_type': MESSAGE_TYPES['OM_SETREQ'],
        'body': '',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['OM_SETPARAMS']],
        'expected_body_content': None  # Settings content can vary
    },
    {
        'desc': "Send invalid mode request",
        'dest': 1,
        'msg_type': MESSAGE_TYPES['OM_MODEREQ'],
        'body': '999',  # assuming mode 999 is invalid
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['OM_MODECHNG']],
        'expected_body_content': None,  # Could be error response or rejection
        'timeout': 1.5
    },
    {
        'desc': "Trigger wake request",
        'dest': 1,
        'msg_type': MESSAGE_TYPES['OM_WAKE_REQ'],
        'body': '1',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['OM_WAKE_RESP']],
        'expected_body_content': None
    },
    # Additional comprehensive test cases
    {
        'desc': "Ping with echo validation",
        'dest': 1,
        'msg_type': MESSAGE_TYPES['PING'],
        'body': 'test_ping_echo',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['PING_RET']],
        'expected_body_content': 'test_ping_echo'  # Expect echo of sent body
    },
    {
        'desc': "Request status",
        'dest': 1,
        'msg_type': MESSAGE_TYPES['OM_STATUSREQ'],
        'body': '',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['OM_STATUS']],
        'expected_body_content': None,
        'timeout': 2.0
    },
    {
        'desc': "Request bandwidth info",
        'dest': 1,
        'msg_type': MESSAGE_TYPES['OM_BANDWIDTH_REQ'],
        'body': '',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['OM_BANDWIDTH']],
        'expected_body_content': None
    },
    {
        'desc': "Request mode list",
        'dest': 1,
        'msg_type': MESSAGE_TYPES['OM_MODELIST'],
        'body': '',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['OM_MODES']],
        'expected_body_content': None,
        'timeout': 2.0
    },
    {
        'desc': "Test ACK/NACK response",
        'dest': 1,
        'msg_type': MESSAGE_TYPES['OM_TEST'],
        'body': 'test_command',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['ACK'], MESSAGE_TYPES['NACK']],
        'expected_body_content': None
    },
    {
        'desc': "Settings request with possible indication",
        'dest': 1,
        'msg_type': MESSAGE_TYPES['OM_SETREQ'],
        'body': '',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['OM_SETPARAMS'], MESSAGE_TYPES['OM_SETTINGS']],
        'expected_body_content': None,
        'timeout': 2.0
    },
    {
        'desc': "App address update (no response expected)",
        'dest': 0,
        'msg_type': MESSAGE_TYPES['APP_ADDRESS'],
        'body': 'GUI 192.168.2.10 1',
        'expect_response': False
    },
    {
        'desc': "System log message (no response expected)",
        'dest': 1,
        'msg_type': MESSAGE_TYPES['SYSLOG'],
        'body': 'Test log entry',
        'expect_response': False
    },
    {
        'desc': "Invalid message type test",
        'dest': 1,
        'msg_type': 999,  # Invalid message type
        'body': '',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['NACK'], MESSAGE_TYPES['ERROR_MSG']],
        'expected_body_content': None,
        'timeout': 1.5
    },
    # Gemini produced the following test cases
    # =================================================================
    # I. Basic Connectivity and System Control
    # =================================================================
    {
        'desc': "Basic Ping",
        'dest': 1, 'msg_type': MESSAGE_TYPES['PING'], 'body': '',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['PING_RET']],
        'expected_body_content': "ping received"
    }, # Rationale: Verifies the simplest request-response loop (case PING).
    {
        'desc': "Request Version",
        'dest': 1, 'msg_type': MESSAGE_TYPES['VERSIONREQ'], 'body': '',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['VERSION']],
        'expected_body_content': "Arm software version"
    }, # Rationale: Tests case VERSIONREQ, which returns the compiled software version string.
    {
        'desc': "Save Settings (Success)",
        'dest': 1, 'msg_type': MESSAGE_TYPES['OM_SETTINGS_SAVE_REQ'], 'body': '',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['OM_SETTINGS_SAVE_RESP'], MESSAGE_TYPES['OM_SETTINGS']],
        'expected_body_content': "1 Settings successfully saved"
    }, # Rationale: Assumes writable config, causing updateUserConfigFile() to succeed and return a success message. Also expects a follow-up OM_SETTINGS message.
    {
        'desc': "System Reboot Command",
        'dest': 1, 'msg_type': MESSAGE_TYPES['OM_RESET'], 'body': '',
        'expect_response': False
    }, # Rationale: Tests case OM_RESET. The code calls system reboot and sends no response.
    {
        'desc': "Invalid Message Type",
        'dest': 1, 'msg_type': 999, 'body': '',
        'expect_response': False
    }, # Rationale: Tests the default case of the main switch statement, which logs an error but sends no reply.

    # =================================================================
    # II. Configuration and Settings Management
    # =================================================================
    {
        'desc': "Request Full Settings",
        'dest': 1, 'msg_type': MESSAGE_TYPES['OM_SETREQ'], 'body': '',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['OM_SETTINGS']]
    }, # Rationale: Tests case OM_SETREQ, which sends back the full configuration string.
    {
        'desc': "Request Mode List",
        'dest': 1, 'msg_type': MESSAGE_TYPES['OM_MODELIST'], 'body': '',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['OM_MODES']]
    }, # Rationale: Tests case OM_MODELIST, which sends back a list of available modes.
    {
        'desc': "Valid Mode Change",
        'dest': 1, 'msg_type': MESSAGE_TYPES['OM_MODEREQ'], 'body': '2',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['OM_MODECHNG'], MESSAGE_TYPES['OM_SETTINGS']],
        'expected_body_content': '02'
    }, # Rationale: Assuming mode '2' is valid, the code confirms with OM_MODECHNG and then sends the new settings to the WEB interface.
    {
        'desc': "Invalid Mode Change (Mode does not exist)",
        'dest': 1, 'msg_type': MESSAGE_TYPES['OM_MODEREQ'], 'body': '99',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['ERROR_MSG']],
        'expected_body_content': 'Invalid mode 99'
    }, # Rationale: findModeById() returns NULL for an invalid mode, triggering the else block that sends an ERROR_MSG.
    {
        'desc': "Malformed Mode Change (Non-numeric)",
        'dest': 1, 'msg_type': MESSAGE_TYPES['OM_MODEREQ'], 'body': 'abc',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['ERROR_MSG']],
        'expected_body_content': 'Invalid mode 00'
    }, # Rationale: `atoi("abc")` returns 0. Assuming mode 0 is also invalid, tests how non-numeric input is handled.
    {
        'desc': "Set Parameters (Valid)",
        'dest': 1, 'msg_type': MESSAGE_TYPES['OM_SETPARAMS'], 'body': 'fpga.tdma.default_mode=2\n',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['OM_SETTINGS']]
    }, # Rationale: `parse_om_settings_mesg()` modifies the config, and a full OM_SETTINGS response is sent to confirm.
    {
        'desc': "Set Parameters (Malformed Key)",
        'dest': 1, 'msg_type': MESSAGE_TYPES['OM_SETPARAMS'], 'body': 'invalid.parameter=true',
        'expect_response': False
    }, # Rationale: `parse_om_settings_mesg()` will fail. The `if` condition in the C++ is false, and no response is sent.
    {
        'desc': "Update App Address (3 params)",
        'dest': 1, 'msg_type': MESSAGE_TYPES['APP_ADDRESS'], 'body': 'GUI 192.168.2.100 1',
        'expect_response': False
    }, # Rationale: Tests case APP_ADDRESS. `sscanf` succeeds, config is updated, no direct response is sent.

    # =================================================================
    # III. "Not Implemented" Functionality
    # =================================================================
    {
        'desc': "Test Not Implemented: OM_TXRTREQ",
        'dest': 1, 'msg_type': MESSAGE_TYPES['OM_TXRTREQ'], 'body': '100',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['ERROR_MSG']],
        'expected_body_content': "OM_TXRTREQ not implemented"
    }, # Rationale: Confirms that this case falls through to the generic "not implemented" error response.
    {
        'desc': "Test Not Implemented: OM_RXKICK",
        'dest': 1, 'msg_type': MESSAGE_TYPES['OM_RXKICK'], 'body': '',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['ERROR_MSG']],
        'expected_body_content': "OM_RXKICK not implemented"
    }, # Rationale: Confirms that this case falls through to the generic "not implemented" error response.
    {
        'desc': "Test Not Implemented: OM_POWER",
        'dest': 1, 'msg_type': MESSAGE_TYPES['OM_POWER'], 'body': 'EXTPWR 1',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['ERROR_MSG']],
        'expected_body_content': "OM_POWER not implemented"
    }, # Rationale: Confirms that this case falls through to the generic "not implemented" error response.
    {
        'desc': "Test Not Implemented: SELFTEST",
        'dest': 1, 'msg_type': MESSAGE_TYPES['SELFTEST'], 'body': '1',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['ERROR_MSG']],
        'expected_body_content': "SELFTEST not implemented"
    }, # Rationale: Confirms that this case falls through to the generic "not implemented" error response.

    # =================================================================
    # IV. Navigation and Status
    # =================================================================
    {
        'desc': "Request Bandwidth",
        'dest': 1, 'msg_type': MESSAGE_TYPES['OM_BANDWIDTH_REQ'], 'body': '',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['OM_BANDWIDTH']]
    }, # Rationale: Tests case OM_BANDWIDTH_REQ, which reads from om_status and sends back formatted rates.
    {
        'desc': "Request Status",
        'dest': 1, 'msg_type': MESSAGE_TYPES['OM_STATUSREQ'], 'body': '',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['OM_STATUS']]
    }, # Rationale: Tests case OM_STATUSREQ, which sends back the current link status message string from shared memory.
    {
        'desc': "Request Nav ID",
        'dest': 1, 'msg_type': MESSAGE_TYPES['OM_NAVID_REQ'], 'body': '',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['OM_NAVID_RESP']],
        'expected_body_content': 'NAVID'
    }, # Rationale: Tests case OM_NAVID_REQ, which returns a formatted string from nav info in shared memory.
    {
        'desc': "Submit Nav Data (Valid)",
        'dest': 1, 'msg_type': MESSAGE_TYPES['OM_NAVDATA'], 'body': '1 1678886400 0 1678886399 900000000 10.5 -5.2 0.1 0.1 1.0 -1.0 123',
        'expect_response': False
    }, # Rationale: Happy path for case OM_NAVDATA. `sscanf` succeeds (retval == 12), and no response is sent.
    {
        'desc': "Submit Nav Data (Malformed)",
        'dest': 1, 'msg_type': MESSAGE_TYPES['OM_NAVDATA'], 'body': '1 1678886400 0',
        'expect_response': False
    }, # Rationale: Failure path for OM_NAVDATA. `sscanf` retval is not 12, an error is logged, but no response is sent.
    {
        'desc': "Enable Wake Request (if built)",
        'dest': 1, 'msg_type': MESSAGE_TYPES['OM_WAKE_REQ'], 'body': '1',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['OM_WAKE_RESP']]
    }, # Rationale: Tests case OM_WAKE_REQ. Response depends on whether code was compiled with OM_WAKE flag.
    {
        'desc': "Malformed Wake Request (Invalid argument)",
        'dest': 1, 'msg_type': MESSAGE_TYPES['OM_WAKE_REQ'], 'body': '2',
        'expect_response': True,
        'expected_response_types': [MESSAGE_TYPES['OM_WAKE_RESP']],
        'expected_body_content': "ERROR Invalid argument to OM_WAKE_REQ"
    }, # Rationale: Tests failure path of OM_WAKE_REQ. Sending a value other than 0 or 1 results in a specific error message.
]


# Utility functions for test cases
def get_message_name(msg_type):
    """Get message name from message type ID"""
    for name, value in MESSAGE_TYPES.items():
        if value == msg_type:
            return name
    return f"UNKNOWN_{msg_type}"


def get_process_name(process_id):
    """Get process name from process ID"""
    for name, value in PROCESS_IDS.items():
        if value == process_id:
            return name
    return f"UNKNOWN_{process_id}"


def is_deprecated_message(msg_type):
    """Check if message type is deprecated based on comments in header"""
    deprecated_types = {
        MESSAGE_TYPES['FTP_LDIR'],
        MESSAGE_TYPES['FTP_RDIR'],
        MESSAGE_TYPES['FTP_RDIR_ACK'],
        MESSAGE_TYPES['FTP_PWD'],
        MESSAGE_TYPES['FTP_CD'],
        MESSAGE_TYPES['FTP_GET_DIR'],
        MESSAGE_TYPES['FTP_GET_FILE'],
        MESSAGE_TYPES['FTP_GET_NEXT_FILE'],
        MESSAGE_TYPES['FTP_GET_FILE_ACK'],
        MESSAGE_TYPES['FTP_RESEND_REQ'],
        MESSAGE_TYPES['FTP_RESEND_PKT'],
        MESSAGE_TYPES['FTP_SET_ARG'],
        MESSAGE_TYPES['FTP_SEND_REQ'],
        MESSAGE_TYPES['FTP_SEND_ACK']
    }
    return msg_type in deprecated_types