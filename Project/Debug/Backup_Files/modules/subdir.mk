################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Backup_Files/modules/arbitrary.c \
../Backup_Files/modules/noise.c \
../Backup_Files/modules/sawtooth.c \
../Backup_Files/modules/sine.c \
../Backup_Files/modules/square.c \
../Backup_Files/modules/triangle.c 

OBJS += \
./Backup_Files/modules/arbitrary.o \
./Backup_Files/modules/noise.o \
./Backup_Files/modules/sawtooth.o \
./Backup_Files/modules/sine.o \
./Backup_Files/modules/square.o \
./Backup_Files/modules/triangle.o 

C_DEPS += \
./Backup_Files/modules/arbitrary.d \
./Backup_Files/modules/noise.d \
./Backup_Files/modules/sawtooth.d \
./Backup_Files/modules/sine.d \
./Backup_Files/modules/square.d \
./Backup_Files/modules/triangle.d 


# Each subdirectory must supply rules for building sources it contributes
Backup_Files/modules/%.o: ../Backup_Files/modules/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"D:\UTS\Year 3\Spring Semester\48434 - Embedded Software\SVN\embedded_software\Project - Yasin\Library" -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Project - Yasin/Static_Code/IO_Map" -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Project - Yasin/Sources" -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Project - Yasin/Generated_Code" -I"D:/UTS/Year 3/Spring Semester/48434 - Embedded Software/SVN/embedded_software/Project - Yasin/Static_Code/PDD" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


