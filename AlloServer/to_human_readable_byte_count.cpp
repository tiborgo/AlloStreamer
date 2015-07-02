#include <math.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>

#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "to_human_readable_byte_count.hpp"

// Adapted from http://agentzlerich.blogspot.com/2011/01/converting-to-and-from-human-readable.html
// how-to-convert-byte-size-into-human-readable-format-in-java
std::string to_human_readable_byte_count(unsigned long bs, bool bit, bool si)
{
    static const char* unit[2] = { "Byte", "Bit" };
    static const char* siSuffix[2] = { "", "i" };
    static const char* factor[] = { "", "k", "M", "G", "T", "E", "Z", "Y" };
    
    int base = si ? 1024 : 1000;
    
	int exp = 0;
	if (bs > 0)
	{
        exp = std::min((int)(log(bs) / log(base)),
			(int) (sizeof(factor) / sizeof(factor[0]) - 1));
	}
    
    double coeff = (double)bs / pow(base, exp);

    std::stringstream result;
    result << std::setprecision(3) << coeff << " " << factor[exp] << (bit ? unit[1] : unit[0]) << (si ? siSuffix[1] : siSuffix[0]);
    return result.str();
}

// Convert strings like the following into byte counts
//    5MB, 5 MB, 5M, 3.7GB, 123b, 456kB
// with some amount of forgiveness baked into the parsing.
long from_human_readable_byte_count(const char *str)
{
	// Parse leading numeric factor
	char *endptr;
	errno = 0;
	const double coeff = strtod(str, &endptr);
	if (errno) return -1;

	// Skip any intermediate white space
	while (isspace(*endptr)) ++endptr;

	// Read off first character which should be an SI prefix
	int exp = 0;
	int unit = 1024;
	switch (toupper(*endptr))
	{
	case 'B':  exp = 0; break;
	case 'K':  exp = 3; break;
	case 'M':  exp = 6; break;
	case 'G':  exp = 9; break;
	case 'T':  exp = 12; break;
	case 'E':  exp = 15; break;
	case 'Z':  exp = 18; break;
	case 'Y':  exp = 21; break;

	case ' ':
	case '\t':
	case '\0': exp = 0; goto done;

	default:   return -1;
	}
	++endptr;

	// If an 'i' or 'I' is present use SI factor-of-1000 units
	if (toupper(*endptr) == 'I')
	{
		++endptr;
		unit = 1000;
	}

	// Next character must be one of B/empty/whitespace
	switch (toupper(*endptr))
	{
	case 'B':
	case ' ':
	case '\t': ++endptr;  break;

	case '\0': goto done;

	default:   return -1;
	}

	// Skip any remaining white space
	while (isspace(*endptr)) ++endptr;

	// Parse error on anything but a null terminator
	if (*endptr) return -1;

done:
	return exp ? coeff * pow(unit, exp / 3) : coeff;
}