#include "font.h"
#include "font8x8_basic.h"

const unsigned char* FontGetChar(char c) {
	if (c >= 0) {
		return font8x8_basic[(int)c];
	}
	return font8x8_basic[0];
}
