main: delete compile link


main-log: delete compile-log link


delete:
	del *.o

compile:
	g++ -c *.cpp

compile-log:
	g++ -c *.cpp -DDO_LOG

link:
	g++ -o test.exe *.o