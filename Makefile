WebPt: Web_Pt.c
	gcc -fopenmp -w -o $@ $<
clean:
	rm WebPt
