################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../drivers/fsl_clock.c \
../drivers/fsl_common.c \
../drivers/fsl_flash.c \
../drivers/fsl_gpio.c \
../drivers/fsl_lpuart.c \
../drivers/fsl_smc.c \
../drivers/fsl_uart.c 

OBJS += \
./drivers/fsl_clock.o \
./drivers/fsl_common.o \
./drivers/fsl_flash.o \
./drivers/fsl_gpio.o \
./drivers/fsl_lpuart.o \
./drivers/fsl_smc.o \
./drivers/fsl_uart.o 

C_DEPS += \
./drivers/fsl_clock.d \
./drivers/fsl_common.d \
./drivers/fsl_flash.d \
./drivers/fsl_gpio.d \
./drivers/fsl_lpuart.d \
./drivers/fsl_smc.d \
./drivers/fsl_uart.d 


# Each subdirectory must supply rules for building sources it contributes
drivers/%.o: ../drivers/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DPRINTF_FLOAT_ENABLE=0 -D__USE_CMSIS -DCR_INTEGER_PRINTF -DSDK_DEBUGCONSOLE=0 -D__MCUXPRESSO -DDEBUG -DSDK_OS_BAREMETAL -DFSL_RTOS_BM -DCPU_MKL43Z256VLH4_cm0plus -DCPU_MKL43Z256VLH4 -I"C:\Users\Niruyan\Documents\MCUXpressoIDE_10.1.1_606\workspace\finproj\board" -I"C:\Users\Niruyan\Documents\MCUXpressoIDE_10.1.1_606\workspace\finproj\source" -I"C:\Users\Niruyan\Documents\MCUXpressoIDE_10.1.1_606\workspace\finproj" -I"C:\Users\Niruyan\Documents\MCUXpressoIDE_10.1.1_606\workspace\finproj\drivers" -I"C:\Users\Niruyan\Documents\MCUXpressoIDE_10.1.1_606\workspace\finproj\CMSIS" -I"C:\Users\Niruyan\Documents\MCUXpressoIDE_10.1.1_606\workspace\finproj\utilities" -I"C:\Users\Niruyan\Documents\MCUXpressoIDE_10.1.1_606\workspace\finproj\startup" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


