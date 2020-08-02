OBJS=dbg.o
SRCS=$(OBJS:%.0=%.c)
CFLAGS=-g -Wall
LDLIBS=
TARGET=debug
$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LDLIBS)
