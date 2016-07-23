CPPFLAGS+=-D__STM32F0DISCO__=1 -Iboard/stm32/include -Iboard/stm32f0disco/include -Iboard/stm32f0disco/cmsis
OBJ+=board/stm32f0disco/startup/crt0.o board/stm32f0disco/main.o
OBJ+=board/stm32/gpio.o board/stm32/rcc.o board/stm32/timer14.o

TARGET_ARCH+=-mcpu=cortex-m0

LDFLAGS+=-Wl,--gc-sections --specs=nosys.specs --specs=nano.specs -Wl,--script=board/stm32f0disco/linker_script/stm32f0.ld
