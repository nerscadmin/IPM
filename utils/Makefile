
IPM_ROOT = ../

MXML_INCDIR = ./mxml/include
MXML_LIBDIR = ./mxml/lib

LIB_MXML=-L$(MXML_LIBDIR) -lmxml -lpthread
#LIB_CUBE=-L./cubew/lib -lcubew3
LIB_CUBE=cubew/lib/libcubew3.a 

CFLAGS = -O2 -I./cubew/lib -I$(IPM_ROOT)/include -I$(MXML_INCDIR)

#include $(IPM_ROOT)/make_config

all: mxml ipm_parse

ipm_parse : ipm_parse.o getopts.o read_ipm.o write_cube.o util.o \
	write_html.o report_banner.o $(LIB_CUBE)
	$(CXX) -o $@ $+ $(LIB_MXML) $(LIB_CUBE)

mxml : 
	./make_mxml

%.o : %.cc 
	$(CXX) -c $(CFLAGS)  $+ -o $@

report_banner.o : $(IPM_ROOT)/src/report_banner.c
	$(CXX) $(CFLAGS) -c $+ -o $@

$(LIB_CUBE) : 
	make -C ./cubew/lib

clean : 
	rm -rf ipm_parse mxml
	rm -rf *.o *~
