clean :
	mkdir -p bin
	rm -f bin/*.exe bin/*.o
	echo Clean done

compile : clean
	gcc -g -o bin/Compiler src/Compiler.c
	echo Compile done

all : compile
	echo All done
