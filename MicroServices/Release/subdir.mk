################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../referable.cpp \
../clog.cpp \
../mutex.cpp \
../scheduler.cpp \
../scheduler_impl.cpp \
../task_base.cpp \
../spinlock.cpp \
../rwlock.cpp

CPP_DEPS += \
./referable.d \
./clog.d \
./mutex.d \
./scheduler.d \
./scheduler_impl.d \
./task_base.d \
./spinlock.d \
./rwlock.d

OBJS += \
./referable.o \
./clog.o \
./mutex.o \
./scheduler.o \
./scheduler_impl.o \
./task_base.o \
./spinlock.o \
./rwlock.o


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -fPIC -DNDEBUG -I../../include -I../../MicroServices -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


