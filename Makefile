GFX_TARGETS=$(addprefix libmbgfx, .so .a)
CHART_TARGETS=$(addprefix libmbchart, .so .a)

CPP=clang++

BIN=bin
INC=$(addprefix -I, include /usr/include/freetype2)
LIB=lib
SRC=src

#DEBUG_FLAGS=-g -O0

DESTDIR=

.PHONY: all shared static clean install

all: mbgfx mbchart

install:
	mkdir -p $(DESTDIR)/usr/lib/mb-libs/
	install -m 755 $(LIB)/* $(DESTDIR)/usr/lib/mb-libs/
	mkdir -p $(DESTDIR)/usr/include/mb-libs/
	install -m 755 $(INC)/* $(DESTDIR)/usr/include/mb-libs/

mbgfx: $(addprefix $(LIB)/, $(GFX_TARGETS))
mbchart: $(addprefix $(LIB)/, $(CHART_TARGETS))

clean:
	rm -rf ./$(BIN) ./$(LIB)

$(LIB)/$(LIB)%.so: $(BIN)/shared/%.o
	mkdir -p $(LIB)
	$(CPP) -shared $< -o $@

$(LIB)/$(LIB)%.a: $(BIN)/static/%.o
	mkdir -p $(LIB)
	ar rcs $@ $<

$(BIN)/shared/%.o: $(SRC)/%.cpp
	mkdir -p $(dir $@)
	$(CPP) -std=c++17 $(DEBUG_FLAGS) $(INC) -fPIC -c $< -o $@

$(BIN)/static/%.o: $(SRC)/%.cpp
	mkdir -p $(dir $@)
	$(CPP) -std=c++17 $(DEBUG_FLAGS) $(INC) -c $< -o $@
