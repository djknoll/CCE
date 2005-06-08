clean :
	mkdir -p bin
	rm -f bin/*.class
	echo Clean done

compile : clean
	javac -classpath src/ src/VM.java -d bin/
	echo compile done

all : compile
	echo All done
