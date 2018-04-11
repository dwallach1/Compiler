CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
CC_FLAGS  := --std=c++11 -I./src -I../lib/PEGTL -I../lib -g3
LD_FLAGS  := 
CC        := g++

all: dirs LA

dirs: obj bin

obj:
	mkdir -p $@

bin:
	mkdir -p $@

LA: $(OBJ_FILES)
	$(CC) $(LD_FLAGS) -o ./bin/$@ $^

obj/%.o: src/%.cpp
	$(CC) $(CC_FLAGS) -c -o $@ $<

oracle: LA
	../scripts/generateOutput.sh a LAc

rm_tests_without_oracle:
	../scripts/rm_tests_without_oracle.sh a

test: dirs LA
	../scripts/test.sh a LAc

clean:
	rm -fr bin obj *.out *.IR *.o *.S core.* tests/liveness/*.tmp tests/*.tmp
