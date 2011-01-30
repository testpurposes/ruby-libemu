#include <ruby.h>

#include <emu/emu.h>
#include <emu/emu_cpu.h>
#include <emu/emu_memory.h>
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

static VALUE m_libemu;
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
static VALUE emulator_memory_read_byte(VALUE, VALUE);
static VALUE emulator_memory_read_word(VALUE, VALUE);
static VALUE emulator_memory_read_dword(VALUE, VALUE);
void Init_rlibemu();

/*
 * call-seq:
 * 	Emulator.memory_read_dword(addr) -> integer
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
 * 	Emulator.memory_read_word(addr) -> integer
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
 * 	Emulator.memory_read_byte(addr) -> integer
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
 * 	Emulator.cpu_eflags_set(value) -> fixnum
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
 * 	Emulator.cpu_eflags_get() -> integer
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
 * 	Emulator.cpu_reg8_set(reg8, value) -> fixnum
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
 * 	Emulator.cpu_reg8_get(reg8) -> integer
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
 * 	Emulator.cpu_reg16_set(reg16, value) -> fixnum
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
 * 	Emulator.cpu_reg16_get(reg16) -> integer
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
 * 	Emulator.cpu_reg32_set(reg32, value) -> fixnum
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
 * 	Emulator.cpu_reg32_get(reg32) -> integer
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
 * 	Emulator.cpu_eip_set(eip) -> fixnum
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
 * 	Emulator.cpu_eip_get() -> integer
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
 * 	Emulator.cpu_parse() -> fixnum
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
 * 	Emulator.cpu_step() -> fixnum
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
 * 	Emulator.cpu_run() -> fixnum
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
 *	Emulator.test(sc) -> fixnum
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
	rb_define_method(c_emulator, "memory_read_byte", emulator_memory_read_byte, 1);
	rb_define_method(c_emulator, "memory_read_word", emulator_memory_read_word, 1);
	rb_define_method(c_emulator, "memory_read_dword", emulator_memory_read_dword, 1);
}
