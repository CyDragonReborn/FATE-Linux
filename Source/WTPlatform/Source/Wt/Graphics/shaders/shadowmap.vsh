//-----------------------------------------------------------------------------
// Vertex shader to compute normalized depth for storing in shadow map
//-----------------------------------------------------------------------------

vs.1.1

dcl_position v0

// Transform and output position
m4x4 r0, v0, c0
mov oPos, r0

// Output depth
rcp r1, r0.w
mul oT0.x, r1, r0.z
