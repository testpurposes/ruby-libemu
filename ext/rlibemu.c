#include <ruby.h>
#include "environment.h"

#include <emu/emu.h>
#include <emu/emu_cpu.h>
#include <emu/emu_memory.h>
#include <emu/emu_log.h>
#include <emu/emu_shellcode.h>

#define EAX 0
#define ECX 1
#define EDX 2
#define EBX 3
#define ESP 4
#define EBP 5
#define ESI 6
#define EDI 7

#define AX 0 /* eax */
#define CX 1 /* ecx */
#define DX 2 /* edx */
#define BX 3 /* ebx */
#define SP 4 /* esp */
#define BP 5 /* ebp */
#define SI 6 /* esi */
#define DI 7 /* edi */


#define AL 0 /* eax */
#define CL 1 /* ecx */
#define DL 2 /* edx */
#define BL 3 /* ebx */
#define AH 4 /* eax */
#define CH 5 /* ecx */
#define DH 6 /* edx */
#define BH 7 /* ebx */

/* segments */
#define CS 0
#define SS 1
#define DS 2
#define ES 3
#define FS 4
#define GS 5

/* debug flags */
#define DEBUG_FLAG_INST_STRING 0
#define DEBUG_FLAG_INST_SIZE 1

/* log levels */
#define EMU_LOG_NONE 0
#define EMU_LOG_INFO 1
#define EMU_LOG_DEBUG 2

VALUE m_libemu;
static VALUE c_emulator;
static VALUE emulator_allocate(VALUE);
static void emulator_free (struct emu *);
static VALUE emulator_test(VALUE, VALUE);
static VALUE emulator_cpu_parse(VALUE);
static VALUE emulator_cpu_step(VALUE);
static VALUE emulator_cpu_run(VALUE);
static VALUE emulator_cpu_eip_get(VALUE);
static VALUE emulator_cpu_eip_set(VALUE, VALUE);
static VALUE emulator_cpu_reg32_get(VALUE, VALUE);
static VALUE emulator_cpu_reg32_set(VALUE, VALUE, VALUE);
static VALUE emulator_cpu_reg16_get(VALUE, VALUE);
static VALUE emulator_cpu_reg16_set(VALUE, VALUE, VALUE);
static VALUE emulator_cpu_reg8_get(VALUE, VALUE);
static VALUE emulator_cpu_reg8_set(VALUE, VALUE, VALUE);
static VALUE emulator_cpu_eflags_get(VALUE);
static VALUE emulator_cpu_eflags_set(VALUE, VALUE);
static VALUE emulator_memory_get_usage(VALUE);
static VALUE emulator_memory_read_byte(VALUE, VALUE);
static VALUE emulator_memory_read_word(VALUE, VALUE);
static VALUE emulator_memory_read_dword(VALUE, VALUE);
static VALUE emulator_memory_read_block(VALUE, VALUE, VALUE);
static VALUE emulator_memory_write_byte(VALUE, VALUE, VALUE);
static VALUE emulator_memory_write_word(VALUE, VALUE, VALUE);
static VALUE emulator_memory_write_dword(VALUE, VALUE, VALUE);
static VALUE emulator_memory_write_block(VALUE, VALUE, VALUE);
static VALUE emulator_memory_mode_ro(VALUE);
static VALUE emulator_memory_mode_rw(VALUE);
static VALUE emulator_memory_segment_select(VALUE, VALUE);
static VALUE emulator_memory_segment_get(VALUE);
static VALUE emulator_log_level_set(VALUE, VALUE);
static VALUE emulator_env_new(VALUE);
static VALUE emulator_env_free(VALUE);
static VALUE emulator_profile_debug(VALUE);
void Init_rlibemu();

/*nanny_new
 * call-seq:
 * 	emu.log_level_set(level) -> true
 * 
 * Set the log level:
 * 	Emulator::EMU_LOG_NONE
 * 	Emulator::EMU_LOG_INFO
 * 	Emulator::EMU_LOG_DEBUG
 * 
 */
static VALUE emulator_log_level_set(klass, level)
	VALUE klass;
	VALUE level;
{
	struct emu *emulator;
	
	FIXNUM_P(level);
	Data_Get_Struct(klass, struct emu, emulator);
	emu_log_level_set(emu_logging_get(emulator), FIX2UINT(level));
	return Qtrue;
}

/*
 * call-seq:
 * 	emu.memory_segment_select(segment) -> fixnum
 * 
 * Set the memory segment. Returns the selected segment:
 * 	Emulator::CS
 * 	Emulator::SS
 * 	Emulator::DS
 * 	Emulator::ES
 * 	Emulator::FS
 * 	Emulator::GS
 * 
 */
static VALUE emulator_memory_segment_select(klass, segment)
	VALUE klass;
	VALUE segment;
{
	struct emu *emulator;
	
	FIXNUM_P(segment);
	Data_Get_Struct(klass, struct emu, emulator);
	emu_memory_segment_select(emu_memory_get(emulator), FIX2UINT(segment));
	return segment;
}

/*
 * call-seq:
 * 	emu.memory_segment_get() -> integer
 * 
 * Get the current memory segment:
 * 	Emulator::CS
 * 	Emulator::SS
 * 	Emulator::DS
 * 	Emulator::ES
 * 	Emulator::FS
 * 	Emulator::GS
 * 
 */
static VALUE emulator_memory_segment_get(klass)
	VALUE klass;
{
	struct emu *emulator;
	uint32_t c_result;
	
	Data_Get_Struct(klass, struct emu, emulator);
	c_result = emu_memory_segment_get(emu_memory_get(emulator));
	return UINT2NUM(c_result);
}

/*
 * call-seq:
 * 	emu.memory_mode_ro => true
 * 
 *  Set the memory access to read only.
 * 
 */
static VALUE emulator_memory_mode_ro(klass)
	VALUE klass;
{
	struct emu *emulator;
	
	Data_Get_Struct(klass, struct emu, emulator);
	emu_memory_mode_ro(emu_memory_get(emulator));
	return Qtrue;
}

/*
 * call-seq:
 * 	emu.memory_mode_rw => true
 * 
 *  Set the memory access to read and write.
 * 
 */
static VALUE emulator_memory_mode_rw(klass) 
	VALUE klass;
{
	struct emu *emulator;
	
	Data_Get_Struct(klass, struct emu, emulator);
	emu_memory_mode_rw(emu_memory_get(emulator));
	return Qtrue;
}

/*
 * call-seq:
 * 	emu.memory_get_usage() -> integer
 * 
 * Get information about memory usage. Returns the size of memory used 
 * (allocated) in bytes.
 * 
 */
static VALUE emulator_memory_get_usage(klass) 
	VALUE klass;
{
	struct emu *emulator;
	uint32_t c_result;
	
	Data_Get_Struct(klass, struct emu, emulator);
	c_result = emu_memory_get_usage(emu_memory_get(emulator));
	return UINT2NUM(c_result);
}

/*
 * call-seq:
 * 	emu.memory_write_block(addr, src) -> fixnum
 * 
 * Write access. If success returns 0. If addr is read only returns 0. 
 * If error (allocating memory) returns -1.
 * 
 */
static VALUE emulator_memory_write_block(klass, addr, src)
	VALUE klass;
	VALUE addr;
	VALUE src;
{
	struct emu *emulator;
	int32_t c_result;
	
	FIXNUM_P(addr);
	Check_Type(src, T_STRING);
	Data_Get_Struct(klass, struct emu, emulator);
	c_result = emu_memory_write_block(emu_memory_get(emulator), 
										NUM2UINT(addr), 
										RSTRING_PTR(src),
										RSTRING_LEN(src));
	return INT2FIX(c_result);
}

/*
 * call-seq:
 * 	emu.memory_write_byte(addr, byte) -> fixnum
 * 
 * Write access. If success returns 0. If addr is read only returns 0. 
 * If error (allocating memory) returns -1.
 * 
 */
static VALUE emulator_memory_write_byte(klass, addr, byte)
	VALUE klass;
	VALUE addr;
	VALUE byte;
{
	struct emu *emulator;
	int32_t c_result;
	
	FIXNUM_P(addr);
	FIXNUM_P(byte);
	Data_Get_Struct(klass, struct emu, emulator);
	c_result = emu_memory_write_byte(emu_memory_get(emulator), 
										NUM2UINT(addr), 
										(uint8_t)NUM2UINT(byte));
	return INT2FIX(c_result);
}

/*
 * call-seq:
 * 	emu.memory_write_word(addr, word) -> fixnum
 * 
 * Write access. If success returns 0. If addr is read only returns 0. 
 * If error (allocating memory) returns -1.
 * 
 */
static VALUE emulator_memory_write_word(klass, addr, word)
	VALUE klass;
	VALUE addr;
	VALUE word;
{
	struct emu *emulator;
	int32_t c_result;
	
	FIXNUM_P(addr);
	FIXNUM_P(word);
	Data_Get_Struct(klass, struct emu, emulator);
	c_result = emu_memory_write_word(emu_memory_get(emulator), 
										NUM2UINT(addr), 
										(uint16_t)NUM2UINT(word));
	return INT2FIX(c_result);
}

/*
 * call-seq:
 * 	emu.memory_write_dword(addr, dword) -> fixnum
 * 
 * Write access. If success returns 0. If addr is read only returns 0. 
 * If error (allocating memory) returns -1.
 * 
 */
static VALUE emulator_memory_write_dword(klass, addr, dword)
	VALUE klass;
	VALUE addr;
	VALUE dword;
{
	struct emu *emulator;
	int32_t c_result;
	
	FIXNUM_P(addr);
	FIXNUM_P(dword);
	Data_Get_Struct(klass, struct emu, emulator);
	c_result = emu_memory_write_dword(emu_memory_get(emulator), 
										NUM2UINT(addr), 
										NUM2UINT(dword));
	return INT2FIX(c_result);
}

/*
 * call-seq:
 * 	emu.memory_read_block(addr, len) -> string
 * 
 * Read len bytes from addr. If success returns a string of len 
 * btyes with the contents read from addr. If error returns nil.
 * 
 */
static VALUE emulator_memory_read_block(klass, addr, len)
	VALUE klass;
	VALUE addr;
	VALUE len;
{
	struct emu *emulator;
	VALUE str;
	uint32_t n;
	
	FIXNUM_P(addr);
	FIXNUM_P(len);
	
	n = NUM2UINT(len);
	str = rb_str_new(0, n);
	Data_Get_Struct(klass, struct emu, emulator);
	
	if (emu_memory_read_block(emu_memory_get(emulator), 
								NUM2UINT(addr), 
								RSTRING_PTR(str),
								n) == -1) {
		return Qnil;
	} 
	return str;
}

/*
 * call-seq:
 * 	emu.memory_read_dword(addr) -> integer
 * 
 * Read access. If success returns the dword value at address addr. If 
 * error returns -1.
 * 
 */
static VALUE emulator_memory_read_dword(VALUE klass, VALUE addr)
{
	struct emu *emulator;
	uint32_t c_value;
	
	FIXNUM_P(addr);
	Data_Get_Struct(klass, struct emu, emulator);
	if (emu_memory_read_dword(emu_memory_get(emulator), 
								NUM2UINT(addr), 
								&c_value) == -1) {
		return INT2NUM(-1);
	} 
	return UINT2NUM(c_value);
}

/*
 * call-seq:
 * 	emu.memory_read_word(addr) -> integer
 * 
 * Read access. If success returns the word value at address addr. If 
 * error returns -1.
 * 
 */
static VALUE emulator_memory_read_word(VALUE klass, VALUE addr)
{
	struct emu *emulator;
	uint16_t c_value;
	
	FIXNUM_P(addr);
	Data_Get_Struct(klass, struct emu, emulator);
	if (emu_memory_read_word(emu_memory_get(emulator), 
								NUM2UINT(addr), 
								&c_value) == -1) {
		return INT2NUM(-1);
	} 
	return UINT2NUM(c_value);
}

/*
 * call-seq:
 * 	emu.memory_read_byte(addr) -> integer
 * 
 * Read access. If success returns the byte value at address addr. If 
 * error returns -1.
 * 
 */
static VALUE emulator_memory_read_byte(VALUE klass, VALUE addr)
{
	struct emu *emulator;
	uint8_t c_value;
	
	FIXNUM_P(addr);
	Data_Get_Struct(klass, struct emu, emulator);
	if (emu_memory_read_byte(emu_memory_get(emulator), 
								NUM2UINT(addr), 
								&c_value) == -1) {
		return INT2NUM(-1);
	} 
	return UINT2NUM(c_value);
}

/*
 * call-seq:
 * 	emu.cpu_debugflag_unset(flag) -> true
 * 
 * Unsets the cpu debug flag:
 * 	Emulator::DEBUG_FLAG_INST_STRING
 *	Emulator::DEBUG_FLAG_INST_SIZE
 * 
 * Returns true
 * 
 */
static VALUE emulator_cpu_debugflag_unset(VALUE klass, VALUE flag)
{
	struct emu *emulator;
	
	FIXNUM_P(flag);
	Data_Get_Struct(klass, struct emu, emulator);
	emu_cpu_debugflag_unset(emu_cpu_get(emulator), (uint8_t)FIX2UINT(flag));
	return Qtrue;
}

/*
 * call-seq:
 * 	emu.cpu_debugflag_set(flag) -> true
 * 
 * Set the cpu debug flag:
 *	Emulator::DEBUG_FLAG_INST_STRING
 *	Emulator::DEBUG_FLAG_INST_SIZE
 * 
 * Returns true
 * 
 */
static VALUE emulator_cpu_debugflag_set(VALUE klass, VALUE flag)
{
	struct emu *emulator;
	
	FIXNUM_P(flag);
	Data_Get_Struct(klass, struct emu, emulator);
	emu_cpu_debugflag_set(emu_cpu_get(emulator), (uint8_t)FIX2UINT(flag));
	return Qtrue;
}

/*
 * call-seq:
 * 	emu.cpu_eflags_set(value) -> fixnum
 * 
 * Set the cpu's eflags. If success returns the new value of eflags.
 * 
 */
static VALUE emulator_cpu_eflags_set(VALUE klass, VALUE value)
{
	struct emu *emulator;
	
	FIXNUM_P(value);
	Data_Get_Struct(klass, struct emu, emulator);
	emu_cpu_eflags_set(emu_cpu_get(emulator), NUM2UINT(value));
	return value;
}

/*
 * call-seq:
 * 	emu.cpu_eflags_get() -> integer
 * 
 * Get the cpu's eflags.
 * 
 */
static VALUE emulator_cpu_eflags_get(VALUE klass)
{
	struct emu *emulator;
	uint32_t c_result;
	
	Data_Get_Struct(klass, struct emu, emulator);
	c_result = emu_cpu_eflags_get(emu_cpu_get(emulator));
	UINT2NUM(c_result);
}

/*
 * call-seq:
 * 	emu.cpu_reg8_set(reg8, value) -> fixnum
 * 
 * Set the cpu's reg8 to value. If success returns the new value of 
 * reg8.
 * 
 */
static VALUE emulator_cpu_reg8_set(VALUE klass, VALUE reg8, VALUE value)
{
	struct emu *emulator;
	
	FIXNUM_P(reg8);
	FIXNUM_P(value);
	Data_Get_Struct(klass, struct emu, emulator);
	emu_cpu_reg8_set(emu_cpu_get(emulator), FIX2INT(reg8), (uint8_t)NUM2UINT(value));
	return value;
}

/*
 * call-seq:
 * 	emu.cpu_reg8_get(reg8) -> integer
 * 
 * Get the cpu's reg16:
 *		Emulator::AL
 *		Emulator::CL
 *		Emulator::DL
 *		Emulator::BL
 *		Emulator::AX
 *		Emulator::CX
 *		Emulator::DX
 *		Emulator::BX
 * 
 */
static VALUE emulator_cpu_reg8_get(VALUE klass, VALUE reg8)
{
	struct emu *emulator;
	uint8_t c_result;
	
	FIXNUM_P(reg8);
	Data_Get_Struct(klass, struct emu, emulator);
	c_result = emu_cpu_reg8_get(emu_cpu_get(emulator), FIX2INT(reg8));
	UINT2NUM(c_result);
}

/*
 * call-seq:
 * 	emu.cpu_reg16_set(reg16, value) -> fixnum
 * 
 * Set the cpu's reg16 to value. If success returns the new value of 
 * reg16.
 * 
 */
static VALUE emulator_cpu_reg16_set(VALUE klass, VALUE reg16, VALUE value)
{
	struct emu *emulator;
	
	FIXNUM_P(reg16);
	FIXNUM_P(value);
	Data_Get_Struct(klass, struct emu, emulator);
	emu_cpu_reg16_set(emu_cpu_get(emulator), FIX2INT(reg16), (uint16_t)NUM2UINT(value));
	return value;
}

/*
 * call-seq:
 * 	emu.cpu_reg16_get(reg16) -> integer
 * 
 * Get the cpu's reg16:
 *		Emulator::AX
 *		Emulator::CX
 *		Emulator::DX
 *		Emulator::BX
 *		Emulator::SP
 *		Emulator::BP
 *		Emulator::SI
 *		Emulator::DI
 * 
 */
static VALUE emulator_cpu_reg16_get(VALUE klass, VALUE reg16)
{
	struct emu *emulator;
	uint16_t c_result;
	
	FIXNUM_P(reg16);
	Data_Get_Struct(klass, struct emu, emulator);
	c_result = emu_cpu_reg16_get(emu_cpu_get(emulator), FIX2INT(reg16));
	UINT2NUM(c_result);
}

/*
 * call-seq:
 * 	emu.cpu_reg32_set(reg32, value) -> fixnum
 * 
 * Set the cpu's reg32 to value. If success returns the new value of 
 * reg32.
 * 
 */
static VALUE emulator_cpu_reg32_set(VALUE klass, VALUE reg32, VALUE value)
{
	struct emu *emulator;
	
	FIXNUM_P(reg32);
	FIXNUM_P(value);
	Data_Get_Struct(klass, struct emu, emulator);
	emu_cpu_reg32_set(emu_cpu_get(emulator), FIX2INT(reg32), NUM2UINT(value));
	return value;
}

/*
 * call-seq:
 * 	emu.cpu_reg32_get(reg32) -> integer
 * 
 * Get the cpu's REG32:
 *		Emulator::EAX
 *		Emulator::ECX
 *		Emulator::EDX
 *		Emulator::EBX
 *		Emulator::ESP
 *		Emulator::EBP
 *		Emulator::ESI
 *		Emulator::EDI
 * 
 */
static VALUE emulator_cpu_reg32_get(VALUE klass, VALUE reg32)
{
	struct emu *emulator;
	uint32_t c_result;
	
	FIXNUM_P(reg32);
	Data_Get_Struct(klass, struct emu, emulator);
	c_result = emu_cpu_reg32_get(emu_cpu_get(emulator), FIX2INT(reg32));
	UINT2NUM(c_result);
}

/*
 * call-seq:
 * 	emu.cpu_eip_set(eip) -> fixnum
 * 
 * Set the cpu's EIP. If success returns the new value of EIP.
 * 
 */
static VALUE emulator_cpu_eip_set(VALUE klass, VALUE eip)
{
	struct emu *emulator;
	
	FIXNUM_P(eip);
	Data_Get_Struct(klass, struct emu, emulator);
	emu_cpu_eip_set(emu_cpu_get(emulator), NUM2UINT(eip));
	return eip;
}

/*
 * call-seq:
 * 	emu.cpu_eip_get() -> integer
 * 
 * Get the cpu's EIP.
 * 
 */
static VALUE emulator_cpu_eip_get(VALUE klass)
{
	struct emu *emulator;
	uint32_t c_result;
	
	Data_Get_Struct(klass, struct emu, emulator);
	c_result = emu_cpu_eip_get(emu_cpu_get(emulator));
	UINT2NUM(c_result);
}

/*
 * call-seq:
 * 	emu.cpu_parse() -> fixnum
 * 
 * Parse a instruction at EIP. On success, returns 0 and on error,
 * returns -1.
 * 
 */
static VALUE emulator_cpu_parse(VALUE klass)
{
	struct emu *emulator;
	int32_t c_result;
	
	Data_Get_Struct(klass, struct emu, emulator);
	c_result = emu_cpu_parse(emu_cpu_get(emulator));
	return INT2FIX(c_result);
}

/*
 * call-seq:
 * 	emu.cpu_step() -> fixnum
 * 
 * Steps the last instruction. On success, returns 0 and on error, 
 * returns -1.
 * 
 */

static VALUE emulator_cpu_step(VALUE klass)
{
	struct emu *emulator;
	int32_t c_result;
	
	Data_Get_Struct(klass, struct emu, emulator);
	c_result = emu_cpu_step(emu_cpu_get(emulator));
	return INT2FIX(c_result);
}

/*
 * call-seq:
 * 	emu.cpu_run() -> fixnum
 * 
 * Parses and steps instructions from EIP. Returns the number of
 * instructions parsed and stepped.
 * 
 */

static VALUE emulator_cpu_run(VALUE klass)
{
	struct emu *emulator;
	int32_t c_result;
	
	Data_Get_Struct(klass, struct emu, emulator);
	c_result = emu_cpu_run(emu_cpu_get(emulator));
	return INT2FIX(c_result);
}

/*
 * call-seq:
 *	emu.test(sc) -> fixnum
 * 
 * Tests a given buffer for possible shellcodes. On success, returns 
 * the offset within the buffer where the shellcode is suspected. On
 * failure (no shellcode detected), returns -1
 *
 */
static VALUE emulator_test(VALUE klass, VALUE sc) 
{
	VALUE shellcode;
	char *c_shellcode;
	long len_shellcode;
	int32_t c_result;
	struct emu *emulator;
	
	// Get emulator
	Data_Get_Struct(klass, struct emu, emulator);
	
	// Get Shellcode
	SafeStringValue(sc);
	shellcode = StringValue(sc);
	c_shellcode = RSTRING_PTR(shellcode);
	len_shellcode = RSTRING_LEN(shellcode);

	// test shellcode
	c_result = emu_shellcode_test(emulator, (uint8_t *)c_shellcode, (uint16_t) len_shellcode);
	
	return INT2FIX(c_result);
}

static void emulator_free (struct emu *emulator)
{
  emu_free(emulator);
}

static VALUE emulator_allocate(VALUE klass)
{
  struct emu *emulator = emu_new();

  return Data_Wrap_Struct(klass, NULL, emulator_free, emulator);
}

void Init_rlibemu() 
{
	m_libemu = rb_define_module("Libemu");
	/*
	 * Class Libemu::Emulator
	 */
	c_emulator = rb_define_class_under(m_libemu, 
										"Emulator", 
										rb_cObject);
	rb_define_alloc_func(c_emulator, emulator_allocate);
	rb_define_const(c_emulator, "EAX", INT2FIX(EAX));
	rb_define_const(c_emulator, "ECX", INT2FIX(ECX));
	rb_define_const(c_emulator, "EDX", INT2FIX(EDX));
	rb_define_const(c_emulator, "EBX", INT2FIX(EBX));
	rb_define_const(c_emulator, "ESP", INT2FIX(ESP));
	rb_define_const(c_emulator, "EBP", INT2FIX(EBP));
	rb_define_const(c_emulator, "ESI", INT2FIX(ESI));
	rb_define_const(c_emulator, "EDI", INT2FIX(EDI));
	rb_define_const(c_emulator, "AX", INT2FIX(AX));
	rb_define_const(c_emulator, "CX", INT2FIX(CX));
	rb_define_const(c_emulator, "DX", INT2FIX(DX));
	rb_define_const(c_emulator, "BX", INT2FIX(BX));
	rb_define_const(c_emulator, "SP", INT2FIX(SP));
	rb_define_const(c_emulator, "BP", INT2FIX(BP));
	rb_define_const(c_emulator, "SI", INT2FIX(SI));
	rb_define_const(c_emulator, "DI", INT2FIX(DI));
	rb_define_const(c_emulator, "AL", INT2FIX(AL));
	rb_define_const(c_emulator, "CL", INT2FIX(CL));
	rb_define_const(c_emulator, "DL", INT2FIX(DL));
	rb_define_const(c_emulator, "BL", INT2FIX(BL));
	rb_define_const(c_emulator, "AH", INT2FIX(AL));
	rb_define_const(c_emulator, "CH", INT2FIX(CL));
	rb_define_const(c_emulator, "DH", INT2FIX(DL));
	rb_define_const(c_emulator, "BH", INT2FIX(BL));
	rb_define_const(c_emulator, "CS", INT2FIX(CS));
	rb_define_const(c_emulator, "SS", INT2FIX(SS));
	rb_define_const(c_emulator, "DS", INT2FIX(DS));
	rb_define_const(c_emulator, "ES", INT2FIX(ES));
	rb_define_const(c_emulator, "FS", INT2FIX(FS));
	rb_define_const(c_emulator, "GS", INT2FIX(GS));
	rb_define_const(c_emulator, "DEBUG_FLAG_INST_STRING", INT2FIX(DEBUG_FLAG_INST_STRING));
	rb_define_const(c_emulator, "DEBUG_FLAG_INST_SIZE", INT2FIX(DEBUG_FLAG_INST_SIZE));
	rb_define_const(c_emulator, "EMU_LOG_NONE", INT2FIX(EMU_LOG_NONE));
	rb_define_const(c_emulator, "EMU_LOG_INFO", INT2FIX(EMU_LOG_INFO));
	rb_define_const(c_emulator, "EMU_LOG_DEBUG", INT2FIX(EMU_LOG_DEBUG));
	rb_define_method(c_emulator, "test", emulator_test, 1);
	rb_define_method(c_emulator, "cpu_parse", emulator_cpu_parse, 0);
	rb_define_method(c_emulator, "cpu_step", emulator_cpu_step, 0);
	rb_define_method(c_emulator, "cpu_run", emulator_cpu_run, 0);
	rb_define_method(c_emulator, "cpu_eip_get", emulator_cpu_eip_get, 0);
	rb_define_method(c_emulator, "cpu_eip_set", emulator_cpu_eip_set, 1);
	rb_define_method(c_emulator, "cpu_reg32_get", emulator_cpu_reg32_get, 1);
	rb_define_method(c_emulator, "cpu_reg32_set", emulator_cpu_reg32_set, 2);
	rb_define_method(c_emulator, "cpu_reg16_get", emulator_cpu_reg16_get, 1);
	rb_define_method(c_emulator, "cpu_reg16_set", emulator_cpu_reg16_set, 2);
	rb_define_method(c_emulator, "cpu_reg8_get", emulator_cpu_reg8_get, 1);
	rb_define_method(c_emulator, "cpu_reg8_set", emulator_cpu_reg8_set, 2);
	rb_define_method(c_emulator, "cpu_eflags_get", emulator_cpu_eflags_get, 0);
	rb_define_method(c_emulator, "cpu_eflags_set", emulator_cpu_eflags_set, 1);
	rb_define_method(c_emulator, "cpu_debugflag_set", emulator_cpu_debugflag_set, 1);
	rb_define_method(c_emulator, "cpu_debugflag_unset", emulator_cpu_debugflag_unset, 1);
	rb_define_method(c_emulator, "memory_get_usage", emulator_memory_get_usage, 0);
	rb_define_method(c_emulator, "memory_read_byte", emulator_memory_read_byte, 1);
	rb_define_method(c_emulator, "memory_read_word", emulator_memory_read_word, 1);
	rb_define_method(c_emulator, "memory_read_dword", emulator_memory_read_dword, 1);
	rb_define_method(c_emulator, "memory_read_block", emulator_memory_read_block, 2);
	rb_define_method(c_emulator, "memory_write_byte", emulator_memory_write_byte, 2);
	rb_define_method(c_emulator, "memory_write_word", emulator_memory_write_word, 2);
	rb_define_method(c_emulator, "memory_write_dword", emulator_memory_write_dword, 2);
	rb_define_method(c_emulator, "memory_write_block", emulator_memory_write_block, 2);
	rb_define_method(c_emulator, "memory_mode_ro", emulator_memory_mode_ro, 0);
	rb_define_method(c_emulator, "memory_mode_rw", emulator_memory_mode_rw, 0);
	rb_define_method(c_emulator, "memory_segment_select", emulator_memory_segment_select, 1);
	rb_define_method(c_emulator, "memory_segment_get", emulator_memory_segment_get, 0);
	rb_define_method(c_emulator, "log_level_set", emulator_log_level_set, 1);
	
	init_environment();
}
