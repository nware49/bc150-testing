import socket
import time
from test_cases import test_sequence

DEST_IP = "192.168.2.51"
DEST_PORT = 2200  # Matches modem's receiving port
SRC_ID = 10  # GUI
LISTEN_PORT = 2013  # Listen on port 2013 for responses

# Function to send a message
def send_message(sock, dest, src, msg_type, body):
    msg_len = len(body)
    msg_len = '{:0>4}'.format(msg_len)
    formatted = f"{dest},{src},{msg_type},{msg_len},{body}"
    sock.sendto(formatted.encode('utf-8'), (DEST_IP, DEST_PORT))
    print(f"Sent [{msg_type}] to {dest}: {formatted}")

# Function to receive a response (with handling for multiple responses)
def receive_responses(sock, timeout=1.0, max_responses=5):
    responses = []
    start_time = time.time()

    while True:
        if time.time() - start_time > timeout:
            break
        
        try:
            data, addr = sock.recvfrom(2048)
            message = data.decode('utf-8')
            parts = message.split(',', 4)

            if len(parts) == 5:
                dest, src, msg_type, msg_len, body = parts
                msg_type = int(msg_type)
                
                # Filter out status messages with type 126 and 127
                if msg_type in [126, 127]:
                   # print(f"Ignored status message: {message}")
                    continue
                
                responses.append({
                    'dest': int(dest),
                    'src': int(src),
                    'msg_type': msg_type,
                    'msg_len': int(msg_len),
                    'body': body
                })

                # If we've reached the maximum responses, break out
                if len(responses) >= max_responses:
                    break

        except socket.timeout:
            break

    return responses

def main():
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.settimeout(1.0)
        
        # Bind the socket to listen on port 2013
        sock.bind(('', LISTEN_PORT))
        print(f"Listening for responses on port {LISTEN_PORT}...")

        for test in test_sequence:
            print(f"\nTest: {test['desc']}")
            send_message(sock, test['dest'], SRC_ID, test['msg_type'], test['body'])

            if test.get('expect_response', False):
                responses = receive_responses(sock)
                if responses:
                    for i, response in enumerate(responses):
                        print(f"  Received [{response['msg_type']}] from {response['src']}: {response['body']}")
                else:
                    print("  No response received (timeout or ignored status message)")

            # Introduce a pause between sending messages to allow time for responses
            time.sleep(test.get('delay', 0.5))

if __name__ == "__main__":
    main()
