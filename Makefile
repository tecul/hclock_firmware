CC=arm-none-eabi-g++
CXX=arm-none-eabi-g++

CFLAGS=-Os -g -Wall -fno-rtti -fno-exceptions
CXXFLAGS=${CFLAGS}
CPPFLAGS=-Iinclude

include app/Makefile
include cpu/Makefile
include board/${BOARD}/Makefile
include scheduler/${SCHEDULER}/Makefile

${APP}: ${OBJ}
	${CC} ${OBJ} -o ${APP} ${LDFLAGS}

clean:
	rm -f ${OBJ} ${APP}
