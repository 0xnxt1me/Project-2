import hashlib
import sys


def main():
    if len(sys.argv) < 4:
        print("Usage: python3 hashing_password.py <username> <hex_salt> <password>")
        sys.exit(1)
    user = sys.argv[1]
    salt = sys.argv[2]
    password = sys.argv[3]
    
    user_bytes = user.encode('utf-8')
    salt_bytes = bytes.fromhex(salt)
    password_bytes = password.encode('utf-8')
    
    combined_bytes = user_bytes + salt_bytes + password_bytes
    
    result_hash = hashlib.md5(combined_bytes).hexdigest()
    
    print(f"MD5 Hash: {result_hash}")
    
def verify_arguments(argv):
    pass

main()