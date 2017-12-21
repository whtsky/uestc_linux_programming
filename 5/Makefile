cpr: cp.o cp_wrapper.o cpr.c
	gcc -o cpr cp.o cp_wrapper.o cpr.c -lpthread

cp.o: ../2/cp.c ../2/cp.h

cp_wrapper.o: cp_wrapper.c cp_wrapper.h

.PHONY : clean
clean :
	rm *.o cpr