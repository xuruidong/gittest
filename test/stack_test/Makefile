CC = gcc
VPATH = .:./../../
CFLAGS = -Wall -O0 -g -I./ -I../../
OUTPUT = main


SRCS := link_list.c stack.c main.c
OBJS := $(addsuffix .o,$(basename ${SRCS}))

all:${OUTPUT}

%.o:%.cpp
	${CC} -c ${CFLAGS} $< -o $@
${OUTPUT}:${OBJS}
	${CC} -o $@ ${OBJS} ${LIBS}

clean:
	rm -f ${OBJS} ${OUTPUT}