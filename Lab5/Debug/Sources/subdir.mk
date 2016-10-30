################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/Events.c \
../Sources/FIFO.c \
../Sources/FTM.c \
../Sources/Flash.c \
../Sources/LEDs.c \
../Sources/PIT.c \
../Sources/RTC.c \
../Sources/SPI.c \
../Sources/UART.c \
../Sources/analog.c \
../Sources/main.c \
../Sources/median.c \
../Sources/packet.c 

OBJS += \
./Sources/Events.o \
./Sources/FIFO.o \
./Sources/FTM.o \
./Sources/Flash.o \
./Sources/LEDs.o \
./Sources/PIT.o \
./Sources/RTC.o \
./Sources/SPI.o \
./Sources/UART.o \
./Sources/analog.o \
./Sources/main.o \
./Sources/median.o \
./Sources/packet.o 

C_DEPS += \
./Sources/Events.d \
./Sources/FIFO.d \
./Sources/FTM.d \
./Sources/Flash.d \
./Sources/LEDs.d \
./Sources/PIT.d \
./Sources/RTC.d \
./Sources/SPI.d \
./Sources/UART.d \
./Sources/analog.d \
./Sources/main.d \
./Sources/median.d \
./Sources/packet.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/%.o: ../Sources/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"D:\UTS\Year 3\Spring Semester\48434 - Embedded Software\SVN\embedded_software\Lab5\Library" -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Lab5/Static_Code/IO_Map" -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Lab5/Sources" -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Lab5/Generated_Code" -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Lab5/Static_Code/PDD" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


