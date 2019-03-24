# Flags to be passed to the C compiler to add additional header searching path
CFLAGS += $(shell pkg-config --cflags glib-2.0 gio-2.0)

# Object files to be compiled (in .o suffix, not .c)
OBJS = server.o app.o main.o

.PHONY: all clean

all: $(OBJS)
	$(AR) $(ARFLAGS) src.a $(OBJS)

clean:
	$(RM) $(RMFLAGS) $(OBJS)
	$(RM) $(RMFLAGS) src.a

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
