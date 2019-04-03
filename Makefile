WebPt: WebPt.c
	gcc -fopenmp -w -o $@ $<
clean:
	rm WebPt
