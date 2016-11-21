CFLAGS = -Wall -std=gnu99
NAME = mcts


build: $(OBJECTS)
	gcc $(CFLAGS) mcts.c -o mcts -lm

debug:
	gcc $(CFLAGS) -ggdb mcts.c -o mcts

rebuild:
	make clean
	make build

clean:
	rm -f  *.o
	rm -f *.elf
	rm -f mcts
	rm -f $(NAME)
	rm -rf $(NAME).dSYM
	rm -rf $(NAME).elf.dSYM
	rm -f *.log

run:
	make clean
	make build
	./mcts

default:
	make debug
