// mat4f_mul.inl: Declares the types, etc., used in the application.
//
// Author: Stephen Higgins <sjh@viathefalcon.net>
// Blog: http://blog.viathefalcon.net/
// Twitter: @viathefalcon
//

#ifndef mat4f_mul_inl_
#define mat4f_mul_inl_

// Includes
//

// Intrinsics Headers
#include <intrin.h>

// Functions
//

inline mat4f_t __fastcall mat4f_mul(__in_ecount(1) const_mat4f_ptr m1, __in_ecount(1) const_mat4f_ptr m2, __out_ecount(1) uint64_ptr dt) {

	// Capture the current value of the timestamp counter
	int info[4] = { 0 };
	__cpuid( info, 0 );
	uint32_t ui = 0;
	const uint64_t u = __rdtscp( &ui );

	mat4f_t m = { 0 };
	float32_ptr value = m.values;
	(*value++) = m2->values[0]*m1->values[0] + m2->values[4]*m1->values[1] + m2->values[8]*m1->values[2] + m2->values[12]*m1->values[3];
	(*value++) = m2->values[1]*m1->values[0] + m2->values[5]*m1->values[1] + m2->values[9]*m1->values[2] + m2->values[13]*m1->values[3];
	(*value++) = m2->values[2]*m1->values[0] + m2->values[6]*m1->values[1] + m2->values[10]*m1->values[2] + m2->values[14]*m1->values[3];
	(*value++) = m2->values[3]*m1->values[0] + m2->values[7]*m1->values[1] + m2->values[11]*m1->values[2] + m2->values[15]*m1->values[3];
	(*value++) = m2->values[0]*m1->values[4] + m2->values[4]*m1->values[5] + m2->values[8]*m1->values[6] + m2->values[12]*m1->values[7];
	(*value++) = m2->values[1]*m1->values[4] + m2->values[5]*m1->values[5] + m2->values[9]*m1->values[6] + m2->values[13]*m1->values[7];
	(*value++) = m2->values[2]*m1->values[4] + m2->values[6]*m1->values[5] + m2->values[10]*m1->values[6] + m2->values[14]*m1->values[7];
	(*value++) = m2->values[3]*m1->values[4] + m2->values[7]*m1->values[5] + m2->values[11]*m1->values[6] + m2->values[15]*m1->values[7];
	(*value++) = m2->values[0]*m1->values[8] + m2->values[4]*m1->values[9] + m2->values[8]*m1->values[10] + m2->values[12]*m1->values[11];
	(*value++) = m2->values[1]*m1->values[8] + m2->values[5]*m1->values[9] + m2->values[9]*m1->values[10] + m2->values[13]*m1->values[11];
	(*value++) = m2->values[2]*m1->values[8] + m2->values[6]*m1->values[9] + m2->values[10]*m1->values[10] + m2->values[14]*m1->values[11];
	(*value++) = m2->values[3]*m1->values[8] + m2->values[7]*m1->values[9] + m2->values[11]*m1->values[10] + m2->values[15]*m1->values[11];
	(*value++) = m2->values[0]*m1->values[12] + m2->values[4]*m1->values[13] + m2->values[8]*m1->values[14] + m2->values[12]*m1->values[15];
	(*value++) = m2->values[1]*m1->values[12] + m2->values[5]*m1->values[13] + m2->values[9]*m1->values[14] + m2->values[13]*m1->values[15];
	(*value++) = m2->values[2]*m1->values[12] + m2->values[6]*m1->values[13] + m2->values[10]*m1->values[14] + m2->values[14]*m1->values[15];
	(*value++) = m2->values[3]*m1->values[12] + m2->values[7]*m1->values[13] + m2->values[11]*m1->values[14] + m2->values[15]*m1->values[15];

	const uint64_t v = __rdtscp( &ui );
	__cpuid( info, 0 );
	*dt = (v - u);
	return m;
}

#endif // mat4f_mul_inl_
