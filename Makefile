GPLUS = g++ -std=c++11 -Wall -pedantic -g
OBJECTS = mshmain.o parser.o scanner.o global.o
HEADERS = global.h


MYSHELL: $(OBJECTS)
	$(GPLUS) $(OBJECTS) -o MYSHELL
%.o: %.cpp $(HEADERS)
	$(GPLUS) -c $< -o $@
clean:
	rm -f $(OBJECTS) MYSHELL
