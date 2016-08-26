################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/FIFO.c \
../Sources/UART.c \
../Sources/cmd.c \
../Sources/main.c \
../Sources/packet.c 

OBJS += \
./Sources/FIFO.o \
./Sources/UART.o \
./Sources/cmd.o \
./Sources/main.o \
./Sources/packet.o 

C_DEPS += \
./Sources/FIFO.d \
./Sources/UART.d \
./Sources/cmd.d \
./Sources/main.d \
./Sources/packet.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/%.o: ../Sources/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Static_Code/PDD" -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Static_Code/IO_Map" -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Sources" -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Generated_Code" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


