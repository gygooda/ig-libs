XX       = g++
AR       = ar rcs 
NOATIME  = 
CFLAGS   = -DCONFIG_SMP -DNDEBUG -std=c++0x -Wall -Wno-non-virtual-dtor -ggdb
LIBS     = -ldl -lrt -pthread
TARGET   = libiglib-sys.a 
INCLUDES = -I.
SOURCES  = $(wildcard *.cpp)
OBJS     = $(patsubst %.cpp,%.o,$(SOURCES)) 
SUBOBJS  = 

.PHONY: clean 

all: $(TARGET)

$(TARGET) : $(OBJS) $(SUBOBJS)
	$(AR) $(TARGET) $(OBJS) $(SUBOBJS)

test: $(OBJS)
	$(XX) $(OBJS) -o $@ $(LIBS) 
	
%.o: %.cpp
	$(XX) -c $< -o $@ $(CFLAGS) $(INCLUDES) $(LIBS)
	
$(SUBOBJS):
	@make_all_sub_dir()\
	{\
		for dir in `ls`; do \
			if [ -d $$dir ]; then \
				cd $$dir;\
				make_all_sub_dir;\
				if [ -f makefile -o -f Makefile ]; then\
					make;\
				fi;\
				cd ..;\
			fi;\
		done;\
	};\
	make_all_sub_dir		

clean:
	rm -rf *.o $(TARGET) test 
	@make_clean_sub_dir()\
	{\
		for dir in `ls`; do \
			if [ -d $$dir ]; then \
				cd $$dir;\
				make_clean_sub_dir;\
				if [ -f makefile -o -f Makefile ]; then\
					make clean;\
				fi;\
				cd ..;\
			fi;\
		done;\
	};\
	make_clean_sub_dir
	
rebuild: clean all	
	
c: clean

r: rebuild
