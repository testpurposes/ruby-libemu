#include <ruby.h>

#include <emu/emu.h>
#include <emu/emu_shellcode.h>

static VALUE m_libemu;
static VALUE c_emulator;
static void emulator_mark(struct emu *);
static void emulator_free (struct emu *);
void Init_rlibemu();

/*
 * call-seq:
 *	Emulator.test(sc) -> fixnum
 * 
 * Tests a given buffer for possible shellcodes. On success, returns 
 * the offset within the buffer where the shellcode is suspected. On
 * failure (no shellcode detected), returns -1
 *
 */
static VALUE emulator_test(VALUE klass, VALUE sc) {
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

static void emulator_mark(struct emu *emulator) {}

static void emulator_free (struct emu *emulator)
{
  emu_free(emulator);
}

static VALUE emulator_allocate(VALUE klass)
{
  struct emu *emulator = emu_new();

  return Data_Wrap_Struct(klass, emulator_mark, emulator_free, emulator);
}

void Init_rlibemu() {
	m_libemu = rb_define_module("Libemu");
	/*
	 * Class Libemu::Emulator
	 */
	c_emulator = rb_define_class_under(m_libemu, 
										"Emulator", 
										rb_cObject);
	rb_define_alloc_func(c_emulator, emulator_allocate);
	rb_define_method(c_emulator, "test", emulator_test, 1);
}
