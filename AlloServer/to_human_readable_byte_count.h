#pragma once

void to_human_readable_byte_count(long bytes,
	int si,
	int bit,
	double* coeff,
	const char** units);