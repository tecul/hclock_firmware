CPPFLAGS+=-D__STM32DEV__=1 -Iboard/stm32/include -Iboard/stm32dev/include
OBJ+=board/stm32dev/startup/crt0.o board/stm32dev/main.o
OBJ+=board/stm32/rcc.o board/stm32/gpio.o board/stm32/timer14.o

TARGET_ARCH+=-mcpu=cortex-m0

LDFLAGS+=-Wl,--gc-sections --specs=nosys.specs --specs=nano.specs -Wl,--script=board/stm32dev/linker_script/stm32dev.ld