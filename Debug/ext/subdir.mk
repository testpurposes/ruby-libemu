################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../ext/environment.o \
../ext/rlibemu.o 

C_SRCS += \
../ext/environment.c \
../ext/rlibemu.c 

OBJS += \
./ext/environment.o \
./ext/rlibemu.o 

C_DEPS += \
./ext/environment.d \
./ext/rlibemu.d 


# Each subdirectory must supply rules for building sources it contributes
ext/%.o: ../ext/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


