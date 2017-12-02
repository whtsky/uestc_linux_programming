cpr: mycp cpr.c
	gcc -o cpr cpr.c cp.o

cp.o: ../2/cp.c
	gcc -o cp.o ../2/cp.c

.PHONY : clean
clean :
	rm mycp cpr