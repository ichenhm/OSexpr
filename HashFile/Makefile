
run: HashFile.o jtRecord.o
	gcc -o run hashFile.o jtRecord.o
jtRecord.o:jtRecord.c HashFile.h jtRecord.h
	gcc -c jtRecord.c
HashFile.o:HashFile.c HashFile.h
	gcc -c hashFile.c
clean:
	-rm hashFile.o jtRecord.o

