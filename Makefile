GFX_TARGETS=$(addprefix libmbgfx, .so .a)
GFX_OBJ=$(addsuffix .o, camera colors gl font light errors mbgfx renderObject scene shader texture window)
GFX_OBJS=$(addprefix $(BIN)/shared/, $(GFX_OBJ))
GFX_OBJS_STATIC=$(addprefix $(BIN)/static/, $(GFX_OBJ))

CHART_TARGETS=$(addprefix libmbchart, .so .a)
CHART_OBJ=$(addsuffix .o, mbchart)
CHART_OBJS=$(addprefix $(BIN)/shared/, $(CHART_OBJ))
CHART_OBJS_STATIC=$(addprefix $(BIN)/static/, $(CHART_OBJ))

BIN=bin
INC=include
LIB=lib
SRC=src
TEST=test

CTEST_IDS=$(addprefix c, 005)
CTESTS=$(addprefix $(TEST)/test, $(CTEST_IDS))
GTEST_IDS=$(addprefix g, 008)
GTESTS=$(addprefix $(TEST)/test, $(GTEST_IDS))

LINK=clang++
TEST_LFLAGS=-Llib -lmbgfx -lGL -lglfw -lfreetype

IFLAGS=$(addprefix -I, include /usr/include/freetype2)
DFLAGS=-g -O0
DFLAGS2=-DCOMPILE_TIME_SHADERS

CXX=clang++
CXXFLAGS=$(TEST_LFLAGS) $(IFLAGS)
C=clang

DESTDIR=

.PHONY: all shared static clean install tests

all: mbgfx mbchart

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

tests: gtests ctests

gtests: $(GTESTS)	
ctests: $(CTESTS)

$(GTESTS): $(BIN)/$(GTESTS).o mbgfx
	echo $(GTESTS)
	mkdir -p $(dir $@)
	$(LINK) -o $@ $(DFLAGS) $(IFLAGS) $(TEST_LFLAGS) $<

$(CTESTS): $(BIN)/$(CTESTS).o mbgfx mbchart
	mkdir -p $(dir $@)
	$(LINK) -o $@ $(DFLAGS) $(IFLAGS) $(TEST_LFLAGS) -lmbchart $<

clean:
	rm -rf ./$(BIN) ./$(LIB) ./$(TEST)

$(LIB)/$(LIB)mbgfx.so: $(GFX_OBJS)
	mkdir -p $(LIB)
	$(CXX) $(DFLAGS) -shared $(GFX_OBJS) -o $@

$(LIB)/$(LIB)mbchart.so: $(CHART_OBJS)
	mkdir -p $(LIB)
	$(CXX) $(DFLAGS) -shared $(CHART_OBJS) -o $@

$(LIB)/$(LIB)mbgfx.a: $(GFX_OBJS_STATIC)
	mkdir -p $(LIB)
	ar rcs $@ $^

$(LIB)/$(LIB)mbchart.a: $(CHART_OBJS_STATIC)
	mkdir -p $(LIB)
	ar rcs $@ $^

$(BIN)/shared/%.o: $(SRC)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) -std=c++17 $(DFLAGS) $(DFLAGS2) $(IFLAGS) -fPIC -c $< -o $@

$(BIN)/shared/%.o: $(SRC)/%.c
	mkdir -p $(dir $@)
	$(C) -std=c17 $(DFLAGS) $(DFLAGS2) $(IFLAGS) -fPIC -c $< -o $@

$(BIN)/static/%.o: $(SRC)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) -std=c++17 $(DFLAGS) $(DFLAGS2) $(IFLAGS) -c $< -o $@

$(BIN)/static/%.o: $(SRC)/%.c
	mkdir -p $(dir $@)
	$(C) -std=c17 $(DFLAGS) $(DFLAGS2) $(IFLAGS) -c $< -o $@

$(BIN)/test/test$(GTEST_IDS).o: $(SRC)/$(TEST)/test$(GTEST_IDS).cpp
	mkdir -p $(dir $@)
	$(CXX) -std=c++17 $(DFLAGS) $(IFLAGS) -c $< -o $@

$(BIN)/test/test$(CTEST_IDS).o: $(SRC)/$(TEST)/test$(CTEST_IDS).cpp
	mkdir -p $(dir $@)
	$(CXX) -std=c++17 $(DFLAGS) $(IFLAGS) -c $< -o $@