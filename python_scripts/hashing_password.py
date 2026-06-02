import hashlib
import sys


def main():
    verify_arguments(sys.argv)
    salt = sys.argv[1]
    password = sys.argv[2]
    
    salt_bytes = bytes.fromhex(salt)
    password_bytes = password.encode('utf-8')
    
    combined_bytes = salt_bytes + password_bytes
    
    result_hash = hashlib.md5(combined_bytes).hexdigest()
    
    print(f"MD5 Hash: {result_hash}")
    
def verify_arguments(argv):
    pass

main()