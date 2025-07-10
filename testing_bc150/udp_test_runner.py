# udp_test_runner.py
# Main testing script that sends and receives messages via UDP

import socket
import time
import struct
from test_cases import test_sequence

DEST_IP = "192.168.1.50"
DEST_PORT = 3800
SRC_ID = 2  # UCMD

# Message format: int dest, int src, int msg_type, int msg_len + msg_body (bytes)
HEADER_FORMAT = "!iiii"  # 4 integers, network byte order


def send_message(sock, dest, src, msg_type, body):
    body_bytes = body.encode('utf-8')
    header = struct.pack(HEADER_FORMAT, dest, src, msg_type, len(body_bytes))
    message = header + body_bytes
    sock.sendto(message, (DEST_IP, DEST_PORT))
    print(f"Sent [{msg_type}] to {dest}: {body}")


def receive_response(sock):
    try:
        data, addr = sock.recvfrom(2048)
        if len(data) >= 16:
            hdr = struct.unpack(HEADER_FORMAT, data[:16])
            body = data[16:].decode(errors='ignore')
            return {'dest': hdr[0], 'src': hdr[1], 'msg_type': hdr[2], 'msg_len': hdr[3], 'body': body}
        return None
    except socket.timeout:
        return None


def main():
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.settimeout(1.0)

        for test in test_sequence:
            send_message(sock, test['dest'], SRC_ID, test['msg_type'], test['body'])
            if test.get('expect_response', False):
                response = receive_response(sock)
                if response:
                    print(f"  Received [{response['msg_type']}] from {response['src']}: {response['body']}")
                else:
                    print("  No response received (timeout)")
            time.sleep(test.get('delay', 0.5))


if __name__ == "__main__":
    main()
