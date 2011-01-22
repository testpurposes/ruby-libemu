require 'mkmf'

extension_name = 'rlibemu'

unless pkg_config('libemu')
	raise "libemu not found"
end

have_func('emu_new', 'emu/emu.h')
have_func('emu_free', 'emu/emu.h')
have_func('emu_shellcode_test', 'emu/emu_shellcode.h')

create_header
create_makefile(extension_name)
