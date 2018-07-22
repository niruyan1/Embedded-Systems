################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../utilities/fsl_debug_console.c \
../utilities/fsl_io.c \
../utilities/fsl_log.c \
../utilities/fsl_str.c 

OBJS += \
./utilities/fsl_debug_console.o \
./utilities/fsl_io.o \
./utilities/fsl_log.o \
./utilities/fsl_str.o 

C_DEPS += \
./utilities/fsl_debug_console.d \
./utilities/fsl_io.d \
./utilities/fsl_log.d \
./utilities/fsl_str.d 


# Each subdirectory must supply rules for building sources it contributes
utilities/%.o: ../utilities/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DPRINTF_FLOAT_ENABLE=0 -D__USE_CMSIS -DCR_INTEGER_PRINTF -DSDK_DEBUGCONSOLE=0 -D__MCUXPRESSO -DDEBUG -DSDK_OS_BAREMETAL -DFSL_RTOS_BM -DCPU_MKL43Z256VLH4_cm0plus -DCPU_MKL43Z256VLH4 -I"C:\Users\Niruyan\Documents\MCUXpressoIDE_10.1.1_606\workspace\finproj\board" -I"C:\Users\Niruyan\Documents\MCUXpressoIDE_10.1.1_606\workspace\finproj\source" -I"C:\Users\Niruyan\Documents\MCUXpressoIDE_10.1.1_606\workspace\finproj" -I"C:\Users\Niruyan\Documents\MCUXpressoIDE_10.1.1_606\workspace\finproj\drivers" -I"C:\Users\Niruyan\Documents\MCUXpressoIDE_10.1.1_606\workspace\finproj\CMSIS" -I"C:\Users\Niruyan\Documents\MCUXpressoIDE_10.1.1_606\workspace\finproj\utilities" -I"C:\Users\Niruyan\Documents\MCUXpressoIDE_10.1.1_606\workspace\finproj\startup" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


