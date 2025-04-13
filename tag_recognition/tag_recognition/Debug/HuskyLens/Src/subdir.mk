################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../HuskyLens/Src/HuskyLens_HAL.c 

OBJS += \
./HuskyLens/Src/HuskyLens_HAL.o 

C_DEPS += \
./HuskyLens/Src/HuskyLens_HAL.d 


# Each subdirectory must supply rules for building sources it contributes
HuskyLens/Src/%.o HuskyLens/Src/%.su HuskyLens/Src/%.cyclo: ../HuskyLens/Src/%.c HuskyLens/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I"D:/Project/SmartPole/Patient-Recognition/tag_recognition/tag_recognition/HuskyLens/Inc" -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-HuskyLens-2f-Src

clean-HuskyLens-2f-Src:
	-$(RM) ./HuskyLens/Src/HuskyLens_HAL.cyclo ./HuskyLens/Src/HuskyLens_HAL.d ./HuskyLens/Src/HuskyLens_HAL.o ./HuskyLens/Src/HuskyLens_HAL.su

.PHONY: clean-HuskyLens-2f-Src

