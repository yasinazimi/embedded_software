################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Project_Settings/Startup_Code/startup.c 

OBJS += \
./Project_Settings/Startup_Code/startup.o 

C_DEPS += \
./Project_Settings/Startup_Code/startup.d 


# Each subdirectory must supply rules for building sources it contributes
Project_Settings/Startup_Code/%.o: ../Project_Settings/Startup_Code/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"D:\UTS\Year 3\Spring Semester\48434 - Embedded Software\SVN\embedded_software\Lab5\Library" -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Lab5/Static_Code/IO_Map" -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Lab5/Sources" -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Lab5/Generated_Code" -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Lab5/Static_Code/PDD" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

