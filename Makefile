LFLAGS=-lpulse-simple -lpulse -lfftw3 -lm
CFLAGS=-D_REENTRANT -std=c11 -D _DEFAULT_SOURCE -O3

main: src/main.c
	gcc src/main.c $(CFLAGS) $(LFLAGS) -o main

run_and_plot: main
	echo "#running main"
	./main >freqs
	echo "#running gnuplot"
	gnuplot -persist plot.plt

clean: 
	rm main

