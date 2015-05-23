OBJS = obj/formatting.o obj/replay.o obj/reader.o obj/dispatcher.o obj/serialized/nodes.o obj/serialized/versioned.o
HEADER_FILES = include/sc2/*.hpp
STATLIB = lib/libsc2rep.a
CXXFLAGS = -g -std=c++14 -Wall -static -I/usr/local/include
CXX = g++
# End of configuration options

#What needs to be built to make all files and dependencies
all: makedirs $(STATLIB)

$(STATLIB): $(OBJS)
#Recursively build object files
obj/%.o: source/%.cpp
	$(CXX) $(CXXFLAGS) -I./include  -c -o $@ $<

lib/libsc2rep.a: $(OBJS)
	ar rcs $@ $^

$(OBJS) : $(HEADER_FILES)

makedirs:
	@mkdir -p obj
	@mkdir -p obj/serialized
	@mkdir -p lib

clean:
	@rm -rf obj
	@rm -rf lib
