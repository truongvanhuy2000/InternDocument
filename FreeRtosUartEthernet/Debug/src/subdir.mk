################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
LD_SRCS += \
../src/lscript.ld 

C_SRCS += \
../src/freertos_hello_world.c \
../src/main.c \
../src/tcpIpController.c \
../src/tcp_perf_client.c \
../src/uartController.c 

OBJS += \
./src/freertos_hello_world.o \
./src/main.o \
./src/tcpIpController.o \
./src/tcp_perf_client.o \
./src/uartController.o 

C_DEPS += \
./src/freertos_hello_world.d \
./src/main.d \
./src/tcpIpController.d \
./src/tcp_perf_client.d \
./src/uartController.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../FreeRtosUartEthernet_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


