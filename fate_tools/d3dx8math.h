/*
 * d3dx8math.h  --  clean-room replacement for the D3DX8 math library.
 *
 * Header-only, no DLL needed.  Types derive from D3DVECTOR / D3DMATRIX like the
 * originals so existing code (casts, m[i][j], _11.._44, operators) keeps working.
 * Conventions match D3DX: row vectors, left-handed.
 *
 * Needs (from d3d8types.h or a test stub): FLOAT, DWORD, BOOL, UINT, D3DVECTOR,
 * D3DMATRIX, D3DCOLORVALUE.
 */
#ifndef D3DX8MATH_SHIM_H
#define D3DX8MATH_SHIM_H

#include <math.h>

#define D3DX_PI        3.141592654f
#define D3DX_1BYPI     0.318309886f
#define D3DXToRadian(d) ((d) * (D3DX_PI / 180.0f))
#define D3DXToDegree(r) ((r) * (180.0f / D3DX_PI))
#define D3DXINLINE inline

/* ------------------------------------------------------------------ types */
struct D3DXVECTOR2 {
    D3DXVECTOR2() {}
    D3DXVECTOR2(const FLOAT *p) : x(p[0]), y(p[1]) {}
    D3DXVECTOR2(FLOAT fx, FLOAT fy) : x(fx), y(fy) {}
    operator FLOAT*() { return &x; }
    operator const FLOAT*() const { return &x; }
    D3DXVECTOR2& operator+=(const D3DXVECTOR2& v) { x += v.x; y += v.y; return *this; }
    D3DXVECTOR2& operator-=(const D3DXVECTOR2& v) { x -= v.x; y -= v.y; return *this; }
    D3DXVECTOR2& operator*=(FLOAT f) { x *= f; y *= f; return *this; }
    D3DXVECTOR2& operator/=(FLOAT f) { FLOAT i = 1.0f / f; x *= i; y *= i; return *this; }
    D3DXVECTOR2 operator+() const { return *this; }
    D3DXVECTOR2 operator-() const { return D3DXVECTOR2(-x, -y); }
    D3DXVECTOR2 operator+(const D3DXVECTOR2& v) const { return D3DXVECTOR2(x + v.x, y + v.y); }
    D3DXVECTOR2 operator-(const D3DXVECTOR2& v) const { return D3DXVECTOR2(x - v.x, y - v.y); }
    D3DXVECTOR2 operator*(FLOAT f) const { return D3DXVECTOR2(x * f, y * f); }
    D3DXVECTOR2 operator/(FLOAT f) const { FLOAT i = 1.0f / f; return D3DXVECTOR2(x * i, y * i); }
    friend D3DXVECTOR2 operator*(FLOAT f, const D3DXVECTOR2& v) { return D3DXVECTOR2(f * v.x, f * v.y); }
    BOOL operator==(const D3DXVECTOR2& v) const { return x == v.x && y == v.y; }
    BOOL operator!=(const D3DXVECTOR2& v) const { return x != v.x || y != v.y; }
    FLOAT x, y;
};

struct D3DXVECTOR3 : public D3DVECTOR {
    D3DXVECTOR3() {}
    D3DXVECTOR3(const FLOAT *p) { x = p[0]; y = p[1]; z = p[2]; }
    D3DXVECTOR3(const D3DVECTOR& v) { x = v.x; y = v.y; z = v.z; }
    D3DXVECTOR3(FLOAT fx, FLOAT fy, FLOAT fz) { x = fx; y = fy; z = fz; }
    operator FLOAT*() { return &x; }
    operator const FLOAT*() const { return &x; }
    D3DXVECTOR3& operator+=(const D3DXVECTOR3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    D3DXVECTOR3& operator-=(const D3DXVECTOR3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    D3DXVECTOR3& operator*=(FLOAT f) { x *= f; y *= f; z *= f; return *this; }
    D3DXVECTOR3& operator/=(FLOAT f) { FLOAT i = 1.0f / f; x *= i; y *= i; z *= i; return *this; }
    D3DXVECTOR3 operator+() const { return *this; }
    D3DXVECTOR3 operator-() const { return D3DXVECTOR3(-x, -y, -z); }
    D3DXVECTOR3 operator+(const D3DXVECTOR3& v) const { return D3DXVECTOR3(x + v.x, y + v.y, z + v.z); }
    D3DXVECTOR3 operator-(const D3DXVECTOR3& v) const { return D3DXVECTOR3(x - v.x, y - v.y, z - v.z); }
    D3DXVECTOR3 operator*(FLOAT f) const { return D3DXVECTOR3(x * f, y * f, z * f); }
    D3DXVECTOR3 operator/(FLOAT f) const { FLOAT i = 1.0f / f; return D3DXVECTOR3(x * i, y * i, z * i); }
    friend D3DXVECTOR3 operator*(FLOAT f, const D3DXVECTOR3& v) { return D3DXVECTOR3(f * v.x, f * v.y, f * v.z); }
    BOOL operator==(const D3DXVECTOR3& v) const { return x == v.x && y == v.y && z == v.z; }
    BOOL operator!=(const D3DXVECTOR3& v) const { return x != v.x || y != v.y || z != v.z; }
};

struct D3DXVECTOR4 {
    D3DXVECTOR4() {}
    D3DXVECTOR4(const FLOAT *p) : x(p[0]), y(p[1]), z(p[2]), w(p[3]) {}
    D3DXVECTOR4(const D3DXVECTOR3& v, FLOAT fw) : x(v.x), y(v.y), z(v.z), w(fw) {}
    D3DXVECTOR4(FLOAT fx, FLOAT fy, FLOAT fz, FLOAT fw) : x(fx), y(fy), z(fz), w(fw) {}
    operator FLOAT*() { return &x; }
    operator const FLOAT*() const { return &x; }
    D3DXVECTOR4& operator+=(const D3DXVECTOR4& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
    D3DXVECTOR4& operator-=(const D3DXVECTOR4& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
    D3DXVECTOR4& operator*=(FLOAT f) { x *= f; y *= f; z *= f; w *= f; return *this; }
    D3DXVECTOR4& operator/=(FLOAT f) { FLOAT i = 1.0f / f; x *= i; y *= i; z *= i; w *= i; return *this; }
    D3DXVECTOR4 operator+() const { return *this; }
    D3DXVECTOR4 operator-() const { return D3DXVECTOR4(-x, -y, -z, -w); }
    D3DXVECTOR4 operator+(const D3DXVECTOR4& v) const { return D3DXVECTOR4(x + v.x, y + v.y, z + v.z, w + v.w); }
    D3DXVECTOR4 operator-(const D3DXVECTOR4& v) const { return D3DXVECTOR4(x - v.x, y - v.y, z - v.z, w - v.w); }
    D3DXVECTOR4 operator*(FLOAT f) const { return D3DXVECTOR4(x * f, y * f, z * f, w * f); }
    D3DXVECTOR4 operator/(FLOAT f) const { FLOAT i = 1.0f / f; return D3DXVECTOR4(x * i, y * i, z * i, w * i); }
    friend D3DXVECTOR4 operator*(FLOAT f, const D3DXVECTOR4& v) { return D3DXVECTOR4(f * v.x, f * v.y, f * v.z, f * v.w); }
    BOOL operator==(const D3DXVECTOR4& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
    BOOL operator!=(const D3DXVECTOR4& v) const { return x != v.x || y != v.y || z != v.z || w != v.w; }
    FLOAT x, y, z, w;
};

struct D3DXMATRIX : public D3DMATRIX {
    D3DXMATRIX() {}
    D3DXMATRIX(const FLOAT *p) { for (int i = 0; i < 16; i++) (&_11)[i] = p[i]; }
    D3DXMATRIX(const D3DMATRIX& m) { *(D3DMATRIX*)this = m; }
    D3DXMATRIX(FLOAT f11, FLOAT f12, FLOAT f13, FLOAT f14,
               FLOAT f21, FLOAT f22, FLOAT f23, FLOAT f24,
               FLOAT f31, FLOAT f32, FLOAT f33, FLOAT f34,
               FLOAT f41, FLOAT f42, FLOAT f43, FLOAT f44) {
        _11 = f11; _12 = f12; _13 = f13; _14 = f14;
        _21 = f21; _22 = f22; _23 = f23; _24 = f24;
        _31 = f31; _32 = f32; _33 = f33; _34 = f34;
        _41 = f41; _42 = f42; _43 = f43; _44 = f44;
    }
    FLOAT& operator()(UINT row, UINT col) { return m[row][col]; }
    FLOAT  operator()(UINT row, UINT col) const { return m[row][col]; }
    operator FLOAT*() { return &_11; }
    operator const FLOAT*() const { return &_11; }
    D3DXMATRIX& operator*=(const D3DXMATRIX& rhs);
    D3DXMATRIX& operator+=(const D3DXMATRIX& r) { for (int i = 0; i < 16; i++) (&_11)[i] += (&r._11)[i]; return *this; }
    D3DXMATRIX& operator-=(const D3DXMATRIX& r) { for (int i = 0; i < 16; i++) (&_11)[i] -= (&r._11)[i]; return *this; }
    D3DXMATRIX& operator*=(FLOAT f) { for (int i = 0; i < 16; i++) (&_11)[i] *= f; return *this; }
    D3DXMATRIX& operator/=(FLOAT f) { FLOAT inv = 1.0f / f; for (int i = 0; i < 16; i++) (&_11)[i] *= inv; return *this; }
    D3DXMATRIX operator+() const { return *this; }
    D3DXMATRIX operator-() const { D3DXMATRIX r; for (int i = 0; i < 16; i++) (&r._11)[i] = -(&_11)[i]; return r; }
    D3DXMATRIX operator*(const D3DXMATRIX& rhs) const { D3DXMATRIX r(*this); r *= rhs; return r; }
    D3DXMATRIX operator+(const D3DXMATRIX& rhs) const { D3DXMATRIX r(*this); r += rhs; return r; }
    D3DXMATRIX operator-(const D3DXMATRIX& rhs) const { D3DXMATRIX r(*this); r -= rhs; return r; }
    D3DXMATRIX operator*(FLOAT f) const { D3DXMATRIX r(*this); r *= f; return r; }
    D3DXMATRIX operator/(FLOAT f) const { D3DXMATRIX r(*this); r /= f; return r; }
    friend D3DXMATRIX operator*(FLOAT f, const D3DXMATRIX& m) { return m * f; }
    BOOL operator==(const D3DXMATRIX& r) const { for (int i = 0; i < 16; i++) if ((&_11)[i] != (&r._11)[i]) return 0; return 1; }
    BOOL operator!=(const D3DXMATRIX& r) const { return !(*this == r); }
};

struct D3DXQUATERNION {
    D3DXQUATERNION() {}
    D3DXQUATERNION(const FLOAT *p) : x(p[0]), y(p[1]), z(p[2]), w(p[3]) {}
    D3DXQUATERNION(FLOAT fx, FLOAT fy, FLOAT fz, FLOAT fw) : x(fx), y(fy), z(fz), w(fw) {}
    operator FLOAT*() { return &x; }
    operator const FLOAT*() const { return &x; }
    D3DXQUATERNION& operator+=(const D3DXQUATERNION& q) { x += q.x; y += q.y; z += q.z; w += q.w; return *this; }
    D3DXQUATERNION& operator-=(const D3DXQUATERNION& q) { x -= q.x; y -= q.y; z -= q.z; w -= q.w; return *this; }
    D3DXQUATERNION& operator*=(const D3DXQUATERNION& q);
    D3DXQUATERNION& operator*=(FLOAT f) { x *= f; y *= f; z *= f; w *= f; return *this; }
    D3DXQUATERNION& operator/=(FLOAT f) { FLOAT i = 1.0f / f; x *= i; y *= i; z *= i; w *= i; return *this; }
    D3DXQUATERNION operator+() const { return *this; }
    D3DXQUATERNION operator-() const { return D3DXQUATERNION(-x, -y, -z, -w); }
    D3DXQUATERNION operator+(const D3DXQUATERNION& q) const { return D3DXQUATERNION(x + q.x, y + q.y, z + q.z, w + q.w); }
    D3DXQUATERNION operator-(const D3DXQUATERNION& q) const { return D3DXQUATERNION(x - q.x, y - q.y, z - q.z, w - q.w); }
    D3DXQUATERNION operator*(const D3DXQUATERNION& q) const { D3DXQUATERNION r(*this); r *= q; return r; }
    D3DXQUATERNION operator*(FLOAT f) const { return D3DXQUATERNION(x * f, y * f, z * f, w * f); }
    D3DXQUATERNION operator/(FLOAT f) const { FLOAT i = 1.0f / f; return D3DXQUATERNION(x * i, y * i, z * i, w * i); }
    friend D3DXQUATERNION operator*(FLOAT f, const D3DXQUATERNION& q) { return D3DXQUATERNION(f * q.x, f * q.y, f * q.z, f * q.w); }
    BOOL operator==(const D3DXQUATERNION& q) const { return x == q.x && y == q.y && z == q.z && w == q.w; }
    BOOL operator!=(const D3DXQUATERNION& q) const { return !(*this == q); }
    FLOAT x, y, z, w;
};

struct D3DXPLANE {
    D3DXPLANE() {}
    D3DXPLANE(const FLOAT *p) : a(p[0]), b(p[1]), c(p[2]), d(p[3]) {}
    D3DXPLANE(FLOAT fa, FLOAT fb, FLOAT fc, FLOAT fd) : a(fa), b(fb), c(fc), d(fd) {}
    operator FLOAT*() { return &a; }
    operator const FLOAT*() const { return &a; }
    D3DXPLANE operator+() const { return *this; }
    D3DXPLANE operator-() const { return D3DXPLANE(-a, -b, -c, -d); }
    D3DXPLANE& operator*=(FLOAT f) { a *= f; b *= f; c *= f; d *= f; return *this; }
    D3DXPLANE& operator/=(FLOAT f) { FLOAT i = 1.0f / f; a *= i; b *= i; c *= i; d *= i; return *this; }
    D3DXPLANE operator*(FLOAT f) const { return D3DXPLANE(a * f, b * f, c * f, d * f); }
    D3DXPLANE operator/(FLOAT f) const { FLOAT i = 1.0f / f; return D3DXPLANE(a * i, b * i, c * i, d * i); }
    friend D3DXPLANE operator*(FLOAT f, const D3DXPLANE& p) { return D3DXPLANE(f * p.a, f * p.b, f * p.c, f * p.d); }
    BOOL operator==(const D3DXPLANE& p) const { return a == p.a && b == p.b && c == p.c && d == p.d; }
    BOOL operator!=(const D3DXPLANE& p) const { return !(*this == p); }
    FLOAT a, b, c, d;
};

struct D3DXCOLOR {
    D3DXCOLOR() {}
    D3DXCOLOR(DWORD argb)
        : r(((argb >> 16) & 0xff) * (1.0f / 255.0f)), g(((argb >> 8) & 0xff) * (1.0f / 255.0f)),
          b((argb & 0xff) * (1.0f / 255.0f)), a(((argb >> 24) & 0xff) * (1.0f / 255.0f)) {}
    D3DXCOLOR(const FLOAT *p) : r(p[0]), g(p[1]), b(p[2]), a(p[3]) {}
    D3DXCOLOR(const D3DCOLORVALUE& c) : r(c.r), g(c.g), b(c.b), a(c.a) {}
    D3DXCOLOR(FLOAT fr, FLOAT fg, FLOAT fb, FLOAT fa) : r(fr), g(fg), b(fb), a(fa) {}
    operator DWORD() const {
        DWORD A = a >= 1.0f ? 255 : a <= 0.0f ? 0 : (DWORD)(a * 255.0f + 0.5f);
        DWORD R = r >= 1.0f ? 255 : r <= 0.0f ? 0 : (DWORD)(r * 255.0f + 0.5f);
        DWORD G = g >= 1.0f ? 255 : g <= 0.0f ? 0 : (DWORD)(g * 255.0f + 0.5f);
        DWORD B = b >= 1.0f ? 255 : b <= 0.0f ? 0 : (DWORD)(b * 255.0f + 0.5f);
        return (A << 24) | (R << 16) | (G << 8) | B;
    }
    operator FLOAT*() { return &r; }
    operator const FLOAT*() const { return &r; }
    operator D3DCOLORVALUE*() { return (D3DCOLORVALUE*)&r; }
    operator const D3DCOLORVALUE*() const { return (const D3DCOLORVALUE*)&r; }
    operator D3DCOLORVALUE&() { return *(D3DCOLORVALUE*)&r; }
    operator const D3DCOLORVALUE&() const { return *(const D3DCOLORVALUE*)&r; }
    D3DXCOLOR& operator+=(const D3DXCOLOR& c) { r += c.r; g += c.g; b += c.b; a += c.a; return *this; }
    D3DXCOLOR& operator-=(const D3DXCOLOR& c) { r -= c.r; g -= c.g; b -= c.b; a -= c.a; return *this; }
    D3DXCOLOR& operator*=(FLOAT f) { r *= f; g *= f; b *= f; a *= f; return *this; }
    D3DXCOLOR& operator/=(FLOAT f) { FLOAT i = 1.0f / f; r *= i; g *= i; b *= i; a *= i; return *this; }
    D3DXCOLOR operator+() const { return *this; }
    D3DXCOLOR operator-() const { return D3DXCOLOR(-r, -g, -b, -a); }
    D3DXCOLOR operator+(const D3DXCOLOR& c) const { return D3DXCOLOR(r + c.r, g + c.g, b + c.b, a + c.a); }
    D3DXCOLOR operator-(const D3DXCOLOR& c) const { return D3DXCOLOR(r - c.r, g - c.g, b - c.b, a - c.a); }
    D3DXCOLOR operator*(FLOAT f) const { return D3DXCOLOR(r * f, g * f, b * f, a * f); }
    D3DXCOLOR operator/(FLOAT f) const { FLOAT i = 1.0f / f; return D3DXCOLOR(r * i, g * i, b * i, a * i); }
    friend D3DXCOLOR operator*(FLOAT f, const D3DXCOLOR& c) { return D3DXCOLOR(f * c.r, f * c.g, f * c.b, f * c.a); }
    BOOL operator==(const D3DXCOLOR& c) const { return r == c.r && g == c.g && b == c.b && a == c.a; }
    BOOL operator!=(const D3DXCOLOR& c) const { return !(*this == c); }
    FLOAT r, g, b, a;
};

/* ---------------------------------------------------------------- Vec3 */
D3DXINLINE FLOAT D3DXVec3LengthSq(const D3DXVECTOR3 *v) { return v->x * v->x + v->y * v->y + v->z * v->z; }
D3DXINLINE FLOAT D3DXVec3Length(const D3DXVECTOR3 *v) { return sqrtf(D3DXVec3LengthSq(v)); }
D3DXINLINE FLOAT D3DXVec3Dot(const D3DXVECTOR3 *a, const D3DXVECTOR3 *b) { return a->x * b->x + a->y * b->y + a->z * b->z; }
D3DXINLINE D3DXVECTOR3* D3DXVec3Cross(D3DXVECTOR3 *out, const D3DXVECTOR3 *a, const D3DXVECTOR3 *b) {
    D3DXVECTOR3 t(a->y * b->z - a->z * b->y, a->z * b->x - a->x * b->z, a->x * b->y - a->y * b->x);
    *out = t; return out;
}
D3DXINLINE D3DXVECTOR3* D3DXVec3Normalize(D3DXVECTOR3 *out, const D3DXVECTOR3 *v) {
    FLOAT l = D3DXVec3Length(v);
    if (l == 0.0f) { out->x = out->y = out->z = 0.0f; }
    else { FLOAT i = 1.0f / l; D3DXVECTOR3 t(v->x * i, v->y * i, v->z * i); *out = t; }
    return out;
}
D3DXINLINE D3DXVECTOR3* D3DXVec3Lerp(D3DXVECTOR3 *out, const D3DXVECTOR3 *a, const D3DXVECTOR3 *b, FLOAT s) {
    D3DXVECTOR3 t(a->x + s * (b->x - a->x), a->y + s * (b->y - a->y), a->z + s * (b->z - a->z));
    *out = t; return out;
}
D3DXINLINE D3DXVECTOR3* D3DXVec3CatmullRom(D3DXVECTOR3 *out, const D3DXVECTOR3 *v0, const D3DXVECTOR3 *v1,
                                           const D3DXVECTOR3 *v2, const D3DXVECTOR3 *v3, FLOAT s) {
    FLOAT s2 = s * s, s3 = s2 * s;
    const FLOAT *p0 = &v0->x, *p1 = &v1->x, *p2 = &v2->x, *p3 = &v3->x;
    FLOAT r[3];
    for (int i = 0; i < 3; i++)
        r[i] = 0.5f * (2.0f * p1[i] + (-p0[i] + p2[i]) * s + (2.0f * p0[i] - 5.0f * p1[i] + 4.0f * p2[i] - p3[i]) * s2 +
                       (-p0[i] + 3.0f * p1[i] - 3.0f * p2[i] + p3[i]) * s3);
    out->x = r[0]; out->y = r[1]; out->z = r[2]; return out;
}
D3DXINLINE D3DXVECTOR3* D3DXVec3TransformCoord(D3DXVECTOR3 *out, const D3DXVECTOR3 *v, const D3DXMATRIX *m) {
    FLOAT x = v->x * m->_11 + v->y * m->_21 + v->z * m->_31 + m->_41;
    FLOAT y = v->x * m->_12 + v->y * m->_22 + v->z * m->_32 + m->_42;
    FLOAT z = v->x * m->_13 + v->y * m->_23 + v->z * m->_33 + m->_43;
    FLOAT w = v->x * m->_14 + v->y * m->_24 + v->z * m->_34 + m->_44;
    FLOAT i = (w != 0.0f) ? 1.0f / w : 0.0f;
    out->x = x * i; out->y = y * i; out->z = z * i; return out;
}
D3DXINLINE D3DXVECTOR3* D3DXVec3TransformNormal(D3DXVECTOR3 *out, const D3DXVECTOR3 *v, const D3DXMATRIX *m) {
    FLOAT x = v->x * m->_11 + v->y * m->_21 + v->z * m->_31;
    FLOAT y = v->x * m->_12 + v->y * m->_22 + v->z * m->_32;
    FLOAT z = v->x * m->_13 + v->y * m->_23 + v->z * m->_33;
    out->x = x; out->y = y; out->z = z; return out;
}

/* -------------------------------------------------------------- Matrix */
D3DXINLINE D3DXMATRIX* D3DXMatrixIdentity(D3DXMATRIX *o) {
    for (int i = 0; i < 16; i++) (&o->_11)[i] = 0.0f;
    o->_11 = o->_22 = o->_33 = o->_44 = 1.0f; return o;
}
D3DXINLINE D3DXMATRIX* D3DXMatrixMultiply(D3DXMATRIX *o, const D3DXMATRIX *a, const D3DXMATRIX *b) {
    D3DXMATRIX t;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            t.m[i][j] = a->m[i][0] * b->m[0][j] + a->m[i][1] * b->m[1][j] + a->m[i][2] * b->m[2][j] + a->m[i][3] * b->m[3][j];
    *o = t; return o;
}
D3DXINLINE D3DXMATRIX& D3DXMATRIX::operator*=(const D3DXMATRIX& rhs) { D3DXMatrixMultiply(this, this, &rhs); return *this; }
D3DXINLINE D3DXMATRIX* D3DXMatrixTranspose(D3DXMATRIX *o, const D3DXMATRIX *m) {
    D3DXMATRIX t;
    for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) t.m[i][j] = m->m[j][i];
    *o = t; return o;
}
D3DXINLINE D3DXMATRIX* D3DXMatrixInverse(D3DXMATRIX *o, FLOAT *pDet, const D3DXMATRIX *in) {
    const FLOAT *m = &in->_11; FLOAT inv[16];
    inv[0]  =  m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15] + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
    inv[4]  = -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15] - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
    inv[8]  =  m[4]*m[9]*m[15]  - m[4]*m[11]*m[13] - m[8]*m[5]*m[15] + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
    inv[12] = -m[4]*m[9]*m[14]  + m[4]*m[10]*m[13] + m[8]*m[5]*m[14] - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
    inv[1]  = -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15] - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
    inv[5]  =  m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15] + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
    inv[9]  = -m[0]*m[9]*m[15]  + m[0]*m[11]*m[13] + m[8]*m[1]*m[15] - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
    inv[13] =  m[0]*m[9]*m[14]  - m[0]*m[10]*m[13] - m[8]*m[1]*m[14] + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
    inv[2]  =  m[1]*m[6]*m[15]  - m[1]*m[7]*m[14]  - m[5]*m[2]*m[15] + m[5]*m[3]*m[14] + m[13]*m[2]*m[7]  - m[13]*m[3]*m[6];
    inv[6]  = -m[0]*m[6]*m[15]  + m[0]*m[7]*m[14]  + m[4]*m[2]*m[15] - m[4]*m[3]*m[14] - m[12]*m[2]*m[7]  + m[12]*m[3]*m[6];
    inv[10] =  m[0]*m[5]*m[15]  - m[0]*m[7]*m[13]  - m[4]*m[1]*m[15] + m[4]*m[3]*m[13] + m[12]*m[1]*m[7]  - m[12]*m[3]*m[5];
    inv[14] = -m[0]*m[5]*m[14]  + m[0]*m[6]*m[13]  + m[4]*m[1]*m[14] - m[4]*m[2]*m[13] - m[12]*m[1]*m[6]  + m[12]*m[2]*m[5];
    inv[3]  = -m[1]*m[6]*m[11]  + m[1]*m[7]*m[10]  + m[5]*m[2]*m[11] - m[5]*m[3]*m[10] - m[9]*m[2]*m[7]   + m[9]*m[3]*m[6];
    inv[7]  =  m[0]*m[6]*m[11]  - m[0]*m[7]*m[10]  - m[4]*m[2]*m[11] + m[4]*m[3]*m[10] + m[8]*m[2]*m[7]   - m[8]*m[3]*m[6];
    inv[11] = -m[0]*m[5]*m[11]  + m[0]*m[7]*m[9]   + m[4]*m[1]*m[11] - m[4]*m[3]*m[9]  - m[8]*m[1]*m[7]   + m[8]*m[3]*m[5];
    inv[15] =  m[0]*m[5]*m[10]  - m[0]*m[6]*m[9]   - m[4]*m[1]*m[10] + m[4]*m[2]*m[9]  + m[8]*m[1]*m[6]   - m[8]*m[2]*m[5];
    FLOAT det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
    if (pDet) *pDet = det;
    if (det == 0.0f) return 0;
    FLOAT id = 1.0f / det;
    for (int i = 0; i < 16; i++) (&o->_11)[i] = inv[i] * id;
    return o;
}
D3DXINLINE D3DXMATRIX* D3DXMatrixScaling(D3DXMATRIX *o, FLOAT sx, FLOAT sy, FLOAT sz) {
    D3DXMatrixIdentity(o); o->_11 = sx; o->_22 = sy; o->_33 = sz; return o;
}
D3DXINLINE D3DXMATRIX* D3DXMatrixTranslation(D3DXMATRIX *o, FLOAT x, FLOAT y, FLOAT z) {
    D3DXMatrixIdentity(o); o->_41 = x; o->_42 = y; o->_43 = z; return o;
}
D3DXINLINE D3DXMATRIX* D3DXMatrixRotationX(D3DXMATRIX *o, FLOAT a) {
    D3DXMatrixIdentity(o); FLOAT s = sinf(a), c = cosf(a);
    o->_22 = c; o->_23 = s; o->_32 = -s; o->_33 = c; return o;
}
D3DXINLINE D3DXMATRIX* D3DXMatrixRotationY(D3DXMATRIX *o, FLOAT a) {
    D3DXMatrixIdentity(o); FLOAT s = sinf(a), c = cosf(a);
    o->_11 = c; o->_13 = -s; o->_31 = s; o->_33 = c; return o;
}
D3DXINLINE D3DXMATRIX* D3DXMatrixRotationZ(D3DXMATRIX *o, FLOAT a) {
    D3DXMatrixIdentity(o); FLOAT s = sinf(a), c = cosf(a);
    o->_11 = c; o->_12 = s; o->_21 = -s; o->_22 = c; return o;
}
D3DXINLINE D3DXMATRIX* D3DXMatrixRotationQuaternion(D3DXMATRIX *o, const D3DXQUATERNION *q) {
    FLOAT x = q->x, y = q->y, z = q->z, w = q->w;
    D3DXMatrixIdentity(o);
    o->_11 = 1.0f - 2.0f * (y * y + z * z); o->_12 = 2.0f * (x * y + z * w);        o->_13 = 2.0f * (x * z - y * w);
    o->_21 = 2.0f * (x * y - z * w);        o->_22 = 1.0f - 2.0f * (x * x + z * z); o->_23 = 2.0f * (y * z + x * w);
    o->_31 = 2.0f * (x * z + y * w);        o->_32 = 2.0f * (y * z - x * w);        o->_33 = 1.0f - 2.0f * (x * x + y * y);
    return o;
}
D3DXINLINE D3DXMATRIX* D3DXMatrixLookAtLH(D3DXMATRIX *o, const D3DXVECTOR3 *eye, const D3DXVECTOR3 *at, const D3DXVECTOR3 *up) {
    D3DXVECTOR3 zax = *at - *eye, xax, yax;
    D3DXVec3Normalize(&zax, &zax);
    D3DXVec3Cross(&xax, up, &zax); D3DXVec3Normalize(&xax, &xax);
    D3DXVec3Cross(&yax, &zax, &xax);
    o->_11 = xax.x; o->_12 = yax.x; o->_13 = zax.x; o->_14 = 0.0f;
    o->_21 = xax.y; o->_22 = yax.y; o->_23 = zax.y; o->_24 = 0.0f;
    o->_31 = xax.z; o->_32 = yax.z; o->_33 = zax.z; o->_34 = 0.0f;
    o->_41 = -D3DXVec3Dot(&xax, eye); o->_42 = -D3DXVec3Dot(&yax, eye); o->_43 = -D3DXVec3Dot(&zax, eye); o->_44 = 1.0f;
    return o;
}
D3DXINLINE D3DXMATRIX* D3DXMatrixPerspectiveFovLH(D3DXMATRIX *o, FLOAT fovy, FLOAT aspect, FLOAT zn, FLOAT zf) {
    FLOAT ys = 1.0f / tanf(fovy * 0.5f), xs = ys / aspect;
    for (int i = 0; i < 16; i++) (&o->_11)[i] = 0.0f;
    o->_11 = xs; o->_22 = ys; o->_33 = zf / (zf - zn); o->_34 = 1.0f; o->_43 = -zn * zf / (zf - zn);
    return o;
}
D3DXINLINE D3DXMATRIX* D3DXMatrixOrthoLH(D3DXMATRIX *o, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf) {
    for (int i = 0; i < 16; i++) (&o->_11)[i] = 0.0f;
    o->_11 = 2.0f / w; o->_22 = 2.0f / h; o->_33 = 1.0f / (zf - zn); o->_43 = zn / (zn - zf); o->_44 = 1.0f;
    return o;
}

/* ---------------------------------------------------------- Quaternion */
D3DXINLINE D3DXQUATERNION* D3DXQuaternionIdentity(D3DXQUATERNION *o) { o->x = o->y = o->z = 0.0f; o->w = 1.0f; return o; }
D3DXINLINE FLOAT D3DXQuaternionLength(const D3DXQUATERNION *q) { return sqrtf(q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w); }
D3DXINLINE D3DXQUATERNION* D3DXQuaternionMultiply(D3DXQUATERNION *o, const D3DXQUATERNION *a, const D3DXQUATERNION *b) {
    D3DXQUATERNION t(b->w * a->x + b->x * a->w + b->y * a->z - b->z * a->y,
                     b->w * a->y - b->x * a->z + b->y * a->w + b->z * a->x,
                     b->w * a->z + b->x * a->y - b->y * a->x + b->z * a->w,
                     b->w * a->w - b->x * a->x - b->y * a->y - b->z * a->z);
    *o = t; return o;
}
D3DXINLINE D3DXQUATERNION& D3DXQUATERNION::operator*=(const D3DXQUATERNION& q) { D3DXQuaternionMultiply(this, this, &q); return *this; }
D3DXINLINE D3DXQUATERNION* D3DXQuaternionRotationMatrix(D3DXQUATERNION *o, const D3DXMATRIX *mt) {
    FLOAT trace = mt->m[0][0] + mt->m[1][1] + mt->m[2][2], q[4];
    if (trace > 0.0f) {
        FLOAT s = sqrtf(trace + 1.0f);
        q[3] = s * 0.5f; s = 0.5f / s;
        q[0] = (mt->m[1][2] - mt->m[2][1]) * s;
        q[1] = (mt->m[2][0] - mt->m[0][2]) * s;
        q[2] = (mt->m[0][1] - mt->m[1][0]) * s;
    } else {
        int i = 0;
        if (mt->m[1][1] > mt->m[0][0]) i = 1;
        if (mt->m[2][2] > mt->m[i][i]) i = 2;
        int j = (i + 1) % 3, k = (j + 1) % 3;
        FLOAT s = sqrtf(mt->m[i][i] - mt->m[j][j] - mt->m[k][k] + 1.0f);
        q[i] = 0.5f * s; s = 0.5f / s;
        q[3] = (mt->m[j][k] - mt->m[k][j]) * s;
        q[j] = (mt->m[i][j] + mt->m[j][i]) * s;
        q[k] = (mt->m[i][k] + mt->m[k][i]) * s;
    }
    o->x = q[0]; o->y = q[1]; o->z = q[2]; o->w = q[3]; return o;
}
D3DXINLINE D3DXQUATERNION* D3DXQuaternionSlerp(D3DXQUATERNION *o, const D3DXQUATERNION *a, const D3DXQUATERNION *b, FLOAT t) {
    FLOAT dot = a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w, sign = 1.0f;
    if (dot < 0.0f) { dot = -dot; sign = -1.0f; }
    FLOAT s1, s2;
    if (1.0f - dot > 0.0001f) {
        FLOAT th = acosf(dot), sn = sinf(th);
        s1 = sinf((1.0f - t) * th) / sn; s2 = sinf(t * th) / sn;
    } else { s1 = 1.0f - t; s2 = t; }
    s2 *= sign;
    D3DXQUATERNION r(s1 * a->x + s2 * b->x, s1 * a->y + s2 * b->y, s1 * a->z + s2 * b->z, s1 * a->w + s2 * b->w);
    *o = r; return o;
}

/* --------------------------------------------------------------- Plane */
D3DXINLINE D3DXPLANE* D3DXPlaneFromPointNormal(D3DXPLANE *o, const D3DXVECTOR3 *p, const D3DXVECTOR3 *n) {
    o->a = n->x; o->b = n->y; o->c = n->z; o->d = -D3DXVec3Dot(p, n); return o;
}
D3DXINLINE D3DXPLANE* D3DXPlaneFromPoints(D3DXPLANE *o, const D3DXVECTOR3 *v1, const D3DXVECTOR3 *v2, const D3DXVECTOR3 *v3) {
    D3DXVECTOR3 e1 = *v2 - *v1, e2 = *v3 - *v1, n;
    D3DXVec3Cross(&n, &e1, &e2); D3DXVec3Normalize(&n, &n);
    return D3DXPlaneFromPointNormal(o, v1, &n);
}

#endif /* D3DX8MATH_SHIM_H */
