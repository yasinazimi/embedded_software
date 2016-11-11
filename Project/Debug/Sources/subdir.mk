################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/AWG.c \
../Sources/Events.c \
../Sources/FIFO.c \
../Sources/LEDs.c \
../Sources/PIT.c \
../Sources/SPI.c \
../Sources/UART.c \
../Sources/analog.c \
../Sources/main.c \
../Sources/packet.c \
../Sources/waveform.c 

OBJS += \
./Sources/AWG.o \
./Sources/Events.o \
./Sources/FIFO.o \
./Sources/LEDs.o \
./Sources/PIT.o \
./Sources/SPI.o \
./Sources/UART.o \
./Sources/analog.o \
./Sources/main.o \
./Sources/packet.o \
./Sources/waveform.o 

C_DEPS += \
./Sources/AWG.d \
./Sources/Events.d \
./Sources/FIFO.d \
./Sources/LEDs.d \
./Sources/PIT.d \
./Sources/SPI.d \
./Sources/UART.d \
./Sources/analog.d \
./Sources/main.d \
./Sources/packet.d \
./Sources/waveform.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/%.o: ../Sources/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"D:\UTS\Year 3\Spring Semester\48434 - Embedded Software\SVN\embedded_software\Project - Yasin\Library" -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Project - Yasin/Static_Code/IO_Map" -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Project - Yasin/Sources" -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Project - Yasin/Generated_Code" -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Project - Yasin/Static_Code/PDD" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


