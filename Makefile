# Makefile for modq
# 080822 suehara, 140424 modified

# NOTE: .c & .h file not permitted for this Makefile.
# please rename them to .cc & .hh (even if it doesn't use any c++ features).

# common source files for all projects
BASE_SRCS = $(wildcard src/base/*.cc)
BASE_HEADS = $(wildcard include/modq/*.hh)

# target files
#TARGETS1 = $(PROJECT_SRCS:.cc=)
#TARGETS = $(subst src,bin,$(TARGETS1))
TARGETS = bin/modq

# object files
BASE_OBJS1 = $(subst src/base,obj,$(BASE_SRCS))
BASE_OBJS = $(BASE_OBJS1:.cc=.o)
#PROJECT_OBJS1 = $(subst src,obj,$(PROJECT_SRCS))
#PROJECT_OBJS = $(PROJECT_OBJS1:.cc=.o)
#OBJS = $(PROJECT_OBJS) $(COMMON_OBJS) $(PROJECT_NOVME_OBJS)
#SRCS = $(PROJECT_SRCS) $(COMMON_SRCS) $(PROJECT_NOVME_SRCS)
#LIBNAME = obj/libsuedaq.a
OBJS = $(BASE_OBJS)
SRCS = $(BASE_SRCS)
HEADS = $(BASE_HEADS)

# dependency search path
vpath %.o ../obj
vpath % bin
vpath %.cc src src/base ../src ../src/base

# flags
LD = gcc
#CPPFLAGS = -O0 -g -Wall -Wno-deprecated -Isrc/common -I/usr/local/root5.26.00/include/root
CPPFLAGS = -g -Wall -Wno-deprecated -Iinclude
#LDLIBS := -lstdc++ -lpthread -Lobj -lsuedaq -lnidaqmxbase $(shell root-config --libs)
LDLIBS := -lstdc++ -lpthread

# suffix definition
.SUFFIXES:
.SUFFIXES: .bin .o .c .cpp .cc .h .hpp .hh

# .PHONY (fake target)
.PHONY: all clean dep commit

# default target
all: $(BASE_OBJS) $(TARGETS)

# dependency file update (not automatic)
dep:
	-@rm depend.mk
	@g++ -MM $(CPPFLAGS) $(BASE_SRCS) > depend.mk1
	@sed "s/^\([0-9A-Za-z]\)/obj\/\1/" depend.mk1 > depend.mk
	@rm depend.mk1

# include dep file
-include depend.mk

# clean
clean:
	rm -f $(TARGETS)
	rm -f core
	rm -f $(OBJS)

# rule for executable
$(TARGETS): bin/% : $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)
# rule for object files
$(BASE_OBJS) : obj/%.o : src/base/%.cc Makefile
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

# suffixes
.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
.cc.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
.cpp.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
