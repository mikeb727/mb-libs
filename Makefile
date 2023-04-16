GFX_TARGETS=$(addprefix libmbgfx, .so)
GFX_OBJ=$(addsuffix .o, camera gl errors mbgfx renderObject scene shader texture)
GFX_OBJS=$(addprefix $(BIN)/shared/, $(GFX_OBJ))

CHART_TARGETS=$(addprefix libmbchart, .so .a)

CPP=clang++
C=clang

BIN=bin
INC=$(addprefix -I, include /usr/include/freetype2)
LIB=lib
SRC=src
TEST=test

LINK=clang++
TEST_LFLAGS=-Llib -lmbgfx -lGL -lglfw -lfreetype

DEBUG_FLAGS=-g -O0

DESTDIR=

.PHONY: all shared static clean install

all: mbgfx
tests: $(TEST)/test1 $(TEST)/test2

install:
	mkdir -p $(DESTDIR)/usr/lib/mb-libs/
	install -m 755 $(LIB)/* $(DESTDIR)/usr/lib/mb-libs/
	mkdir -p $(DESTDIR)/usr/include/mb-libs/
	install -m 755 $(INC)/* $(DESTDIR)/usr/include/mb-libs/

mbgfx: $(addprefix $(LIB)/, $(GFX_TARGETS))
mbchart: $(addprefix $(LIB)/, $(CHART_TARGETS))

$(TEST)/test1: $(BIN)/test/test1.o mbgfx
	$(LINK) -o $@ $(DEBUG_FLAGS) $(TEST_LFLAGS) $<

$(TEST)/test2: $(BIN)/test/test2.o mbgfx
	$(LINK) -o $@ $(DEBUG_FLAGS) $(TEST_LFLAGS) $<

clean:
	rm -rf ./$(BIN) ./$(LIB)

$(LIB)/$(LIB)mbgfx.so: $(GFX_OBJS)
	mkdir -p $(LIB)
	$(CPP) -shared $(GFX_OBJS) -o $@

$(LIB)/$(LIB)%.a: $(BIN)/static/%.o
	mkdir -p $(LIB)
	ar rcs $@ $<

$(BIN)/shared/%.o: $(SRC)/%.cpp
	mkdir -p $(dir $@)
	$(CPP) -std=c++17 $(DEBUG_FLAGS) $(INC) -fPIC -c $< -o $@

$(BIN)/shared/%.o: $(SRC)/%.c
	mkdir -p $(dir $@)
	$(C) -std=c17 $(DEBUG_FLAGS) $(INC) -fPIC -c $< -o $@

$(BIN)/static/%.o: $(SRC)/%.cpp
	mkdir -p $(dir $@)
	$(CPP) -std=c++17 $(DEBUG_FLAGS) $(INC) -c $< -o $@

$(BIN)/test/%.o: $(TEST)/%.cpp
	mkdir -p $(dir $@)
	$(CPP) -std=c++17 $(DEBUG_FLAGS) $(INC) -c $< -o $@