// mat4f_mul_avx.h: Declares the types, etc., used in the application.
//
// Author: Stephen Higgins <sjh@viathefalcon.net>
// Blog: http://blog.viathefalcon.net/
// Twitter: @viathefalcon
//

#ifndef mat4f_mul_avx_h_
#define mat4f_mul_avx_h_

// Includes
//

// SAL Headers
#include <sal.h>

// Types
//

// 32 bit integers
typedef signed __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int32* uint32_ptr;

// 64 bit integers
typedef unsigned __int64 uint64_t;
typedef unsigned __int64* uint64_ptr;

// 32 bit floating-point
typedef float float32_t;
typedef float32_t* float32_ptr;

typedef struct _mat4f_t {

	float32_t values[16];

} mat4f_t, *mat4f_ptr;
typedef const mat4f_t* const_mat4f_ptr;

// Defines the signature of a function which implements a 4x4 matrix multiplication
typedef mat4f_t (__fastcall* mat4f_mul_t)(__in_ecount(1) const mat4f_t*, __in_ecount(1) const mat4f_t*, __out_ecount(1) uint64_ptr);

// Prototypes
//

// Prototypes for functions implemented elsewhere in assembly
extern "C" mat4f_t __fastcall mat4f_mul_avx0(__in_ecount(1) const mat4f_t*, __in_ecount(1) const mat4f_t*, __out_ecount(1) uint64_ptr);
extern "C" mat4f_t __fastcall mat4f_mul_avx1(__in_ecount(1) const mat4f_t*, __in_ecount(1) const mat4f_t*, __out_ecount(1) uint64_ptr);
extern "C" mat4f_t __fastcall mat4f_mul_avx2(__in_ecount(1) const mat4f_t*, __in_ecount(1) const mat4f_t*, __out_ecount(1) uint64_ptr);
extern "C" mat4f_t __fastcall mat4f_mul_avx3(__in_ecount(1) const mat4f_t*, __in_ecount(1) const mat4f_t*, __out_ecount(1) uint64_ptr);

#endif // mat4f_mul_avx_h_
