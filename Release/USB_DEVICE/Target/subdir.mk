################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB_DEVICE/Target/usbd_conf.c 

OBJS += \
./USB_DEVICE/Target/usbd_conf.o 

C_DEPS += \
./USB_DEVICE/Target/usbd_conf.d 


# Each subdirectory must supply rules for building sources it contributes
USB_DEVICE/Target/%.o USB_DEVICE/Target/%.su USB_DEVICE/Target/%.cyclo: ../USB_DEVICE/Target/%.c USB_DEVICE/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -DUSE_HAL_DRIVER -DSTM32L072xx -DARM_MATH_CM0 -c -I../Core/Inc -I"C:/Users/itsme/OneDrive/Documenten/GitHub/MJSGadget - dB meter/Drivers/CMSIS/DSP/Include" -I../USB_DEVICE/App -I../USB_DEVICE/Target -IC:/Users/itsme/STM32Cube/Repository/STM32Cube_FW_L0_V1.12.2/Drivers/STM32L0xx_HAL_Driver/Inc -IC:/Users/itsme/STM32Cube/Repository/STM32Cube_FW_L0_V1.12.2/Drivers/STM32L0xx_HAL_Driver/Inc/Legacy -IC:/Users/itsme/STM32Cube/Repository/STM32Cube_FW_L0_V1.12.2/Middlewares/ST/STM32_USB_Device_Library/Core/Inc -IC:/Users/itsme/STM32Cube/Repository/STM32Cube_FW_L0_V1.12.2/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -IC:/Users/itsme/STM32Cube/Repository/STM32Cube_FW_L0_V1.12.2/Drivers/CMSIS/Device/ST/STM32L0xx/Include -IC:/Users/itsme/STM32Cube/Repository/STM32Cube_FW_L0_V1.12.2/Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-USB_DEVICE-2f-Target

clean-USB_DEVICE-2f-Target:
	-$(RM) ./USB_DEVICE/Target/usbd_conf.cyclo ./USB_DEVICE/Target/usbd_conf.d ./USB_DEVICE/Target/usbd_conf.o ./USB_DEVICE/Target/usbd_conf.su

.PHONY: clean-USB_DEVICE-2f-Target

