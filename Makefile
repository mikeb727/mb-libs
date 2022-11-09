GFX_TARGETS=$(addprefix libmbgfx, .so .a)
CHART_TARGETS=$(addprefix libmbchart, .so .a)

CPP=clang++

BIN=bin
INC=include
LIB=lib
SRC=src

.PHONY: all shared static clean

all: mbgfx mbchart

mbgfx: $(addprefix $(LIB)/, $(GFX_TARGETS))
mbchart: $(addprefix $(LIB)/, $(CHART_TARGETS))

clean:
	rm -rf $(BIN) $(LIB)

$(LIB)/$(LIB)%.so: $(BIN)/shared/%.o
	mkdir -p $(LIB)
	$(CPP) -shared $< -o $@

$(LIB)/$(LIB)%.a: $(BIN)/static/%.o
	mkdir -p $(LIB)
	ar rcs $@ $<

$(BIN)/shared/%.o: $(SRC)/%.cpp
	mkdir -p $(dir $@)
	$(CPP) -std=c++17 -I$(INC) -fPIC -c $< -o $@

$(BIN)/static/%.o: $(SRC)/%.cpp
	mkdir -p $(dir $@)
	$(CPP) -std=c++17 -I$(INC) -c $< -o $@