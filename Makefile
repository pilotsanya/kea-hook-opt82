KEA_INCLUDE ?= /usr/include/kea
KEA_LIB ?= /usr/lib

OBJECTS = option82.o
MODULES = kea-hook-option82.so
DEPS = $(OBJECTS:.o=.d)
CXXFLAGS = -I $(KEA_INCLUDE) -fPIC -Wno-deprecated -std=c++11
LDFLAGS = -L $(KEA_LIB) -shared -lkea-dhcpsrv -lkea-dhcp++ -lkea-hooks -lkea-log -lkea-util -lkea-exceptions

all: $(OBJECTS) $(MODULES)

kea-hook-option82.so: option82.o
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) option82.o

%.o: %.cc
	$(CXX) -MMD -MP -c $(CXXFLAGS) -o $@ $<

clean:
	rm -f *.o
	rm -f *.d
	rm -f *.so

install:
	mkdir -p $(DESTDIR)
	/usr/bin/install -c -m 644 kea-hook-option82.so $(DESTDIR)/kea-hook-option82.so

-include $(DEPS)
