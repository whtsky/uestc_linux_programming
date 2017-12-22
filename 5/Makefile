cpr: cp_wrapper.o cpr.c cp.o
	gcc -o cpr cp.o cp_wrapper.o cpr.c -lpthread

cp_wrapper.o: cp_wrapper.c cp_wrapper.h cp.h

.PHONY : clean
clean :
	rm *.o cpr