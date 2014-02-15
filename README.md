Multiplying Matrices with AVX. For Fun*
=======================================

Having previously [tinkered](http://blog.viathefalcon.net/2012/10/03/using-intels-secure-key-rdrand-in-ms-visual-c-2010/) only very briefly, in assembly, I was keen to try my hand at more.

I've been using more or less the same unrolled-loop implementation of a 4x4 matrix multiplication I wrote in university, and it seemed a good candidate for a 21st Century update, using [Advanced Vector Extensions (AVX)](https://en.wikipedia.org/wiki/Advanced_Vector_Extensions) which first shipped with Sandy Bridge processors in 2011: non-trivial, but tractable.

*Performance was never a motivation of this side project.
