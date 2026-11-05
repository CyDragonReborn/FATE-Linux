#pragma once

// Numeric types only: these preserve the existing engine math API without
// importing Windows headers, COM, Direct3D, or a Wine runtime.
#include <cstdint>
using FLOAT = float;
using DWORD = std::uint32_t;
using UINT = std::uint32_t;
using BOOL = int;
struct D3DVECTOR { float x, y, z; };
struct D3DCOLORVALUE { float r, g, b, a; };
struct D3DMATRIX {
    union {
        struct {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
    };
};
struct D3DGAMMARAMP {
    std::uint16_t red[256], green[256], blue[256];
};
static_assert(sizeof(D3DVECTOR) == 12 && sizeof(D3DMATRIX) == 64);

#include "d3dx8math.h"
