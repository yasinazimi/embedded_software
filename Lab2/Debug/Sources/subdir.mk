################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/Events.c \
../Sources/FIFO.c \
../Sources/LEDs.c \
../Sources/UART.c \
../Sources/flash.c \
../Sources/main.c \
../Sources/packet.c 

OBJS += \
./Sources/Events.o \
./Sources/FIFO.o \
./Sources/LEDs.o \
./Sources/UART.o \
./Sources/flash.o \
./Sources/main.o \
./Sources/packet.o 

C_DEPS += \
./Sources/Events.d \
./Sources/FIFO.d \
./Sources/LEDs.d \
./Sources/UART.d \
./Sources/flash.d \
./Sources/main.d \
./Sources/packet.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/%.o: ../Sources/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"C:/Users/mac532/Desktop/L2/Lab2/Static_Code/IO_Map" -I"C:/Users/mac532/Desktop/L2/Lab2/Sources" -I"C:/Users/mac532/Desktop/L2/Lab2/Generated_Code" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Sources/main.o: ../Sources/main.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"C:/Users/mac532/Desktop/L2/Lab2/Static_Code/IO_Map" -I"C:/Users/mac532/Desktop/L2/Lab2/Sources" -I"C:/Users/mac532/Desktop/L2/Lab2/Generated_Code" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"Sources/main.d" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


