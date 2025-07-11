import paramiko
import os
import sys
import re
from dotenv import load_dotenv

# Load environment variables from .env file
load_dotenv()

# Set connection details
SSH_HOST = "192.168.2.51"
SSH_PORT = 22
SSH_USER = os.getenv("SSH_USER")
SSH_PASSWORD = os.getenv("SSH_PASSWORD")
SSH_KEY_PATH = os.getenv("SSH_KEY_PATH")  # Optional
COMMAND = "tcpdump -i lo port 3800 -XX"

def extract_readable_data(hex_dump):
    # This regex will extract readable ASCII characters that are printable and part of the packet
    ascii_data = re.findall(r'([ -~]{4,})', hex_dump)
    return ' '.join(ascii_data)

def ssh_and_run():
    if not SSH_USER or (not SSH_PASSWORD and not SSH_KEY_PATH):
        print("Missing required environment variables: SSH_USER and either SSH_PASSWORD or SSH_KEY_PATH.")
        sys.exit(1)

    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())

    try:
        if SSH_KEY_PATH:
            key = paramiko.RSAKey.from_private_key_file(SSH_KEY_PATH)
            client.connect(hostname=SSH_HOST, port=SSH_PORT, username=SSH_USER, pkey=key)
        else:
            client.connect(hostname=SSH_HOST, port=SSH_PORT, username=SSH_USER, password=SSH_PASSWORD)

        stdin, stdout, stderr = client.exec_command(COMMAND)

        print("----- OUTPUT -----")
        for line in stdout:
            # Only process lines that contain packet data (i.e., lines with hex content)
            hex_line = line.strip()
            if hex_line:
                # Extract and print readable ASCII content from the hex dump
                readable_data = extract_readable_data(hex_line)
                if readable_data:
                    print(f"Read: {readable_data}")

        print("----- ERRORS -----")
        for line in stderr:
            print(line.strip())

    except Exception as e:
        print(f"Error: {e}")
    finally:
        client.close()

if __name__ == "__main__":
    ssh_and_run()
