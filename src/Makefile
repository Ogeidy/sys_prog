all: kpl.out kass.out load.out

kpl.out: komppl.c
	@echo "________k o m p p l         g e n e r a t i o n______"
	gcc -o kpl.out komppl.c
	@echo "_____________________________________________________"
kass.out: kompassr.c
	@echo "________k o m p a s s r     g e n e r a t i o n______"
	gcc -o kass.out kompassr.c
	@echo "_____________________________________________________"
load.out: absloadm.c
	@echo "________a b s l o a d m     g e n e r a t i o n______"
	gcc -std=c99 -o load.out absloadm.c -lncurses 
	@echo "_____________________________________________________"

clean: 
	rm kpl.out kass.out load.out