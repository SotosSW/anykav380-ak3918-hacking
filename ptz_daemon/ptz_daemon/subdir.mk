################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../ptz_daemon_cpp.cpp 

C_SRCS += \
../mqtt.c 

CPP_DEPS += \
./ptz_daemon_cpp.d 

C_DEPS += \
./mqtt.d 

OBJS += \
./mqtt.o \
./ptz_daemon_cpp.o 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	/opt/arm-anykav200-crosstool/usr/bin/arm-anykav200-linux-uclibcgnueabi-gcc -I"/home/animus/anyka/ptz/include" -I/usr/local/include -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.cpp subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	/opt/arm-anykav200-crosstool/usr/bin/arm-anykav200-linux-uclibcgnueabi-g++ -std=c++0x -I"/home/animus/anyka/ptz/include" -I/usr/local/include -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean--2e-

clean--2e-:
	-$(RM) ./mqtt.d ./mqtt.o ./ptz_daemon_cpp.d ./ptz_daemon_cpp.o

.PHONY: clean--2e-

