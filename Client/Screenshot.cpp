#include "Screenshot.h"
#include <windows.h>
#include <vector>
#include <cstdint>

#pragma pack(push,1)
struct BMPFileHeader {
    uint16_t bfType{ 0x4D42 };        
    uint32_t bfSize{ 0 };           
    uint16_t bfReserved1{ 0 };
    uint16_t bfReserved2{ 0 };
    uint32_t bfOffBits{ 0 };         
};
#pragma pack(pop)

std::vector<BYTE> Screenshot::Capture() {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, hBitmap);
    BitBlt(memDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);

    BITMAPINFOHEADER bih{};
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = w;
    bih.biHeight = h;
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biCompression = BI_RGB;
    int rowSize = ((w * 3 + 3) & ~3);
    int imageSize = rowSize * h;
    std::vector<BYTE> imgData(imageSize);
    GetDIBits(hdc, hBitmap, 0, h, imgData.data(), (BITMAPINFO*)&bih, DIB_RGB_COLORS);

    BMPFileHeader bfh;
    bfh.bfOffBits = sizeof(BMPFileHeader) + sizeof(BITMAPINFOHEADER);
    bfh.bfSize = bfh.bfOffBits + imageSize;

    std::vector<BYTE> bmp;
    bmp.resize(bfh.bfSize);
    memcpy(bmp.data(), &bfh, sizeof(bfh));
    memcpy(bmp.data() + sizeof(bfh), &bih, sizeof(bih));
    memcpy(bmp.data() + bfh.bfOffBits, imgData.data(), imageSize);


    DeleteObject(hBitmap);
    DeleteDC(memDC);
    ReleaseDC(NULL, hdc);

    return bmp;
}