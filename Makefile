# Executables
export CC = clang
export LD = $(CC)
export AR = ar
export RM = rm

# Flags to be passed
export CFLAGS   = -std=c99 -pipe -fstack-protector-all
export CPPFLAGS =
export LDFLAGS  =
export RMFLAGS  = -f
export ARFLAGS  = rcs

# Turn on all warnings
# ... except clang, who yells at glib...
ifeq ($(CC), clang)
CFLAGS += -Weverything -Wno-documentation -Wno-documentation-pedantic -Wno-reserved-id-macro -Wno-used-but-marked-unused -Wno-padded
else
CFLAGS += -Wall -Wextra -Wpedantic
endif

# Generate optimized code for release, otherwise for debug
ifdef RELEASE
CFLAGS   += -O2 -flto
CPPFLAGS += -DNDEBUG -D_FORTIFY_SOURCE=2
LDFLAGS  += -Wl,-O1,--sort-common,--as-needed -flto
else
CFLAGS += -g
endif

# Dependencies
LDFLAGS += $(shell pkg-config --libs glib-2.0 gio-2.0)

# Directories
SRCDIR = src

# Name of the target program
NAME = teapot

.PHONY: all clean $(SRCDIR)

all: $(SRCDIR)
	$(LD) $(LDFLAGS) -o $(NAME) $(SRCDIR)/src.a

clean:
	$(MAKE) clean -C $(SRCDIR)
	$(RM) $(RMFLAGS) $(NAME)

$(SRCDIR):
	$(MAKE) -C $@
