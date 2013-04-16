GPP = g++
GCC = gcc

COMPILE_FLAGS = -c -m32 -O3 -fPIC -w -DLINUX -Wall -I src/SDK/amx/

ifdef STATIC
OUTFILE = "bin/mysql_static.so"
COMPILE_FLAGS_2 = ./lib/libmysqlclient.a
else
OUTFILE = "bin/mysql.so"
COMPILE_FLAGS_2 = -L/usr/lib/mysql -lmysqlclient_r
endif

all:
	$(GPP) $(COMPILE_FLAGS) src/SDK/*.cpp
	$(GPP) $(COMPILE_FLAGS) src/*.cpp
	$(GPP) -fshort-wchar -shared -o $(OUTFILE) *.o $(COMPILE_FLAGS_2) -lpthread
	rm -f *.o