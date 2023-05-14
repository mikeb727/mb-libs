GFX_TARGETS=$(addprefix libmbgfx, .so)
GFX_OBJ=$(addsuffix .o, camera colors gl errors mbgfx renderObject scene shader texture window)
GFX_OBJS=$(addprefix $(BIN)/shared/, $(GFX_OBJ))

CHART_TARGETS=$(addprefix libmbchart, .so .a)

CPP=clang++
C=clang

BIN=bin
INC=include
LIB=lib
SRC=src
TEST=test

LINK=clang++
TEST_LFLAGS=-Llib -lmbgfx -lGL -lglfw -lfreetype

IFLAGS=$(addprefix -I, include /usr/include/freetype2)
DFLAGS=-g -O0

DESTDIR=

.PHONY: all shared static clean install

all: mbgfx
tests: $(TEST)/test1 $(TEST)/test2 $(TEST)/test3 $(TEST)/test4

install:
	mkdir -p $(DESTDIR)/usr/lib/mb-libs/
	install -m 755 $(LIB)/libmb* $(DESTDIR)/usr/lib/mb-libs/
	mkdir -p $(DESTDIR)/usr/include/mb-libs/
	install -m 755 $(INC)/*.h $(DESTDIR)/usr/include/mb-libs/
	mkdir -p $(DESTDIR)/usr/include/mb-libs/glad
	install -m 755 $(INC)/glad/*.h $(DESTDIR)/usr/include/mb-libs/glad/
	mkdir -p $(DESTDIR)/usr/include/mb-libs/KHR
	install -m 755 $(INC)/KHR/*.h $(DESTDIR)/usr/include/mb-libs/KHR/

mbgfx: $(addprefix $(LIB)/, $(GFX_TARGETS))
mbchart: $(addprefix $(LIB)/, $(CHART_TARGETS))

$(TEST)/test1: $(BIN)/test/test1.o mbgfx
	$(LINK) -o $@ $(DFLAGS) $(TEST_LFLAGS) $<

$(TEST)/test2: $(BIN)/test/test2.o mbgfx
	$(LINK) -o $@ $(DFLAGS) $(TEST_LFLAGS) $<

$(TEST)/test3: $(BIN)/test/test3.o mbgfx
	$(LINK) -o $@ $(DFLAGS) $(TEST_LFLAGS) $<

$(TEST)/test4: $(BIN)/test/test4.o mbgfx
	$(LINK) -o $@ $(DFLAGS) $(TEST_LFLAGS) $<

clean:
	rm -rf ./$(BIN) ./$(LIB)

$(LIB)/$(LIB)mbgfx.so: $(GFX_OBJS)
	mkdir -p $(LIB)
	$(CPP) $(DFLAGS) -shared $(GFX_OBJS) -o $@

$(LIB)/$(LIB)%.a: $(BIN)/static/%.o
	mkdir -p $(LIB)
	ar rcs $@ $<

$(BIN)/shared/%.o: $(SRC)/%.cpp
	mkdir -p $(dir $@)
	$(CPP) -std=c++17 $(DFLAGS) $(IFLAGS) -fPIC -c $< -o $@

$(BIN)/shared/%.o: $(SRC)/%.c
	mkdir -p $(dir $@)
	$(C) -std=c17 $(DFLAGS) $(IFLAGS) -fPIC -c $< -o $@

$(BIN)/static/%.o: $(SRC)/%.cpp
	mkdir -p $(dir $@)
	$(CPP) -std=c++17 $(DFLAGS) $(IFLAGS) -c $< -o $@

$(BIN)/test/%.o: $(TEST)/%.cpp
	mkdir -p $(dir $@)
	$(CPP) -std=c++17 $(DFLAGS) $(IFLAGS) -c $< -o $@