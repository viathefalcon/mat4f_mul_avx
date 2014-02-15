// mat4f_mul_avx.cpp: Defines the entry-point for the application.
//
// Author: Stephen Higgins <sjh@viathefalcon.net>
// Blog: http://blog.viathefalcon.net/
// Twitter: @viathefalcon
//

// Includes
//

// Windows Headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// C Standad Library Headers
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <time.h>

// Local Project Headers
#include "mat4f_mul_avx.h"
#include "mat4f_mul.inl"
#include "mat4f_mul_avx.inl"

// Functions
//

// Formats and outputs the given matrix to stdout (and the debugger)
void mat4f_out(__in_z const char* prologue, __in_ecount(1) const mat4f_t* matrix) {

	printf( prologue );
#if defined (_DEBUG)
	OutputDebugStringA( prologue );
#endif

	const size_t cch = 256;
	char buffer[cch] = { 0 };
	const float32_t* f = matrix->values;
	sprintf_s( buffer, cch, "\n%.2f, %.2f, %.2f, %.2f\n", f[0], f[1], f[2], f[3] );
#if defined (_DEBUG)
	OutputDebugStringA( buffer );
#endif
	printf( buffer );
	f += 4;
	sprintf_s( buffer, cch, "%.2f, %.2f, %.2f, %.2f\n", f[0], f[1], f[2], f[3] );
#if defined (_DEBUG)
	OutputDebugStringA( buffer );
#endif
	printf( buffer );
	f += 4;
	sprintf_s( buffer, cch, "%.2f, %.2f, %.2f, %.2f\n", f[0], f[1], f[2], f[3] );
#if defined (_DEBUG)
	OutputDebugStringA( buffer );
#endif
	printf( buffer );
	f += 4;
	sprintf_s( buffer, cch, "%.2f, %.2f, %.2f, %.2f\n", f[0], f[1], f[2], f[3] );
#if defined (_DEBUG)
	OutputDebugStringA( buffer );
#endif
	printf( buffer );
}

// Returns a 4x4 amtrix populated with random values
mat4f_t mat4f_rand(void) {

	const float32_t denominator = (float32_t) rand( );

	mat4f_t m = { 0 };
	float32_ptr fp = m.values;
	for (int i = 0; i < 16; i++){
		const float32_t f = (float32_t) rand( );
		*fp++ = (f / denominator);
	}
	return m;
}

// Gives the entry-point
int wmain(__in int argc, __in_ecount(argc) wchar_t* argv[]) {

	// Parse the command line arguments, if any
	uint32_t runs = 1;
#if !defined (_DEBUG)
	runs <<= 16; // 2^16
#endif
	if (argc > 1){
		runs = (uint32_t) _wtoi( argv[1] );
	}else{
		printf( "Usage mat4f_mul_avx <number of iterations>\n\n" );
	}

	// Seed the RNG
	srand( (unsigned) time( NULL ) );

	const char* labels[] = {"mat4f_mul:",  "mat4f_mul_avx1:", "mat4f_mul_avx2:", "mat4f_mul_avx3:", "mat4f_mul_avx4:" };
	mat4f_mul_t funcs[] = { mat4f_mul, mat4f_mul_avx1, mat4f_mul_avx2, mat4f_mul_avx3, mat4f_mul_avx4 };
	for (size_t s = 0; s < ARRAYSIZE( funcs ); s++){
		const char* label = *(labels + s);
		mat4f_mul_t func = *(funcs + s);

		switch (runs){
			case 1:
				{
					// Gin up a pair of matrices to use as multiplicands
					const mat4f_t ma = mat4f_rand( );
					const mat4f_t mb = mat4f_rand( );

					// Multiply
					uint64_t dt = 0;
					mat4f_t m = func( &ma, &mb, &dt );

					// Output
					mat4f_out( label, &m );
					printf( "%s 1 run took %I64d cycles.\n\n", label, dt );
				}
				break;

			default:
				{
					// Call the function up to many times with different inputs
					// as might occur in a rendering loop in a game or other simulation 
					uint64_t t = 0;
					for (uint32_t u = 0; u < runs; u++){
						// Gin up a pair of matrices to use as multiplicands
						const mat4f_t ma = mat4f_rand( );
						const mat4f_t mb = mat4f_rand( );

						// Multiply
						uint64_t dt = 0;
						mat4f_t m = func( &ma, &mb, &dt );
						t += dt;
					}

					// Get the average
					const double avg = (double) t / (double) runs;
					printf( "%s %u runs took %I64d (%f avg) cycles\n", label, runs, t, avg );
				}
				break;
		}
	}
	return 0;
}
