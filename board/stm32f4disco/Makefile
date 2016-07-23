CPPFLAGS+=-D__STM32F4DISCO__ -Iboard/stm32/include -Iboard/stm32f4disco/include -Iboard/stm32f4disco/cmsis
OBJ+=board/stm32f4disco/cmsis/system_stm32f4xx.o board/stm32f4disco/startup/crt0.o board/stm32f4disco/main.o \
board/stm32/gpio.o board/stm32/rcc.o board/stm32/i2c_soft.o \
board/stm32f4disco/input/stmpe811.o board/stm32f4disco/display/fonts.o

#OBJ+=board/stm32f4disco/display/ili9341_gpio.o
OBJ+=board/stm32f4disco/display/ili9341_lcd.o

TARGET_ARCH+=-mcpu=cortex-m4
#LDFLAGS+=-Wl,--gc-sections --specs=rdimon.specs -Wl,--script=board/stm32f4disco/linker_script/stm32f4.ld
LDFLAGS+=-Wl,--gc-sections --specs=nosys.specs --specs=nano.specs -Wl,--script=board/stm32f4disco/linker_script/stm32f4.ld