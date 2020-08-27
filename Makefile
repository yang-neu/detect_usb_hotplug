# specify the compiler
CC=/usr/bin/g++

# specify library

INCFLAGS=-I ./ 


# specify library
LIBFLAGS=-l pthread -l udev

# specify additional compile flags
FLAGS= -lm -g -Wall -Wextra 

# List of files specific 
SRC:= Udev_Monitor.cpp              
 
testapp:
	${CC} -o Udev_Monitor ${SRC} ${LIBFLAGS} ${INCFLAGS}  ${FLAGS}

clean:
	rm -f Udev_Monitor

