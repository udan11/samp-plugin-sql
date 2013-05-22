GPP = g++
GCC = gcc

COMPILE_FLAGS = -c -m32 -O3 -fPIC -w -DLINUX -Wall -I src/sdk/amx/

ifdef STATIC
OUTFILE = "bin/sql_static.so"
COMPILE_FLAGS_2 = ./lib/mysql/libmysqlclient.a ./lib/pgsql/libpq.a
else
OUTFILE = "bin/sql.so"
COMPILE_FLAGS_2 = -L/usr/lib/mysql -lmysqlclient_r -lpq
endif

all:
	$(GPP) $(COMPILE_FLAGS) src/sdk/*.cpp
	$(GPP) $(COMPILE_FLAGS) src/sql/*.cpp
	$(GPP) $(COMPILE_FLAGS) src/mysql/*.cpp
	$(GPP) $(COMPILE_FLAGS) src/pgsql/*.cpp
	$(GPP) $(COMPILE_FLAGS) src/*.cpp
	$(GPP) -fshort-wchar -shared -o $(OUTFILE) *.o $(COMPILE_FLAGS_2) -lpthread
	rm -f *.o