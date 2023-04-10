all: myshell run

myshell: myShell.cpp
	g++ -o myshell myShell.cpp

.PHONY: run clean

clean:
	rm -f myshell

run:
	./myshell
	
