CC = g++
VPATH = .:./../../
CFLAGS = -Wall -O0 -g -I./ -I../../
LIBS = -lpthread
OUTPUT = main


SRCS := link_list.c queue.c threadpool.cpp main.c debug_print.cpp
OBJS := $(addsuffix .o,$(basename ${SRCS}))

all:${OUTPUT}

%.o:%.cpp
	${CC} -c ${CFLAGS} $< -o $@
${OUTPUT}:${OBJS}
	${CC} -o $@ ${OBJS} ${LIBS}

clean:
	rm -f ${OBJS} ${OUTPUT}