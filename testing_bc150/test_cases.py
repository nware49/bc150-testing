# test_cases.py
# Define the chronological test sequence

test_sequence = [
    {
        'desc': "Simple ping test",
        'dest': 0,  # SOA
        'msg_type': 2,  # PING
        'body': '',
        'expect_response': True
    },
    {
        'desc': "Request version info",
        'dest': 0,  # SOA
        'msg_type': 137,  # VERSIONREQ
        'body': '',
        'expect_response': True
    },
    {
        'desc': "Request settings",
        'dest': 0,
        'msg_type': 113,  # OM_SETREQ
        'body': '',
        'expect_response': True
    },
    {
        'desc': "Send invalid mode",
        'dest': 0,
        'msg_type': 110,  # OM_MODEREQ
        'body': '999',  # assuming mode 999 is invalid
        'expect_response': True
    },
    {
        'desc': "Send GUI IP address",
        'dest': 0,
        'msg_type': 120,  # GUI_ADDRESS
        'body': '192.168.1.100',
        'expect_response': False
    },
    {
        'desc': "Trigger wake enable",
        'dest': 0,
        'msg_type': 145,  # OM_WAKE_REQ
        'body': '1',
        'expect_response': True
    },
    {
        'desc': "Send app address with enable flag",
        'dest': 0,
        'msg_type': 138,  # APP_ADDRESS
        'body': 'GUI 192.168.1.100 1',
        'expect_response': False
    },
    {
        'desc': "Save settings",
        'dest': 0,
        'msg_type': 160,  # OM_SETTINGS_SAVE_REQ
        'body': '',
        'expect_response': True
    },
]
