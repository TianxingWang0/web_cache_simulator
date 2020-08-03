TARGET = webcachesim
OBJS += caches/lru_variants.o
OBJS += caches/gd_variants.o
OBJS += caches/cluster_variants.o
OBJS += caches/gqd_cache.o
OBJS += caches/rtt_variants.o
OBJS += caches/cache_object.o
OBJS += random_helper.o
OBJS += consistent_hash/consistent_hash.o
OBJS += consistent_hash/node.o
OBJS += double_queue_node/double_queue_node.o
OBJS += webcachesim.o
LIBS += -lm

CXX = g++ #clang++ #OSX
CXXFLAGS += -std=c++11 #-stdlib=libc++ #non-linux
CXXFLAGS += -MMD -MP # dependency tracking flags
CXXFLAGS += -g
CXXFLAGS += -I./
CXXFLAGS += -Wall -Werror 
LDFLAGS += $(LIBS)
all: CXXFLAGS += -O2 # release flags
all:		$(TARGET)

debug: CXXFLAGS += -ggdb  -D_GLIBCXX_DEBUG # debug flags
debug: $(TARGET)

$(TARGET):	$(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

DEPS = $(OBJS:%.o=%.d)
-include $(DEPS)

clean:
	-rm $(TARGET) $(OBJS) $(DEPS)
