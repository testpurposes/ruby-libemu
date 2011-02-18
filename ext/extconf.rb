require 'mkmf'

extension_name = 'rlibemu'

unless pkg_config('libemu')
	raise "libemu not found"
end

have_func('emu_new', 'emu/emu.h')
have_func('emu_free', 'emu/emu.h')
have_func('emu_cpu_parse', 'emu/emu_cpu.h')
have_func('emu_cpu_parse', 'emu/emu_cpu.h')
have_func('emu_cpu_step', 'emu/emu_cpu.h')
have_func('emu_cpu_run', 'emu/emu_cpu.h')
have_func('emu_cpu_eip_get', 'emu/emu_cpu.h')
have_func('emu_cpu_eip_set', 'emu/emu_cpu.h')
have_func('emu_cpu_reg32_get', 'emu/emu_cpu.h')
have_func('emu_cpu_reg32_set', 'emu/emu_cpu.h')
have_func('emu_cpu_reg16_get', 'emu/emu_cpu.h')
have_func('emu_cpu_reg16_set', 'emu/emu_cpu.h')
have_func('emu_cpu_reg8_get', 'emu/emu_cpu.h')
have_func('emu_cpu_reg8_set', 'emu/emu_cpu.h')
have_func('emu_cpu_eflags_get', 'emu/emu_cpu.h')
have_func('emu_cpu_eflags_set', 'emu/emu_cpu.h')
have_func('emu_cpu_debugflag_set', 'emu/emu_cpu.h')
have_func('emu_cpu_debugflag_unset', 'emu/emu_cpu.h')
have_func('emu_memory_get_usage', 'emu/emu_memory.h')
have_func('emu_memory_mode_ro', 'emu/emu_memory.h')
have_func('emu_memory_mode_rw', 'emu/emu_memory.h')
have_func('emu_memory_read_byte', 'emu/emu_memory.h')
have_func('emu_memory_read_word', 'emu/emu_memory.h')
have_func('emu_memory_read_dword', 'emu/emu_memory.h')
have_func('emu_memory_read_block', 'emu/emu_memory.h')
have_func('emu_memory_write_byte', 'emu/emu_memory.h')
have_func('emu_memory_write_word', 'emu/emu_memory.h')
have_func('emu_memory_write_dword', 'emu/emu_memory.h')
have_func('emu_memory_write_block', 'emu/emu_memory.h')
have_func('emu_memory_segment_select', 'emu/emu_memory.h')
have_func('emu_memory_segment_get', 'emu/emu_memory.h')
have_func('emu_log_level_set', 'emu/emu_log.h')
have_func('emu_shellcode_test', 'emu/emu_shellcode.h')
have_func('emu_env_new', 'emu/environment/emu_env.h')
have_func('emu_env_free', 'emu/environment/emu_env.h')
have_func('emu_env_linux_new', 'emu/environment/linux/emu_env_linux.h')
have_func('emu_env_w32_new', 'emu/environment/win32/emu_env_w32.h')
have_func('emu_profile_new', 'emu/environment/emu_profile.h')
have_func('emu_profile_debug', 'emu/environment/emu_profile.h');
have_func('emu_env_linux_free', 'emu/environment/linux/emu_env_linux.h');

create_header
create_makefile(extension_name)
