import socket
import time
from test_cases import test_sequence

DEST_IP = "192.168.2.51"
DEST_PORT = 2011  # Matches internal modem forwarding
SRC_ID = 2  # UCMD

def send_message(sock, dest, src, msg_type, body):
    msg_len = len(body)
    formatted = f"{dest},{src},{msg_type},{msg_len},{body}"
    sock.sendto(formatted.encode('utf-8'), (DEST_IP, DEST_PORT))
    print(f"Sent [{msg_type}] to {dest}: {formatted}")

def main():
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.settimeout(1.0)

        for test in test_sequence:
            print(f"\nTest: {test['desc']}")
            send_message(sock, test['dest'], SRC_ID, test['msg_type'], test['body'])
            # The web interface does not return responses via this socket
            # All response evaluation should be done from the system logs or visual feedback
            time.sleep(test.get('delay', 0.5))

if __name__ == "__main__":
    main()
