JAVA=/usr/bin/java
OUTPUT=output
GENERATED=parser
GRAMMAR=SimpleBoolean.g4

# here is where you plug in the runtime for your OS
CC=g++
CCARGS=-c -I antlr4-runtime/ -I $(GENERATED) -std=c++11	
LDARGS=-g
LIBS=dist/libantlr4-runtime.a

all: test

test: dirs antlr4 test.cpp
	$(CC) $(CCARGS) test.cpp  -o $(OUTPUT)/test.o 
	$(CC) $(CCARGS) $(GENERATED)/SimpleBooleanBaseListener.cpp -o $(OUTPUT)/SimpleBooleanBaseListener.o 
	$(CC) $(CCARGS) $(GENERATED)/SimpleBooleanLexer.cpp -o $(OUTPUT)/SimpleBooleanLexer.o 
	$(CC) $(CCARGS) $(GENERATED)/SimpleBooleanListener.cpp -o $(OUTPUT)/SimpleBooleanListener.o 
	$(CC) $(CCARGS) $(GENERATED)/SimpleBooleanParser.cpp -o $(OUTPUT)/SimpleBooleanParser.o 
	$(CC) $(LDARGS) $(OUTPUT)/test.o $(OUTPUT)/SimpleBooleanBaseListener.o $(OUTPUT)/SimpleBooleanLexer.o $(OUTPUT)/SimpleBooleanListener.o $(OUTPUT)/SimpleBooleanParser.o $(LIBS) -o test

antlr4: $(GRAMMAR)
	$(JAVA) -jar antlr-4.8-complete.jar -Dlanguage=Cpp -o $(GENERATED) $(GRAMMAR)

dirs:
	mkdir -p $(OUTPUT) 
	mkdir -p $(GENERATED) 

clean:
	rm -rf $(OUTPUT)
	rm -rf $(GENERATED)
