# Static recompilation of Fire & Ice: the game (gen/: Fire as it decrypts
# itself, ../tools/unpack) and its loader Intro (genintro/), from
# SuperSpider's analyse and gen, and game.c, linked with libroger (the RISC
# OS runtime).
ROGER   ?= $(HOME)/stuff/libroger
CC      ?= gcc
CFLAGS  ?= -O2 -g -w -fno-strict-aliasing $(if $(TRACE),-DRECOMP_TRACE)
GEN     := $(wildcard gen/code*.c) gen/entries.c
GENI    := $(wildcard genintro/code*.c) genintro/entries.c
OBJ     := $(GEN:.c=.o) $(GENI:.c=.o) game.o
LIBS    := $(shell pkg-config --libs sdl3) -ljpeg -lm -lpthread
INC     := -I$(ROGER)/include

fireice: $(OBJ) $(ROGER)/libroger.a
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(ROGER)/libroger.a $(LIBS)

$(ROGER)/libroger.a: FORCE
	$(MAKE) -C $(ROGER) libroger.a

%.o: %.c $(ROGER)/include/roger.h $(ROGER)/include/roger_cpu.h
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

# Code/data analysis: SuperSpider, into ../analysis/<name>.json, then
# superspider gen. Each image's ../analysis/<name>.roots adds entry points
# to the analysis: "make roots" merges the gap entries a --trace-gaps run
# logged (untranslated.log, or LOG=...) into them, so the next "make regen"
# translates them.
SS      ?= $(HOME)/stuff/superspider
SSBIN   := $(SS)/_build/default/bin/main.exe
AN      := ../analysis
APP     := ../game/!Fire_Ice
# Fire decrypted (../tools/unpack stops it at &81C0, where its start-up
# begins once the layers are undone); its OS_GetEnv at &81BC is where it is
# entered (game.c)
GAME    := ../unpacked/fire
ENTRY   := 81bc
INTRO   := $(firstword $(wildcard $(APP)/Intro,*) $(APP)/Intro)
FIRE    := $(firstword $(wildcard $(APP)/Fire,*) $(APP)/Fire)
LOG     ?= untranslated.log

# --roots for ../analysis/<name>.roots, if there is one
roots = $(if $(wildcard $(AN)/$(1).roots),--roots $(AN)/$(1).roots)

# $(call analyse,name,image,base[,options])
define analyse
	$(SSBIN) analyse "$(2)" $(3) $(4) --full $(call roots,$(1)) -o $(AN)/$(1).json
endef

regen:
	cd $(SS) && dune build
	$(MAKE) -C ../tools
	mkdir -p ../unpacked $(AN)
	cd ../tools && ./unpack "$(FIRE)" --headless --stop=81c0 --dump=8000:3af40 --write=$(GAME)
	$(call analyse,fire,$(GAME),8000,--exec $(ENTRY))
	$(call analyse,intro,$(INTRO),88000)
	rm -rf gen genintro
	$(SSBIN) gen "$(GAME)" 8000 $(AN)/fire.json --c gen --leaders=dispatch
	$(SSBIN) gen "$(INTRO)" 88000 $(AN)/intro.json --c genintro --leaders=dispatch \
	    --prefix=I --tabprefix=recompI --files=1 --covtag=I

roots:
	python3 -I $(SS)/tools/gap_roots.py "$(GAME)" 8000 $(AN)/fire.roots $(LOG)
	python3 -I $(SS)/tools/gap_roots.py "$(INTRO)" 88000 $(AN)/intro.roots $(LOG)

clean:
	rm -f gen/*.o genintro/*.o game.o fireice

.PHONY: FORCE regen roots clean
