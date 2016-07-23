CPPFLAGS+=-D__HCLOCK__=1 -Iboard/stm32/include -Iboard/hclock/include
OBJ+=board/hclock/startup/crt0.o board/hclock/main.o
OBJ+=board/stm32/rcc.o board/stm32/gpio.o board/stm32/timer14.o

TARGET_ARCH+=-mcpu=cortex-m0 -mthumb

LDFLAGS+=-Wl,--gc-sections --specs=nosys.specs --specs=nano.specs -Wl,--script=board/hclock/linker_script/hclock.ld