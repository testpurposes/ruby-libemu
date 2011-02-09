#include <ruby.h>

#include <emu/environment/emu_env.h>
#include <emu/environment/emu_profile.h>
#include <emu/environment/linux/emu_env_linux.h>
#include <emu/environment/win32/emu_env_w32.h>

extern VALUE m_libemu;
static VALUE c_environment;
void init_environment();
static VALUE environment_initialize(VALUE, VALUE);
static void environment_free(struct emu_env *);
static VALUE environment_allocate(VALUE);

static void environment_free(struct emu_env *env)
{
  emu_env_free(env);
}

/*
 *  call-seq:
 *     Environment.new( emu ) -> env
 *
 *  Returns a new environment object for the emulator emu.
 */
static VALUE environment_initialize(klass, emu)
	VALUE klass;
	VALUE emu;
{
	struct emu *emulator;
	struct emu_env *env;
	
	// Data_Get_Struct check types
	Data_Get_Struct(emu, struct emu, emulator);
	Data_Get_Struct(klass, struct emu_env, env);
	
	memset(env, 0, sizeof(struct emu_env));
	env->env.lin = emu_env_linux_new(emulator);
	env->env.win = emu_env_w32_new(emulator);
	env->emu = emulator;
	env->profile = emu_profile_new();
	
	return klass;
}

static VALUE environment_allocate(klass)
	VALUE klass;
{
	struct emu_env *env;
	VALUE obj = Data_Make_Struct(klass, struct emu_env, NULL, environment_free, env);
	
	env->env.lin = NULL;
	env->env.win = NULL;
	env->emu = NULL;
	env->profile = NULL;
	
	return obj;
}

/*
 * Class Libvirt::Environment
 */
void init_environment() {
	c_environment = rb_define_class_under(m_libemu, 
									"Environment", 
									rb_cObject);
	rb_define_alloc_func(c_environment, environment_allocate);
	rb_define_method(c_environment, "initialize", environment_initialize, 1);
}
