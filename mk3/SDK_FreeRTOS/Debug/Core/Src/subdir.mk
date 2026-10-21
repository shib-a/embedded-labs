################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/fonts.c \
../Core/Src/freertos.c \
../Core/Src/game.c \
../Core/Src/gpio.c \
../Core/Src/i2c.c \
../Core/Src/input.c \
../Core/Src/kb.c \
../Core/Src/main.c \
../Core/Src/oled.c \
../Core/Src/pca9538.c \
../Core/Src/stm32f4xx_hal_i2c_ex.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_hal_timebase_tim.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/usart.c 

OBJS += \
./Core/Src/fonts.o \
./Core/Src/freertos.o \
./Core/Src/game.o \
./Core/Src/gpio.o \
./Core/Src/i2c.o \
./Core/Src/input.o \
./Core/Src/kb.o \
./Core/Src/main.o \
./Core/Src/oled.o \
./Core/Src/pca9538.o \
./Core/Src/stm32f4xx_hal_i2c_ex.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_hal_timebase_tim.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/usart.o 

C_DEPS += \
./Core/Src/fonts.d \
./Core/Src/freertos.d \
./Core/Src/game.d \
./Core/Src/gpio.d \
./Core/Src/i2c.d \
./Core/Src/input.d \
./Core/Src/kb.d \
./Core/Src/main.d \
./Core/Src/oled.d \
./Core/Src/pca9538.d \
./Core/Src/stm32f4xx_hal_i2c_ex.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_hal_timebase_tim.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -Og -ffunction-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/fonts.d ./Core/Src/fonts.o ./Core/Src/fonts.su ./Core/Src/freertos.d ./Core/Src/freertos.o ./Core/Src/freertos.su ./Core/Src/game.d ./Core/Src/game.o ./Core/Src/game.su ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/i2c.d ./Core/Src/i2c.o ./Core/Src/i2c.su ./Core/Src/input.d ./Core/Src/input.o ./Core/Src/input.su ./Core/Src/kb.d ./Core/Src/kb.o ./Core/Src/kb.su ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/oled.d ./Core/Src/oled.o ./Core/Src/oled.su ./Core/Src/pca9538.d ./Core/Src/pca9538.o ./Core/Src/pca9538.su ./Core/Src/stm32f4xx_hal_i2c_ex.d ./Core/Src/stm32f4xx_hal_i2c_ex.o ./Core/Src/stm32f4xx_hal_i2c_ex.su ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_hal_timebase_tim.d ./Core/Src/stm32f4xx_hal_timebase_tim.o ./Core/Src/stm32f4xx_hal_timebase_tim.su ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/usart.d ./Core/Src/usart.o ./Core/Src/usart.su

.PHONY: clean-Core-2f-Src

