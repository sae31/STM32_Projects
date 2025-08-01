################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/EC200U.c \
../Core/Src/Modem_BLE.c \
../Core/Src/Modem_RxProcess.c \
../Core/Src/app_freertos.c \
../Core/Src/cJSON.c \
../Core/Src/main.c \
../Core/Src/stm32g0xx_hal_msp.c \
../Core/Src/stm32g0xx_hal_timebase_tim.c \
../Core/Src/stm32g0xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32g0xx.c 

OBJS += \
./Core/Src/EC200U.o \
./Core/Src/Modem_BLE.o \
./Core/Src/Modem_RxProcess.o \
./Core/Src/app_freertos.o \
./Core/Src/cJSON.o \
./Core/Src/main.o \
./Core/Src/stm32g0xx_hal_msp.o \
./Core/Src/stm32g0xx_hal_timebase_tim.o \
./Core/Src/stm32g0xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32g0xx.o 

C_DEPS += \
./Core/Src/EC200U.d \
./Core/Src/Modem_BLE.d \
./Core/Src/Modem_RxProcess.d \
./Core/Src/app_freertos.d \
./Core/Src/cJSON.d \
./Core/Src/main.d \
./Core/Src/stm32g0xx_hal_msp.d \
./Core/Src/stm32g0xx_hal_timebase_tim.d \
./Core/Src/stm32g0xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32g0xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G071xx -c -I../Core/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0 -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/EC200U.cyclo ./Core/Src/EC200U.d ./Core/Src/EC200U.o ./Core/Src/EC200U.su ./Core/Src/Modem_BLE.cyclo ./Core/Src/Modem_BLE.d ./Core/Src/Modem_BLE.o ./Core/Src/Modem_BLE.su ./Core/Src/Modem_RxProcess.cyclo ./Core/Src/Modem_RxProcess.d ./Core/Src/Modem_RxProcess.o ./Core/Src/Modem_RxProcess.su ./Core/Src/app_freertos.cyclo ./Core/Src/app_freertos.d ./Core/Src/app_freertos.o ./Core/Src/app_freertos.su ./Core/Src/cJSON.cyclo ./Core/Src/cJSON.d ./Core/Src/cJSON.o ./Core/Src/cJSON.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32g0xx_hal_msp.cyclo ./Core/Src/stm32g0xx_hal_msp.d ./Core/Src/stm32g0xx_hal_msp.o ./Core/Src/stm32g0xx_hal_msp.su ./Core/Src/stm32g0xx_hal_timebase_tim.cyclo ./Core/Src/stm32g0xx_hal_timebase_tim.d ./Core/Src/stm32g0xx_hal_timebase_tim.o ./Core/Src/stm32g0xx_hal_timebase_tim.su ./Core/Src/stm32g0xx_it.cyclo ./Core/Src/stm32g0xx_it.d ./Core/Src/stm32g0xx_it.o ./Core/Src/stm32g0xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32g0xx.cyclo ./Core/Src/system_stm32g0xx.d ./Core/Src/system_stm32g0xx.o ./Core/Src/system_stm32g0xx.su

.PHONY: clean-Core-2f-Src

