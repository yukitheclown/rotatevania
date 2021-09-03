CC=gcc

SDL_LDFLAGS = $(shell sdl2-config --libs)
GLEW_LDFLAGS = $(shell pkg-config glew --static --libs)

LUA_LIBS = $(shell pkg-config lua52 --static --libs)
LUA_CFLAGS = $(shell pkg-config lua52 --cflags)

LIBS= -static-libgcc $(SDL_LDFLAGS) $(GLEW_LDFLAGS) $(LUA_LIBS) -lpng -lz -lm

# debugging on, source compiled into exe currently
CFLAGS = -Wall -Wextra -g -pg -O2 $(LUA_CFLAGS)

# LIBS = -Wl,-Bdynamic -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lglew32 -Wl,-Bstatic -lpng16 -lz -lm -Llib/freetype/lib/win_cb/ -lfreetype

SOURCES=window.c main.c memory.c math.c framebuffers.c shaders.c log.c sprite.c image_loader.c bounding_box.c \
quadtree.c object.c world.c text.c level.c hud.c player.c character.c abilities.c spell_shields.c spell_bursts.c \
skeleton_sword_spinner.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=main

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBS)

.c.o:
	$(CC) -c  $< -o $@ $(CFLAGS)


# remember readelf -d main
# then just sudo find / -name 'whaever.so.0'