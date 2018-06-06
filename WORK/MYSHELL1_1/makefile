OBJECTS = mshshell.o
MYSHELL: $(OBJECTS)
	g++ -Wall -pedantic -o MYSHELL $^
%.o: %.cpp $(OBJECTS)
	g++ -std=c++11 -g -c $< -o $@
clean:
	rm -f *.o MYSHELL
