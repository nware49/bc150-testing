import socket
import time
from test_cases import test_sequence

DEST_IP = "192.168.2.51"
DEST_PORT = 2200  # Matches modem's receiving port
SRC_ID = 2  # UCMD

def send_message(sock, dest, src, msg_type, body):
    msg_len = len(body)
    formatted = f"{dest},{src},{msg_type},{msg_len},{body}"
    sock.sendto(formatted.encode('utf-8'), (DEST_IP, DEST_PORT))
    print(f"Sent [{msg_type}] to {dest}: {formatted}")

def receive_response(sock):
    try:
        data, addr = sock.recvfrom(2048)
        message = data.decode('utf-8')
        parts = message.split(',', 4)
        if len(parts) == 5:
            dest, src, msg_type, msg_len, body = parts
            return {
                'dest': int(dest),
                'src': int(src),
                'msg_type': int(msg_type),
                'msg_len': int(msg_len),
                'body': body
            }
        else:
            print(f"Malformed response: {message}")
            return None
    except socket.timeout:
        return None

def main():
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.settimeout(1.0)

        for test in test_sequence:
            print(f"\nTest: {test['desc']}")
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
