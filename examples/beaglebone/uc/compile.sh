#!/bin/bash

${ARMCC} -mcpu=cortex-a8 -mtune=cortex-a8 -march=armv7-a \
	-c -mlong-calls -fdata-sections -funsigned-char \
	-I ./AM3359/ -I ./EXAMPLES/ -I ./SOFTWARE/ \
	-ffunction-sections -Wall -Dgcc \
	-D SUPPORT_UNALIGNED \
	--specs=rdimon.specs -Wl,--start-group -lgcc -lc -lc -lm -lrdimon -Wl,--end-group -mthumb \
	./AM3359/*.c ./AM3359/*.S ./EXAMPLES/*.c ./SOFTWARE/*.c



${ARMCC} -mcpu=cortex-a8 -mtune=cortex-a8 -march=armv7-a \
	-mlong-calls -fdata-sections -funsigned-char \
	-I ./AM3359/ -I ./EXAMPLES/ -I ./SOFTWARE/ \
	-ffunction-sections -Wall -Dgcc \
	-D SUPPORT_UNALIGNED \
	--specs=rdimon.specs -Wl,--start-group -lgcc -lc -lc -lm -lrdimon -Wl,--end-group -mthumb \
	-o start ./*.o

./clean.sh
