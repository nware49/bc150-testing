import socket
import time
from test_cases import test_sequence

DEST_IP = "192.168.2.51"
DEST_PORT = 2200  # Matches modem's receiving port
SRC_ID = 10  # GUI
LISTEN_PORT = 2013  # Listen on port 2013 for responses

# Test results tracking
test_results = []


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


def validate_response(test_case, responses):
    """
    Validate received responses against expected criteria
    Returns (passed, details)
    """
    expected_msg_types = test_case.get('expected_response_types', [])
    expected_body_content = test_case.get('expected_body_content', None)

    if not test_case.get('expect_response', False):
        # Test doesn't expect a response
        if not responses:
            return True, "No response expected and none received"
        else:
            return False, f"No response expected but received {len(responses)} response(s)"

    if not responses:
        return False, "Expected response but none received"

    # Check if we have the expected message types
    received_msg_types = [resp['msg_type'] for resp in responses]

    # If expected_msg_types is empty, just check that we got some response
    if not expected_msg_types:
        return True, f"Response received with message types: {received_msg_types}"

    # Check if all expected message types are present
    missing_types = []
    for expected_type in expected_msg_types:
        if expected_type not in received_msg_types:
            missing_types.append(expected_type)

    if missing_types:
        return False, f"Missing expected message types: {missing_types}, received: {received_msg_types}"

    # Check body content if specified
    if expected_body_content is not None:
        body_matches = []
        for response in responses:
            if response['msg_type'] in expected_msg_types:
                if expected_body_content in response['body']:
                    body_matches.append(True)
                else:
                    body_matches.append(False)

        if not any(body_matches):
            response_bodies = [resp['body'] for resp in responses if resp['msg_type'] in expected_msg_types]
            return False, f"Expected body content '{expected_body_content}' not found in responses: {response_bodies}"

    return True, f"All expected responses received: {received_msg_types}"


def print_test_summary():
    """Print a summary of all test results"""
    print("\n" + "=" * 60)
    print("TEST SUMMARY")
    print("=" * 60)

    passed = 0
    failed = 0

    for result in test_results:
        status = "PASS" if result['passed'] else "FAIL"
        print(f"{status:4} | {result['desc']}")
        if not result['passed']:
            print(f"     | Details: {result['details']}")

        if result['passed']:
            passed += 1
        else:
            failed += 1

    print("-" * 60)
    print(f"Total: {len(test_results)} tests | Passed: {passed} | Failed: {failed}")
    print("=" * 60)


def main():
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.settimeout(1.0)

        # Bind the socket to listen on port 2013
        sock.bind(('', LISTEN_PORT))
        print(f"Listening for responses on port {LISTEN_PORT}...")

        for test in test_sequence:
            print(f"\nTest: {test['desc']}")
            send_message(sock, test['dest'], SRC_ID, test['msg_type'], test['body'])

            # Always try to receive responses, even if not expected
            responses = receive_responses(sock, timeout=test.get('timeout', 1.0))

            # Display received responses
            if responses:
                for i, response in enumerate(responses):
                    print(f"  Received [{response['msg_type']}] from {response['src']}: {response['body']}")
            else:
                print("  No response received (timeout or ignored status message)")

            # Validate the response
            passed, details = validate_response(test, responses)
            status = "PASS" if passed else "FAIL"
            print(f"  Result: {status} - {details}")

            # Store test result
            test_results.append({
                'desc': test['desc'],
                'passed': passed,
                'details': details,
                'responses': responses
            })

            # Introduce a pause between sending messages to allow time for responses
            time.sleep(test.get('delay', 0.5))

        # Print summary at the end
        print_test_summary()


if __name__ == "__main__":
    main()