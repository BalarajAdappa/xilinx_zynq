ffjpeg: ffjpeg.o jfif.o bmp.o bitstr.o color.o dct.o huffman.o quant.o zigzag.o
	cc -o ffjpeg ffjpeg.o jfif.o bmp.o bitstr.o color.o dct.o huffman.o quant.o zigzag.o
ffjpeg.o:ffjpeg.c jfif.h bmp.h
	cc -c ffjpeg.c
jfif.o:jfif.c stdefine.h bitstr.h huffman.h quant.h zigzag.h dct.h bmp.h color.h jfif.h
	cc -c jfif.c
bmp.o: bmp.c stdefine.h bmp.h
	cc -c bmp.c
bitstr.o: bitstr.c bitstr.h
	cc -c bitstr.c
color.o:color.c color.h
	cc -c color.c
dct.o: dct.c dct.h stdefine.h
	cc -c dct.c
huffman.o: huffman.c huffman.h bitstr.h
	cc -c huffman.c
quant.o: quant.c quant.h
	cc -c quant.c
zigzag.o: zigzag.c zigzag.h
	cc -c zigzag.c
clean:
	rm *.o
