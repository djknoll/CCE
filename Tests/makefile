
bootstrap-compile :
	make -C ../Compiler/ all

virtmach-compile : 
	make -C ../VirtualMachine/ all

execute-compile-test : bootstrap-compile
	../Compiler/bin/Compiler.exe test.c test.vm > compile_log.txt

execute-run-test : virtmach-compile
	java -classpath ../VirtualMachine/bin VM test.vm > run_log.txt

all : execute-compile-test execute-run-test
	