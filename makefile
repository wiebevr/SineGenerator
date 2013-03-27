all: sin_gen

%.rel: %.c %.h
	sdcc -c $<

dds_values.rel:
	sdcc -c dds_values.c

sin_gen: sin_gen.c lcd.rel dds_values.rel
	sdcc sin_gen.c lcd.rel dds_values.rel
	packihx sin_gen.ihx > sin_gen.HEX
