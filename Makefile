CC = gcc
LD = gcc
OPT_LEVEL = 0
DBG_LEVEL = 3
CFLAGS = $(shell sdl-config --cflags)
LDFLAGS = $(shell sdl-config --libs)
OBJDIR = obj
SOURCES = $(wildcard src/*.c)
HEADERS = $(wildcard src/*.h)
OBJS = $(patsubst src/%.c,$(OBJDIR)/%.o,$(SOURCES))
OUT = dungen

default: $(OUT)

$(OBJDIR)/%.o: src/%.c $(HEADERS)
	@mkdir -p $(OBJDIR)
	$(CC) -O$(OPT_LEVEL) -g$(DBG_LEVEL) $(CFLAGS) -c $< -o $@

$(OUT): $(OBJS)
	$(LD) -o $(OUT) $(OBJS) $(LDFLAGS)

clean:
	@rm -rf $(OBJDIR)
	@rm -f $(OUT)
