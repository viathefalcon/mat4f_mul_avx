Multiplying Matrices with AVX. For Fun*
=======================================

Having previously [tinkered](http://blog.viathefalcon.net/2012/10/03/using-intels-secure-key-rdrand-in-ms-visual-c-2010/) only very briefly, in assembly, I was keen to try my hand at more.

I do best with a practical, defined problem to solve; having used more or less the same unrolled-loop implementation of a 4x4 matrix multiplication I wrote in university, it seemed a good candidate for a 21st Century update, using [Advanced Vector Extensions (AVX)](https://en.wikipedia.org/wiki/Advanced_Vector_Extensions) which first shipped with Sandy Bridge processors in 2011: non-trivial, but tractable.

*Performance was never a motivation of this side project - the problem is too small - but there wouldn't be much point if the output were slower. And it isn't: on my (Ivy Bridge) Macbook Pro, it executes in half as many cycles as my previous unrolled-loop implementation and in slightly more than two-thirds as many cycles on a [Haswell Ultrabook](http://www.dell.com/us/p/xps-13-9333/pd).

But not faster than [XMMatrixMultiply](http://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.matrix.xmmatrixmultiply(v=vs.85).aspx).

P.S. The built executable has a dependency on the [Visual C++ 2012 Update 4 runtime](http://www.microsoft.com/en-us/download/details.aspx?id=30679) and does not check that the host CPU supports AVX instructions.
