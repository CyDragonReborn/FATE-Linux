#include "d3dx8_shim.h"
/*
 * d3dx8_shim.cpp -- texture / surface half of the D3DX8 replacement.
 * Works on the real D3D8 API (Wine/DXVK/native D3D8), uses stb_image for decoding.
 * Pixel work is done on RGBA8 buffers and packed into the destination format.
 */
#include "d3dx8.h"
#include "d3dx8math.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "third_party/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "third_party/stb_image_write.h"

typedef unsigned char u8;
typedef std::vector<u8> Buf;

/* ------------------------------------------------------------ formats */
static int FmtBytes(D3DFORMAT f)
{
    switch (f) {
    case D3DFMT_A8R8G8B8: case D3DFMT_X8R8G8B8: return 4;
    case D3DFMT_R5G6B5: case D3DFMT_A1R5G5B5: case D3DFMT_X1R5G5B5:
    case D3DFMT_A4R4G4B4: case D3DFMT_X4R4G4B4: case D3DFMT_A8L8: return 2;
    case D3DFMT_L8: case D3DFMT_A8: return 1;
    default: return 0;
    }
}
static inline u8 Ex5(unsigned v) { return (u8)((v << 3) | (v >> 2)); }
static inline u8 Ex6(unsigned v) { return (u8)((v << 2) | (v >> 4)); }
static inline u8 Ex4(unsigned v) { return (u8)(v * 17); }
static inline unsigned Q5(u8 v) { return (v * 31u + 127u) / 255u; }
static inline unsigned Q6(u8 v) { return (v * 63u + 127u) / 255u; }
static inline unsigned Q4(u8 v) { return (v * 15u + 127u) / 255u; }
static inline u8 Lum(const u8 *c) { return (u8)((c[0] * 77 + c[1] * 150 + c[2] * 29) >> 8); }

static void Unpack(D3DFORMAT f, const u8 *p, u8 *o /*rgba*/)
{
    unsigned v;
    switch (f) {
    case D3DFMT_A8R8G8B8: o[0] = p[2]; o[1] = p[1]; o[2] = p[0]; o[3] = p[3]; break;
    case D3DFMT_X8R8G8B8: o[0] = p[2]; o[1] = p[1]; o[2] = p[0]; o[3] = 255; break;
    case D3DFMT_R5G6B5: v = p[0] | (p[1] << 8); o[0] = Ex5(v >> 11); o[1] = Ex6((v >> 5) & 63); o[2] = Ex5(v & 31); o[3] = 255; break;
    case D3DFMT_A1R5G5B5: case D3DFMT_X1R5G5B5:
        v = p[0] | (p[1] << 8); o[0] = Ex5((v >> 10) & 31); o[1] = Ex5((v >> 5) & 31); o[2] = Ex5(v & 31);
        o[3] = (f == D3DFMT_A1R5G5B5 && !(v & 0x8000)) ? 0 : 255; break;
    case D3DFMT_A4R4G4B4: case D3DFMT_X4R4G4B4:
        v = p[0] | (p[1] << 8); o[0] = Ex4((v >> 8) & 15); o[1] = Ex4((v >> 4) & 15); o[2] = Ex4(v & 15);
        o[3] = (f == D3DFMT_A4R4G4B4) ? Ex4(v >> 12) : 255; break;
    case D3DFMT_L8: o[0] = o[1] = o[2] = p[0]; o[3] = 255; break;
    case D3DFMT_A8L8: o[0] = o[1] = o[2] = p[0]; o[3] = p[1]; break;
    case D3DFMT_A8: o[0] = o[1] = o[2] = 0; o[3] = p[0]; break;
    default: o[0] = o[1] = o[2] = o[3] = 0; break;
    }
}
static void Pack(D3DFORMAT f, u8 *p, const u8 *c /*rgba*/)
{
    unsigned v;
    switch (f) {
    case D3DFMT_A8R8G8B8: p[0] = c[2]; p[1] = c[1]; p[2] = c[0]; p[3] = c[3]; break;
    case D3DFMT_X8R8G8B8: p[0] = c[2]; p[1] = c[1]; p[2] = c[0]; p[3] = 255; break;
    case D3DFMT_R5G6B5: v = (Q5(c[0]) << 11) | (Q6(c[1]) << 5) | Q5(c[2]); p[0] = (u8)v; p[1] = (u8)(v >> 8); break;
    case D3DFMT_A1R5G5B5: case D3DFMT_X1R5G5B5:
        v = ((f == D3DFMT_X1R5G5B5 || c[3] >= 128) ? 0x8000u : 0u) | (Q5(c[0]) << 10) | (Q5(c[1]) << 5) | Q5(c[2]);
        p[0] = (u8)v; p[1] = (u8)(v >> 8); break;
    case D3DFMT_A4R4G4B4: case D3DFMT_X4R4G4B4:
        v = ((f == D3DFMT_A4R4G4B4 ? Q4(c[3]) : 15u) << 12) | (Q4(c[0]) << 8) | (Q4(c[1]) << 4) | Q4(c[2]);
        p[0] = (u8)v; p[1] = (u8)(v >> 8); break;
    case D3DFMT_L8: p[0] = Lum(c); break;
    case D3DFMT_A8L8: p[0] = Lum(c); p[1] = c[3]; break;
    case D3DFMT_A8: p[0] = c[3]; break;
    default: break;
    }
}

/* ---------------------------------------------------------- resampling */
static void Resample(const u8 *src, int sw, int sh, u8 *dst, int dw, int dh, DWORD filter)
{
    DWORD f = filter & 7;
    if (sw == dw && sh == dh) { memcpy(dst, src, (size_t)sw * sh * 4); return; }
    for (int y = 0; y < dh; y++) {
        for (int x = 0; x < dw; x++) {
            u8 *o = dst + ((size_t)y * dw + x) * 4;
            if (f == D3DX_FILTER_POINT || f == D3DX_FILTER_NONE) {
                int sx = (int)(((x + 0.5f) * sw) / dw), sy = (int)(((y + 0.5f) * sh) / dh);
                if (sx >= sw) sx = sw - 1; if (sy >= sh) sy = sh - 1;
                memcpy(o, src + ((size_t)sy * sw + sx) * 4, 4);
            } else if (f == D3DX_FILTER_BOX || (dw < sw && dh < sh)) {
                int x0 = (int)((long long)x * sw / dw), x1 = (int)(((long long)(x + 1) * sw + dw - 1) / dw);
                int y0 = (int)((long long)y * sh / dh), y1 = (int)(((long long)(y + 1) * sh + dh - 1) / dh);
                if (x1 <= x0) x1 = x0 + 1; if (y1 <= y0) y1 = y0 + 1; if (x1 > sw) x1 = sw; if (y1 > sh) y1 = sh;
                unsigned long long acc[4] = {0, 0, 0, 0}; int n = 0;
                for (int yy = y0; yy < y1; yy++) for (int xx = x0; xx < x1; xx++) {
                    const u8 *s = src + ((size_t)yy * sw + xx) * 4;
                    acc[0] += s[0]; acc[1] += s[1]; acc[2] += s[2]; acc[3] += s[3]; n++;
                }
                for (int k = 0; k < 4; k++) o[k] = (u8)((acc[k] + n / 2) / n);
            } else { /* bilinear, texel-centre aligned */
                float fx = (x + 0.5f) * sw / dw - 0.5f, fy = (y + 0.5f) * sh / dh - 0.5f;
                int x0 = (int)floorf(fx), y0 = (int)floorf(fy); float tx = fx - x0, ty = fy - y0;
                int x1 = x0 + 1, y1 = y0 + 1;
                if (x0 < 0) x0 = 0; if (y0 < 0) y0 = 0; if (x1 >= sw) x1 = sw - 1; if (y1 >= sh) y1 = sh - 1;
                if (x0 >= sw) x0 = sw - 1; if (y0 >= sh) y0 = sh - 1;
                for (int k = 0; k < 4; k++) {
                    float a = src[((size_t)y0 * sw + x0) * 4 + k] * (1 - tx) + src[((size_t)y0 * sw + x1) * 4 + k] * tx;
                    float b = src[((size_t)y1 * sw + x0) * 4 + k] * (1 - tx) + src[((size_t)y1 * sw + x1) * 4 + k] * tx;
                    float v = a * (1 - ty) + b * ty;
                    o[k] = (u8)(v + 0.5f);
                }
            }
        }
    }
}

/* -------------------------------------------------------- device helpers */
static void AdjustSize(LPDIRECT3DDEVICE8 dev, UINT *w, UINT *h)
{
    D3DCAPS8 caps;
    if (FAILED(dev->GetDeviceCaps(&caps))) return;
    if ((caps.TextureCaps & D3DPTEXTURECAPS_POW2) && !(caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)) {
        UINT pw = 1, ph = 1;
        while (pw < *w) pw <<= 1;
        while (ph < *h) ph <<= 1;
        *w = pw; *h = ph;
    }
    if (caps.MaxTextureWidth && *w > caps.MaxTextureWidth) *w = caps.MaxTextureWidth;
    if (caps.MaxTextureHeight && *h > caps.MaxTextureHeight) *h = caps.MaxTextureHeight;
}
static UINT FullChain(UINT w, UINT h) { UINT n = 1; while (w > 1 || h > 1) { w = w > 1 ? w / 2 : 1; h = h > 1 ? h / 2 : 1; n++; } return n; }

/* Write an RGBA8 image into a surface (resampled to the target rect). */
static HRESULT WriteSurface(LPDIRECT3DSURFACE8 dst, const RECT *dstRect, const u8 *rgba, int sw, int sh, DWORD filter)
{
    D3DSURFACE_DESC d; HRESULT hr = dst->GetDesc(&d); if (FAILED(hr)) return hr;
    int bpp = FmtBytes(d.Format); if (!bpp) return D3DERR_INVALIDCALL;
    RECT r = { 0, 0, (LONG)d.Width, (LONG)d.Height }; if (dstRect) r = *dstRect;
    int dw = r.right - r.left, dh = r.bottom - r.top; if (dw <= 0 || dh <= 0) return D3DERR_INVALIDCALL;
    Buf tmp((size_t)dw * dh * 4); Resample(rgba, sw, sh, &tmp[0], dw, dh, filter);

    LPDIRECT3DSURFACE8 target = dst; LPDIRECT3DSURFACE8 staging = NULL; RECT lockRect = r;
    D3DLOCKED_RECT lr;
    hr = dst->LockRect(&lr, &r, 0);
    if (FAILED(hr)) { /* default-pool target: fill an image surface and CopyRects */
        LPDIRECT3DDEVICE8 dev = NULL; if (FAILED(dst->GetDevice(&dev))) return hr;
        hr = dev->CreateImageSurface(dw, dh, d.Format, &staging);
        if (FAILED(hr)) { dev->Release(); return hr; }
        lockRect.left = 0; lockRect.top = 0; lockRect.right = dw; lockRect.bottom = dh;
        hr = staging->LockRect(&lr, &lockRect, 0);
        if (FAILED(hr)) { staging->Release(); dev->Release(); return hr; }
        target = staging;
        for (int y = 0; y < dh; y++) for (int x = 0; x < dw; x++)
            Pack(d.Format, (u8*)lr.pBits + (size_t)y * lr.Pitch + (size_t)x * bpp, &tmp[((size_t)y * dw + x) * 4]);
        staging->UnlockRect();
        POINT pt = { r.left, r.top }; RECT srcR = { 0, 0, dw, dh };
        hr = dev->CopyRects(staging, &srcR, 1, dst, &pt);
        staging->Release(); dev->Release();
        return hr;
    }
    for (int y = 0; y < dh; y++) for (int x = 0; x < dw; x++)
        Pack(d.Format, (u8*)lr.pBits + (size_t)y * lr.Pitch + (size_t)x * bpp, &tmp[((size_t)y * dw + x) * 4]);
    target->UnlockRect();
    return D3D_OK;
}

/* Read a surface (or rect of it) into an RGBA8 buffer. */
static HRESULT ReadSurface(LPDIRECT3DSURFACE8 src, const RECT *srcRect, Buf &out, int &w, int &h)
{
    D3DSURFACE_DESC d; HRESULT hr = src->GetDesc(&d); if (FAILED(hr)) return hr;
    int bpp = FmtBytes(d.Format); if (!bpp) return D3DERR_INVALIDCALL;
    RECT r = { 0, 0, (LONG)d.Width, (LONG)d.Height }; if (srcRect) r = *srcRect;
    w = r.right - r.left; h = r.bottom - r.top; if (w <= 0 || h <= 0) return D3DERR_INVALIDCALL;
    out.resize((size_t)w * h * 4);
    D3DLOCKED_RECT lr; LPDIRECT3DSURFACE8 staging = NULL; LPDIRECT3DSURFACE8 rd = src; RECT lockR = r;
    hr = src->LockRect(&lr, &r, D3DLOCK_READONLY);
    if (FAILED(hr)) { /* render target etc.: copy to an image surface first */
        LPDIRECT3DDEVICE8 dev = NULL; if (FAILED(src->GetDevice(&dev))) return hr;
        hr = dev->CreateImageSurface(w, h, d.Format, &staging);
        if (FAILED(hr)) { dev->Release(); return hr; }
        hr = dev->CopyRects(src, &r, 1, staging, NULL); dev->Release();
        if (FAILED(hr)) { staging->Release(); return hr; }
        lockR.left = 0; lockR.top = 0; lockR.right = w; lockR.bottom = h;
        hr = staging->LockRect(&lr, &lockR, D3DLOCK_READONLY);
        if (FAILED(hr)) { staging->Release(); return hr; }
        rd = staging;
    }
    for (int y = 0; y < h; y++) for (int x = 0; x < w; x++)
        Unpack(d.Format, (const u8*)lr.pBits + (size_t)y * lr.Pitch + (size_t)x * bpp, &out[((size_t)y * w + x) * 4]);
    rd->UnlockRect();
    if (staging) staging->Release();
    return D3D_OK;
}

/* ------------------------------------------------------------- public API */
HRESULT D3DXLoadSurfaceFromSurface(LPDIRECT3DSURFACE8 pDest, CONST PALETTEENTRY*, CONST RECT *pDestRect,
                                   LPDIRECT3DSURFACE8 pSrc, CONST PALETTEENTRY*, CONST RECT *pSrcRect,
                                   DWORD Filter, D3DCOLOR ColorKey)
{
    if (!pDest || !pSrc) return D3DERR_INVALIDCALL;
    Buf img; int w, h; HRESULT hr = ReadSurface(pSrc, pSrcRect, img, w, h); if (FAILED(hr)) return hr;
    if (ColorKey) for (size_t i = 0; i < (size_t)w * h; i++) {
        u8 *p = &img[i * 4];
        if ((DWORD)((p[0] << 16) | (p[1] << 8) | p[2]) == (ColorKey & 0x00ffffff)) p[0] = p[1] = p[2] = p[3] = 0;
    }
    if (Filter == D3DX_DEFAULT) Filter = D3DX_FILTER_TRIANGLE;
    return WriteSurface(pDest, pDestRect, &img[0], w, h, Filter);
}

HRESULT D3DXCreateTexture(LPDIRECT3DDEVICE8 dev, UINT w, UINT h, UINT mips, DWORD usage, D3DFORMAT fmt, D3DPOOL pool,
                          LPDIRECT3DTEXTURE8 *ppTex)
{
    if (!dev || !ppTex) return D3DERR_INVALIDCALL;
    if (w == 0 || w == D3DX_DEFAULT) w = 256;
    if (h == 0 || h == D3DX_DEFAULT) h = 256;
    if (fmt == D3DFMT_UNKNOWN || fmt == (D3DFORMAT)D3DX_DEFAULT) fmt = D3DFMT_A8R8G8B8;
    if (mips == D3DX_DEFAULT) mips = 0;
    AdjustSize(dev, &w, &h);
    return dev->CreateTexture(w, h, mips, usage, fmt, pool, ppTex);
}

HRESULT D3DXCreateCubeTexture(LPDIRECT3DDEVICE8 dev, UINT size, UINT mips, DWORD usage, D3DFORMAT fmt, D3DPOOL pool,
                              LPDIRECT3DCUBETEXTURE8 *ppCube)
{
    if (!dev || !ppCube) return D3DERR_INVALIDCALL;
    if (size == 0 || size == D3DX_DEFAULT) size = 256;
    if (fmt == D3DFMT_UNKNOWN || fmt == (D3DFORMAT)D3DX_DEFAULT) fmt = D3DFMT_A8R8G8B8;
    if (mips == D3DX_DEFAULT) mips = 0;
    UINT w = size, h = size; AdjustSize(dev, &w, &h);
    return dev->CreateCubeTexture(w, mips, usage, fmt, pool, ppCube);
}

static D3DFORMAT FormatFromName(const char* name) { ... }
{
    std::string s(name); size_t dot = s.rfind('.'); std::string e = dot == std::string::npos ? "" : s.substr(dot + 1);
    for (size_t i = 0; i < e.size(); i++) e[i] = (char)tolower((unsigned char)e[i]);
    if (e == "png") return D3DXIFF_PNG; if (e == "jpg" || e == "jpeg") return D3DXIFF_JPG;
    if (e == "tga") return D3DXIFF_TGA; if (e == "dds") return D3DXIFF_DDS; if (e == "ppm") return D3DXIFF_PPM;
    if (e == "dib") return D3DXIFF_DIB; return D3DXIFF_BMP;
}

HRESULT D3DXCreateTextureFromFileEx(LPDIRECT3DDEVICE8 dev, LPCSTR file, UINT w, UINT h, UINT mips, DWORD usage,
                                    D3DFORMAT fmt, D3DPOOL pool, DWORD filter, DWORD mipFilter, D3DCOLOR colorKey,
                                    D3DXIMAGE_INFO *pInfo, PALETTEENTRY*, LPDIRECT3DTEXTURE8 *ppTex)
{
    if (!dev || !file || !ppTex) return D3DERR_INVALIDCALL;
    *ppTex = NULL;
    int iw, ih, comp; u8 *pix = stbi_load(file, &iw, &ih, &comp, 4);
    if (!pix) return D3DERR_INVALIDCALL;
    Buf base(pix, pix + (size_t)iw * ih * 4); stbi_image_free(pix);

    if (pInfo) {
        pInfo->Width = iw; pInfo->Height = ih; pInfo->Depth = 1; pInfo->MipLevels = 1;
        pInfo->Format = (comp == 4 || comp == 2) ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8;
        pInfo->ResourceType = D3DRTYPE_TEXTURE; pInfo->ImageFileFormat = FormatFromName(file);
    }
    if (colorKey) for (size_t i = 0; i < (size_t)iw * ih; i++) {
        u8 *p = &base[i * 4];
        if ((DWORD)((p[0] << 16) | (p[1] << 8) | p[2]) == (colorKey & 0x00ffffff)) p[0] = p[1] = p[2] = p[3] = 0;
    }
    UINT tw = (w == 0 || w == D3DX_DEFAULT) ? (UINT)iw : w, th = (h == 0 || h == D3DX_DEFAULT) ? (UINT)ih : h;
    AdjustSize(dev, &tw, &th);
    if (fmt == D3DFMT_UNKNOWN || fmt == (D3DFORMAT)D3DX_DEFAULT) fmt = D3DFMT_A8R8G8B8;
    if (!FmtBytes(fmt)) fmt = D3DFMT_A8R8G8B8;
    UINT levels = (mips == 0 || mips == D3DX_DEFAULT) ? FullChain(tw, th) : mips;
    if (filter == D3DX_DEFAULT || filter == 0) filter = D3DX_FILTER_TRIANGLE;
    if (mipFilter == D3DX_DEFAULT || mipFilter == 0) mipFilter = D3DX_FILTER_BOX;

    D3DPOOL createPool = (pool == D3DPOOL_DEFAULT) ? D3DPOOL_SYSTEMMEM : pool;
    LPDIRECT3DTEXTURE8 tex = NULL;
    HRESULT hr = dev->CreateTexture(tw, th, levels, (pool == D3DPOOL_DEFAULT) ? 0 : usage, fmt, createPool, &tex);
    if (FAILED(hr)) return hr;

    Buf cur((size_t)tw * th * 4); Resample(&base[0], iw, ih, &cur[0], tw, th, filter);
    UINT cw = tw, ch = th;
    for (UINT lv = 0; lv < tex->GetLevelCount(); lv++) {
        LPDIRECT3DSURFACE8 surf = NULL;
        hr = tex->GetSurfaceLevel(lv, &surf); if (FAILED(hr)) { tex->Release(); return hr; }
        hr = WriteSurface(surf, NULL, &cur[0], cw, ch, D3DX_FILTER_POINT); surf->Release();
        if (FAILED(hr)) { tex->Release(); return hr; }
        if (lv + 1 < tex->GetLevelCount()) {
            UINT nw = cw > 1 ? cw / 2 : 1, nh = ch > 1 ? ch / 2 : 1; Buf nxt((size_t)nw * nh * 4);
            Resample(&cur[0], cw, ch, &nxt[0], nw, nh, mipFilter); cur.swap(nxt); cw = nw; ch = nh;
        }
    }
    if (pInfo) pInfo->MipLevels = tex->GetLevelCount();

    if (pool == D3DPOOL_DEFAULT) {
        LPDIRECT3DTEXTURE8 gpu = NULL;
        hr = dev->CreateTexture(tw, th, levels, usage, fmt, D3DPOOL_DEFAULT, &gpu);
        if (SUCCEEDED(hr)) hr = dev->UpdateTexture(tex, gpu);
        tex->Release();
        if (FAILED(hr)) { if (gpu) gpu->Release(); return hr; }
        tex = gpu;
    }
    *ppTex = tex;
    return D3D_OK;
}

HRESULT D3DXFillTexture(LPDIRECT3DTEXTURE8 tex, LPD3DXFILL2D fn, LPVOID data)
{
    if (!tex || !fn) return D3DERR_INVALIDCALL;
    for (UINT lv = 0; lv < tex->GetLevelCount(); lv++) {
        D3DSURFACE_DESC d; HRESULT hr = tex->GetLevelDesc(lv, &d); if (FAILED(hr)) return hr;
        int bpp = FmtBytes(d.Format); if (!bpp) return D3DERR_INVALIDCALL;
        D3DLOCKED_RECT lr; hr = tex->LockRect(lv, &lr, NULL, 0); if (FAILED(hr)) return hr;
        D3DXVECTOR2 texel(1.0f / d.Width, 1.0f / d.Height);
        for (UINT y = 0; y < d.Height; y++) for (UINT x = 0; x < d.Width; x++) {
            D3DXVECTOR2 uv((x + 0.5f) / d.Width, (y + 0.5f) / d.Height); D3DXVECTOR4 c(0, 0, 0, 0);
            fn(&c, &uv, &texel, data);
            u8 px[4]; const float *cc = &c.x;
            for (int k = 0; k < 4; k++) { float v = cc[k]; px[k] = (u8)(v <= 0 ? 0 : v >= 1 ? 255 : v * 255.0f + 0.5f); }
            Pack(d.Format, (u8*)lr.pBits + (size_t)y * lr.Pitch + (size_t)x * bpp, px);
        }
        tex->UnlockRect(lv);
    }
    return D3D_OK;
}

HRESULT D3DXFilterTexture(LPDIRECT3DBASETEXTURE8 base, CONST PALETTEENTRY*, UINT srcLevel, DWORD filter)
{
    if (!base) return D3DERR_INVALIDCALL;
    if (filter == D3DX_DEFAULT || filter == 0) filter = D3DX_FILTER_BOX;
    UINT n = base->GetLevelCount(); HRESULT hr = D3D_OK;
    if (base->GetType() == D3DRTYPE_TEXTURE) {
        LPDIRECT3DTEXTURE8 t = (LPDIRECT3DTEXTURE8)base;
        for (UINT lv = srcLevel + 1; lv < n && SUCCEEDED(hr); lv++) {
            LPDIRECT3DSURFACE8 s = NULL, d = NULL;
            if (FAILED(hr = t->GetSurfaceLevel(lv - 1, &s))) break;
            if (FAILED(hr = t->GetSurfaceLevel(lv, &d))) { s->Release(); break; }
            hr = D3DXLoadSurfaceFromSurface(d, NULL, NULL, s, NULL, NULL, filter, 0); s->Release(); d->Release();
        }
    } else if (base->GetType() == D3DRTYPE_CUBETEXTURE) {
        LPDIRECT3DCUBETEXTURE8 t = (LPDIRECT3DCUBETEXTURE8)base;
        for (int face = 0; face < 6 && SUCCEEDED(hr); face++)
            for (UINT lv = srcLevel + 1; lv < n && SUCCEEDED(hr); lv++) {
                LPDIRECT3DSURFACE8 s = NULL, d = NULL;
                if (FAILED(hr = t->GetCubeMapSurface((D3DCUBEMAP_FACES)face, lv - 1, &s))) break;
                if (FAILED(hr = t->GetCubeMapSurface((D3DCUBEMAP_FACES)face, lv, &d))) { s->Release(); break; }
                hr = D3DXLoadSurfaceFromSurface(d, NULL, NULL, s, NULL, NULL, filter, 0); s->Release(); d->Release();
            }
    } else return D3DERR_INVALIDCALL;
    return hr;
}

/* ------------------------------------------------------------- saving */
static HRESULT SaveRGBA(const char *file, D3DXIMAGE_FILEFORMAT fmt, const u8 *rgba, int w, int h)
{
    int ok = 0;
    switch (fmt) {
    case D3DXIFF_PNG: ok = stbi_write_png(file, w, h, 4, rgba, w * 4); break;
    case D3DXIFF_JPG: ok = stbi_write_jpg(file, w, h, 4, rgba, 90); break;
    case D3DXIFF_TGA: ok = stbi_write_tga(file, w, h, 4, rgba); break;
    case D3DXIFF_DDS: {
        FILE *f = fopen(file, "wb"); if (!f) return D3DERR_INVALIDCALL;
        DWORD hdr[32] = { 0 };
        hdr[0] = 0x20534444; hdr[1] = 124; hdr[2] = 0x100F; hdr[3] = h; hdr[4] = w; hdr[5] = w * 4;
        hdr[19] = 32; hdr[20] = 0x41; hdr[22] = 32; hdr[23] = 0x00FF0000; hdr[24] = 0x0000FF00; hdr[25] = 0x000000FF;
        hdr[26] = 0xFF000000; hdr[27] = 0x1000;
        fwrite(hdr, 4, 32, f);
        for (int i = 0; i < w * h; i++) { u8 p[4] = { rgba[i*4+2], rgba[i*4+1], rgba[i*4+0], rgba[i*4+3] }; fwrite(p, 1, 4, f); }
        fclose(f); ok = 1; break; }
    default: ok = stbi_write_bmp(file, w, h, 4, rgba); break;
    }
    return ok ? D3D_OK : D3DERR_INVALIDCALL;
}
HRESULT D3DXSaveSurfaceToFile(LPCSTR file, D3DXIMAGE_FILEFORMAT fmt, LPDIRECT3DSURFACE8 surf, CONST PALETTEENTRY*, CONST RECT *rc)
{
    if (!file || !surf) return D3DERR_INVALIDCALL;
    Buf img; int w, h; HRESULT hr = ReadSurface(surf, rc, img, w, h); if (FAILED(hr)) return hr;
    return SaveRGBA(file, fmt, &img[0], w, h);
}
HRESULT D3DXSaveTextureToFile(LPCSTR file, D3DXIMAGE_FILEFORMAT fmt, LPDIRECT3DBASETEXTURE8 base, CONST PALETTEENTRY*)
{
    if (!file || !base || base->GetType() != D3DRTYPE_TEXTURE) return D3DERR_INVALIDCALL;
    LPDIRECT3DSURFACE8 s = NULL; HRESULT hr = ((LPDIRECT3DTEXTURE8)base)->GetSurfaceLevel(0, &s); if (FAILED(hr)) return hr;
    hr = D3DXSaveSurfaceToFile(file, fmt, s, NULL, NULL); s->Release(); return hr;
}

/* ------------------------------------------------------------- stubs */
HRESULT D3DXCreateCubeTextureFromFileEx(LPDIRECT3DDEVICE8, LPCSTR, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, DWORD, DWORD,
                                        D3DCOLOR, D3DXIMAGE_INFO*, PALETTEENTRY*, LPDIRECT3DCUBETEXTURE8 *pp)
{ if (pp) *pp = NULL; return D3DERR_NOTAVAILABLE; /* TODO: needs the game's cubemap usage */ }
HRESULT D3DXCreateMeshFVF(DWORD, DWORD, DWORD, DWORD, LPDIRECT3DDEVICE8, LPD3DXMESH *pp)
{ if (pp) *pp = NULL; return D3DERR_NOTAVAILABLE; /* TODO */ }
HRESULT D3DXCreateRenderToSurface(LPDIRECT3DDEVICE8, UINT, UINT, D3DFORMAT, BOOL, D3DFORMAT, LPD3DXRENDERTOSURFACE *pp)
{ if (pp) *pp = NULL; return D3DERR_NOTAVAILABLE; /* TODO */ }
HRESULT D3DXCreateRenderToEnvMap(LPDIRECT3DDEVICE8, UINT, D3DFORMAT, BOOL, D3DFORMAT, LPD3DXRENDERTOENVMAP *pp)
{ if (pp) *pp = NULL; return D3DERR_NOTAVAILABLE; /* TODO */ }
HRESULT D3DXComputeTangent(LPD3DXMESH, DWORD, LPD3DXMESH, DWORD, DWORD, DWORD, DWORD*)
{ return D3DERR_NOTAVAILABLE; /* TODO */ }
