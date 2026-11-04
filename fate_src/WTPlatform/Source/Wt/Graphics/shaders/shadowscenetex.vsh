//-----------------------------------------------------------------------------
// Vertex shader for rendering shadowed diffuse light contribution 
// This takes into account the vertex and texture colors of the receiving material
// c0..3	- model-to-screen transform
// c4		- opposite of light direction in model coordinates
// c5		- light diffuse color
// c8..11	- model-to-shadow texture transform
// c12..15	- model-to-shadow transform, with z-bias
//-----------------------------------------------------------------------------

vs.1.1

dcl_position v0
dcl_normal v1
dcl_color v2.rgba
dcl_texcoord v3

// Transform and output position
m4x4 oPos, v0, c0

// Diffuse lighting
dp3 r0, v1, c4
mul r0, r0, c5
mul oD0, r0, v2

mov oT0, v3

// Shadow map projection
//m4x4 oT1, v0, c8
m4x4 r1, v0, c8
rcp r2, r1.w
mul oT1, r2, r1

// Depth in light space, plus Z-bias
m4x4 r1, v0, c12
rcp r2, r1.w
mul oT2.x, r2, r1.z
