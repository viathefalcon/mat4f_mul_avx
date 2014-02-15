// mat4f_mul_avx.inl: Defines the inline implementation with intrinsics.
//
// Author: Stephen Higgins <sjh@viathefalcon.net>
// Blog: http://blog.viathefalcon.net/
// Twitter: @viathefalcon
//

#ifndef mat4f_mul_avx_inl_
#define mat4f_mul_avx_inl_

// Includes
//

// Intrinsics Headers
#include <intrin.h>

// Functions
//

// Operates on and outputs a pair of columns per iteration of the multiplication step.
// Among other things, using instrinsics allows us to use more AVX registers without having to (manually) save them onto the stack
inline mat4f_t __fastcall mat4f_mul_avx4(__in_ecount(1) const_mat4f_ptr m1, __in_ecount(1) const_mat4f_ptr m2, __out_ecount(1) uint64_ptr pt) {

	// Capture the current value of the timestamp counter
	int info[4] = { 0 };
	__cpuid( info, 0 );
	uint32_t ui = 0;
	const uint64_t u = __rdtscp( &ui );

	// Get in the columns from the second matrix
	_mm256_zeroupper( );
	const __m256 ymm0 = _mm256_loadu_ps( m2->values );
	const __m256 ymm1 = _mm256_loadu_ps( m2->values + 8 );

	mat4f_t m = { 0 };
	for (size_t offset = 0; offset < 16; offset += 8){
		// Get in the next pair of columns from the first matrix, and permute them
		__m256 ymm2 = _mm256_loadu_ps( m1->values + offset );

		__m256 ymm3 = _mm256_shuffle_ps( ymm2, ymm2, 0 );				// 1st element of either half
		__m256 ymm4 = _mm256_shuffle_ps( ymm2, ymm2, 0x55 );			// 2nd element of either half

		__m256 ymm5 = _mm256_permute2f128_ps( ymm3, ymm4, 0x20 );		// Bring the 1st halves together
		__m256 ymm6 = _mm256_permute2f128_ps( ymm3, ymm4, 0x31 );		// Bring the 2nd halves together

		ymm5 = _mm256_mul_ps( ymm0, ymm5 );								// Multiply with the 1st pair of input columns -> a1 b1 c1 d1 e2 f2 g2 h2
		ymm6 = _mm256_mul_ps( ymm0, ymm6 );								// Multiply with the 1st pair of input columns -> a5 b5 c5 d5 e6 f6 g6 h5

		ymm3 = _mm256_shuffle_ps( ymm2, ymm2, 0xAA );					// 3rd element of either half
		ymm4 = _mm256_shuffle_ps( ymm2, ymm2, 0xFF );					// 4th element of either half

		ymm2 = _mm256_permute2f128_ps( ymm3, ymm4, 0x20 );				// Bring the 1st halves together
		ymm3 = _mm256_permute2f128_ps( ymm3, ymm4, 0x31 );				// Bring the 2nd halves together

		ymm2 = _mm256_mul_ps( ymm1, ymm2 );								// Multiply with the 2nd pair of input columns -> i3 j3 k3 l3 m4 n4 o4 p4
		ymm3 = _mm256_mul_ps( ymm1, ymm3 );								// Multiply with the 2nd pair of input columns -> i7 j7 k7 l7 m8 n8 o8 p8

		ymm5 = _mm256_add_ps( ymm5, ymm2 );								// Addition -> a1+i3 b1+j3 c1+k3 d1+l3 e2+m4 f2+n4 g2+o4 h2+p4
		ymm6 = _mm256_add_ps( ymm6, ymm3 );								// Same for the 2nd output column
		ymm4 = _mm256_permute2f128_ps( ymm5, ymm5, 1 );					// Swop the halves of YMM5
		ymm3 = _mm256_permute2f128_ps( ymm6, ymm6, 1 );					// Same for the 2nd output column
		ymm5 = _mm256_add_ps( ymm5, ymm4 );								// Addition -> a1+i3+e2+m4 b1+j3+f2+n4 c1+k3+g2+o4 d1+l3+h2+p4 e2+m4+a1+i3 f2+n4+b1+j3 g2+o4+d1+l3 h2+p4+e2+m4
		ymm6 = _mm256_add_ps( ymm3, ymm6 );								// Same for the 2nd output column

		// Combine the two columns into one output and, er, output
		ymm5 = _mm256_permute2f128_ps( ymm5, ymm6, 0x20 );
		_mm256_storeu_ps( m.values + offset, ymm5 );
	}
	_mm256_zeroupper( );

	// Capture the current value of the timestamp counter
	// and compute the delta since the start
	const uint64_t v = __rdtscp( &ui );
	__cpuid( info, 0 );
	*pt = (v - u);
	return m;
}

#endif // mat4f_mul_avx_inl_
