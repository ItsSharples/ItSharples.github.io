#include "pch.h"
#include "main.h"
#include "MyMaths.h"
#include "LoadOBJ.h"
#include <array>
#include "MyBVH.h"
// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
namespace
{

// Standard library C-style
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>

#define ESC "\x1b"
#define CSI "\x1b["


    using Vec3f = MyMaths::Vec3f;
    using Shape = MyMaths::Shape;

    using Mesh = MyMaths::Mesh;
    using Sphere = MyMaths::Sphere;


    
    //HDC hdc;
    HWND hwnd_;

    constexpr unsigned border = 5;
    constexpr unsigned x_offset = 0;
    constexpr unsigned y_offset = 5;
    constexpr const unsigned numChunks = 16;
    constexpr const unsigned size = 16;
    //constexpr unsigned width = 3840, height = 2160;
    //constexpr unsigned width = 1920, height = 1080;
    //constexpr unsigned width = 1080, height = 607;
    constexpr unsigned width = 900, height = 507;
    //constexpr unsigned width = 853, height = 480;
    //constexpr unsigned width = 128, height = 96;
    //constexpr unsigned width = 16, height = 9;

    constexpr unsigned int totalPixels = width * height; //*2; // Mul 2 because we're doing two blocks
    constexpr float invWidth = 1 / float(width), invHeight = 1 / float(height);
    constexpr float fov = 90, aspectratio = width / float(height);
    static const float angle = tan(M_PI * 0.5 * fov / 180.);
    std::atomic<unsigned> completedPixels = 0;
    VOID* imageBVH;
    VOID* image;

    HBITMAP bitmapOLD;
    HBITMAP bitmapBVH;
    HDC OLDDC;
    HDC BVHDC;

    constexpr const unsigned chunk_width = width / size;
    constexpr const unsigned chunk_height = height / size;

    constexpr const unsigned int timePerFrame = 1000;//milliseconds
    constexpr const unsigned int timePerChunk = timePerFrame / numChunks / 2; // Div 2 because we're doing two blocks BVH and OLD

    enum State : int
    {
        Building = 0x0000,
        Rendering = 0x0001,
        DrawnOld = 0x0010,
        DrawnBVH = 0x0100,
        Done = 0x1000
    };
    State myState = State::Building;

    static COLORREF Vec3fToCol(const Vec3f& col)
    {
        // It says RGB, but I put the values in as BGR, shhhhh
        return RGB(
            (unsigned char)(std::min(float(1), col.z) * 255),
            (unsigned char)(std::min(float(1), col.y) * 255),
            (unsigned char)(std::min(float(1), col.x) * 255));
    }

    static BOOL BlitToDC(HDC hdc)
    {


        HDC       hDCBits;
        BITMAP    Bitmap;
        BOOL      bResult;

        if (!hdc || !bitmapOLD)
            return FALSE;

        hDCBits = CreateCompatibleDC(hdc);
        GetObject(bitmapOLD, sizeof(BITMAP), (LPSTR)&Bitmap);
        SelectObject(hDCBits, bitmapOLD);
        bResult = BitBlt(hdc, border, border, Bitmap.bmWidth, Bitmap.bmHeight, hDCBits, 0, 0, SRCCOPY);
        DeleteDC(hDCBits);

        hDCBits = CreateCompatibleDC(hdc);
        GetObject(bitmapBVH, sizeof(BITMAP), (LPSTR)&Bitmap);
        SelectObject(hDCBits, bitmapBVH);
        bResult = BitBlt(hdc, border, height + border * 2 + y_offset, Bitmap.bmWidth, Bitmap.bmHeight, hDCBits, 0, 0, SRCCOPY);
        DeleteDC(hDCBits);

        return bResult;
    }


    BITMAPINFO bmpInfo;
}

void errhandler(const char* message)
{
    MessageBoxA(hwnd_, message, "Error", MB_OK | MB_ICONERROR);
}
/// <note>
/// Yoinked from https://docs.microsoft.com/en-us/windows/win32/gdi/storing-an-image
/// </note>
void CreateBMPFile(LPTSTR filename, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC, void* image)
{
    HANDLE hf;                 // file handle  
    BITMAPFILEHEADER hdr;       // bitmap file-header  
    PBITMAPINFOHEADER pbih;     // bitmap info-header  
    //LPBYTE lpBits;              // memory pointer  
    DWORD dwTotal;              // total count of bytes  
    DWORD cb;                   // incremental count of bytes  
    //BYTE* hp;                   // byte pointer  
    DWORD dwTmp;

    pbih = (PBITMAPINFOHEADER)pbi;
    //lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

    //if (!lpBits)
    //    errhandler("GlobalAlloc");

    // Retrieve the color table (RGBQUAD array) and the bits  
    // (array of palette indices) from the DIB.  
    //if (!GetDIBits(hDC, hBMP, 0, height, image, pbi,
    //    DIB_RGB_COLORS))
    //{
    //    errhandler("GetDIBits");
    //}

    // Create the .BMP file.  
    hf = CreateFile(filename,
        GENERIC_READ | GENERIC_WRITE,
        (DWORD)0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        (HANDLE)NULL);
    if (hf == INVALID_HANDLE_VALUE)
        errhandler("CreateFile");
    hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
    // Compute the size of the entire file.  
    hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage);
    hdr.bfReserved1 = 0;
    hdr.bfReserved2 = 0;

    // Compute the offset to the array of color indices.  
    hdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) +
        pbih->biSize + pbih->biClrUsed
        * sizeof(RGBQUAD);

    // Copy the BITMAPFILEHEADER into the .BMP file.  
    if (!WriteFile(hf, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER),
        (LPDWORD)&dwTmp, NULL))
    {
        errhandler("WriteFile");
    }

    // Copy the BITMAPINFOHEADER and RGBQUAD array into the file.  
    if (!WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER)
        + pbih->biClrUsed * sizeof(RGBQUAD),
        (LPDWORD)&dwTmp, (NULL)))
        errhandler("WriteFile");

    // Copy the array of color indices into the .BMP file.  
    dwTotal = cb = pbih->biSizeImage;

    if (!WriteFile(hf, (LPSTR)image, (int)cb, (LPDWORD)&dwTmp, NULL))
        errhandler("WriteFile");

    // Close the .BMP file.  
    if (!CloseHandle(hf))
        errhandler("CloseHandle");
}

void saveImage(HBITMAP hBMP, HDC hDC)
{
    CreateBMPFile(
        LPTSTR(L"Old Ray Tracer.bmp"),
        &bmpInfo,
        hBMP, hDC,
        image);

    CreateBMPFile(
        LPTSTR(L"New Ray Tracer.bmp"),
        &bmpInfo,
        hBMP, hDC,
        imageBVH);
}

Vec3f traceOld(const Vec3f& rayorig, const Vec3f& raydir, const std::vector<Shape*>& shapes, const int& depth)
{
    float tnear = INFINITY;
    const Shape* shape = NULL;
    // find closest shape this ray passes through
    for (unsigned i = 0; i < shapes.size(); ++i) {
        float t0 = INFINITY, t1 = INFINITY;
        if (shapes[i]->intersect(rayorig, raydir, t0, t1)) {
            if (t0 < 0) t0 = t1;
            if (t0 < tnear) {
                tnear = t0;
                shape = shapes[i];
            }
        }
    }
    // if there's no intersection return black or background color
    if (!shape) return MyMaths::Colours::CornflowerBlue;

    // Start Working out Colour and Such
    Vec3f surfaceColor = 0; // color of the ray/surface of the object intersected by the ray 
    Vec3f phit = rayorig + raydir * tnear; // point of intersection (origin + direction * distance)
    Vec3f nhit = phit - shape->position; // normal at the intersection point 
    nhit.normalize(); // normalize normal direction 

    // If the normal and the view direction are not opposite to each other reverse the normal direction. 
    // This also means we are inside the sphere

    float bias = 1e-4; // add some bias to the point from which we will be tracing 
    bool inside = false;
    if (raydir.dot(nhit) > 0) nhit = -nhit, inside = true;
    bool hasTransparency = shape->transparency > 0;
    if ((hasTransparency || shape->reflection > 0) && depth < MyMaths::MAX_RAY_DEPTH) {
        float facingratio = -raydir.dot(nhit);
        // change the mix value to tweak the effect
        float fresneleffect = MyMaths::mix(pow(1.f - facingratio, 3), 1.f, 0.1f);
        // compute reflection direction (vectors are already normalized)
        // dir - 2 * (dir dot normal) *  normal is reflection
        Vec3f refldir = raydir + nhit * 2.f * facingratio;
        refldir.normalize();
        Vec3f reflection = traceOld(phit + nhit * bias, refldir, shapes, depth + 1);
        Vec3f refraction = 0;
        // if the sphere is also transparent compute refraction ray (transmission)
        if (hasTransparency) {
            float ior = 1.1f, eta = (inside) ? ior : 1.f / ior; // are we inside or outside the surface? 
            float cosi = -nhit.dot(raydir);
            float k = 1.f - eta * eta * (1.f - cosi * cosi);
            Vec3f refrdir = raydir * eta + nhit * (eta * cosi - sqrt(k));
            refrdir.normalize();
            refraction = traceOld(phit - nhit * bias, refrdir, shapes, depth + 1);
        }
        // the result is a mix of reflection and refraction (if the sphere is transparent)
        surfaceColor = (
            reflection * fresneleffect +
            refraction * (1.f - fresneleffect) * shape->transparency) * shape->surfaceColour;
    }
    // It's a diffuse object, no need to raytrace any further
    else {
        // Find any Lights that hit this Shape
        for (unsigned i = 0; i < shapes.size(); ++i) {
            // this is a light
            if (shapes[i]->emissionColour > 0.f) {
                Vec3f transmission = 1;
                Vec3f lightDirection = shapes[i]->position - phit;
                lightDirection.normalize();
                for (unsigned j = 0; j < shapes.size(); ++j) {
                    if (i != j) {
                        float t0, t1;
                        if (shapes[j]->intersect(phit + nhit * bias, lightDirection, t0, t1)) {
                            transmission = 0;
                            break;
                        }
                    }
                }
                surfaceColor += shape->surfaceColour * transmission *
                    std::max(float(0), nhit.dot(lightDirection)) * shapes[i]->emissionColour;
            }
        }
    }

    return surfaceColor + shape->emissionColour;
}
Vec3f traceBVH(const Vec3f& rayorig, const Vec3f& raydir, const MyBVH& shapes, const int& depth)
{
    return shapes.intersect(rayorig, raydir);    
}

void renderOld(const std::vector<Shape*>& shapes)
{
    auto line = std::vector<std::future<void>>();
    for (std::atomic<unsigned> y = 0; y < chunk_height; ++y)
    {
        for (std::atomic<unsigned> x = 0; x < chunk_width; ++x)
        {
            line.push_back(std::async(std::launch::async, [&](unsigned chunk_x, unsigned chunk_y)
                {
                    const unsigned start_x = chunk_x * size;
                    const unsigned start_y = chunk_y * size;
                    for (unsigned y = start_y; y < start_y + size; ++y)
                    {
                        for (unsigned x = start_x; x < start_x + size; ++x)
                        {
                            auto index = y * width + x;
                            if (index >= totalPixels)
                            {
                                continue;
                            }

                            float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
                            float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
                            Vec3f raydir(xx, yy, -1);
                            raydir.normalize();

                            auto future = std::async(std::launch::async, traceOld, Vec3f(0), raydir, shapes, 0);
                            auto val = future.get();
                            
                            ((COLORREF *)image)[index] = Vec3fToCol(val);
                            //Vec3fToCol(val);
                            //completedPixels++;
                        }
                    }
                }, x.load(), y.load()));
        }        
    }

    for (auto i = line.size() - 1; i > 0; i--)
    {
        if (line[i].wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
        {
            line.erase(line.begin() + i);
        }
    }
    return;
}
void renderBVH(const MyBVH& bvh)
{
    auto line = std::vector<std::future<void>>();
    for (std::atomic<unsigned> y = 0; y < chunk_height; ++y)
    {
        for (std::atomic<unsigned> x = 0; x < chunk_width; ++x)
        {
            line.push_back(std::async(std::launch::async, [&](unsigned chunk_x, unsigned chunk_y)
                {
                    const unsigned start_x = chunk_x * size;
                    const unsigned start_y = chunk_y * size;
                    for (unsigned y = start_y; y < start_y + size; ++y)
                    {
                        for (unsigned x = start_x; x < start_x + size; ++x)
                        {
                            auto index = y * width + x;
                            if (index >= totalPixels)
                            {
                                continue;
                            }

                            float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
                            float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
                            Vec3f raydir(xx, yy, -1);
                            raydir.normalize();

                            auto future = std::async(std::launch::async, traceBVH, Vec3f(0), raydir, bvh, 0);
                            //auto val = future.get();
                            ((COLORREF*)imageBVH)[index] = Vec3fToCol(future.get()); // bvh.intersect(Vec3f(0), raydir); ;
                            completedPixels++;
                        }
                    }
                }, x.load(), y.load()));
        }
    }

    for (auto i = line.size() - 1; i > 0; i--)
    {
        if (line[i].wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
        {
            line.erase(line.begin() + i);
        }
    }
    return;
}

void renderOldChunk(const std::vector<Shape*>& shapes, const MyMaths::Bounds& chunk)
{
    typedef std::atomic<unsigned> AtomicIndex;
    for (unsigned y = chunk.min.y; y < chunk.max.y; y++)
    {
        for (unsigned x = chunk.min.x; x < chunk.max.x; x++)
        {
            auto index = y * width + x;
            if (index >= totalPixels)
            {
                continue;
            }

            float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
            float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
            Vec3f raydir(xx, yy, -1);
            raydir.normalize();

            //auto future = std::async(std::launch::async, traceOld, Vec3f(0), raydir, shapes, 0);
            //auto val = future.get();

            const auto val = traceOld(Vec3f(0), raydir, shapes, 0);
            ((COLORREF*)image)[index] = Vec3fToCol(val);
            completedPixels++;
        }
    }
    return;
}
void renderBVHChunk(const MyBVH& bvh, MyMaths::Bounds chunk)
{
    typedef std::atomic<unsigned> AtomicIndex;
    for (unsigned y = chunk.min.y; y < chunk.max.y; y++)
    {
        for (unsigned x = chunk.min.x; x < chunk.max.x; x++)
        {
            auto index = y * width + x;
            if (index >= totalPixels)
            {
                continue;
            }

            float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
            float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
            Vec3f raydir(xx, yy, -1);
            raydir.normalize();

            //auto future = std::async(std::launch::async, );
            //auto val = future.get();
            const auto val = traceBVH(Vec3f(0), raydir, bvh, 0);
            ((COLORREF*)imageBVH)[index] = Vec3fToCol(val);
            //completedPixels++;
        }
    }
    return;
}

std::vector<Shape*> buildShapes()
{
    std::vector<Shape*> shapes;
    //srand(13);
    // position, radius, surface color, reflectivity, transparency, emission color
    shapes.push_back(new Sphere(Vec3f(0.0f, -10004.f, -20.f), 10000.f, Vec3f(0.20f, 0.20f, 0.20f), 0, 0.f));
    shapes.push_back(new Sphere(Vec3f(0.0f, 0.f, -20.f), 4.f, Vec3f(1.00f, 0.32f, 0.36f), 1.f, 0.5f));
    shapes.push_back(new Sphere(Vec3f(5.0f, -1.f, -15.f), 2.f, Vec3f(0.90f, 0.76f, 0.46f), 1.f, 0.0f));
    shapes.push_back(new Sphere(Vec3f(5.0f, 0.f, -25.f), 3.f, Vec3f(0.65f, 0.77f, 0.97f), 1.f, 0.0f));
    shapes.push_back(new Sphere(Vec3f(-5.5f, 0.f, -15.f), 3.f, Vec3f(0.90f, 0.90f, 0.90f), 1.f, 0.0f));

    // A nice Red and 50% transparent
    const MyMaths::OBJ& obj = DXL::loadOBJ("snowman");
    for (size_t index = 0; index < 10; index++)
    {
        auto x = 5 * (rand() % 5);
        auto z = 5 * (rand() % 5);

        auto mesh = new Mesh(Vec3f(0.f, -1.f, 0.f), MyMaths::Colours::NiceRed, 1.f, 1.f);
        mesh->addOBJ(obj);
        shapes.push_back(mesh);
    }
    // light
    shapes.push_back(new Sphere(Vec3f(0.0f, 20.f, -30.f), 3.f, Vec3f(0.f), 0.1f, 0.0f, Vec3f(3.f)));

    return shapes;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    //case WM_MOVE:
    case WM_PAINT:
    {
        std::cout << "Paint" << std::endl;
        PAINTSTRUCT ps;
        auto hdc = BeginPaint(hwnd, &ps);

        BlitToDC(hdc);

        EndPaint(hwnd, &ps);
        return 0L;
    }
    case WM_DESTROY:
    {
        ExitGame();
        return 0;
    }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}
void updateWnd(_In_ MSG& msg)
{
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    };
}
void ExitGame(void) noexcept
{
    PostQuitMessage(0);
}

//int __cdecl main(void)
//{
//    std::vector<Shape*> shapes;
//    buildShapes(shapes);
//
//    tmp_image = render(shapes);
//    saveImage(tmp_image);
//}

//FILE* stream;
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{

    if (AllocConsole()) {
#pragma warning(suppress : 4996)
        if (freopen("CONOUT$", "w", stdout) == NULL) { return errno; };
        SetConsoleTitle(L"Debug Console");

        // Set output mode to handle virtual terminal sequences
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut == INVALID_HANDLE_VALUE)
        {
            return GetLastError();
        }

        DWORD dwMode = 0;
        if (!GetConsoleMode(hOut, &dwMode))
        {
            return GetLastError();
        }

        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        if (!SetConsoleMode(hOut, dwMode))
        {
            return GetLastError();
        }

        // Enter the alternate buffer
        //printf(CSI "?1049h");
    }



    const wchar_t CLASS_NAME[] = L"RayTracing Window Class";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"RayTracing Window",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position

        CW_USEDEFAULT, CW_USEDEFAULT, width + (border * 2) + x_offset + 15, (height + border * 2) * 2 + y_offset + 35,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );
    hwnd_ = hwnd;
    ShowWindow(hwnd, SW_SHOW);
    //UpdateWindow(hwnd);

// Create BitMaps to Draw onto

    // Create DIB Data
    bmpInfo = {
      {
        sizeof(BITMAPINFOHEADER),
        width, // width
        -(LONG)height, // height
        1, // planes
        32, // bit size
        BI_RGB,
        totalPixels * 4, // size image
        96, // DPI X
        96, // DPI Y
        0, // clr used
        0 // clr important 
      }
    };
    // Create DIB
    auto hdc = GetDC(hwnd_);
    bitmapBVH = CreateDIBSection(hdc, &bmpInfo, DIB_RGB_COLORS, &imageBVH, NULL, 0x0);
    bitmapOLD = CreateDIBSection(hdc, &bmpInfo, DIB_RGB_COLORS, &image, NULL, 0x0);
    ReleaseDC(hwnd_, hdc);
    // Timestamp before Work is done
    auto lastTime = std::chrono::system_clock::now();
    // Build everything
    std::vector<Shape*> shapes;
    myState = State::Building;
    printf("Building\n");
    shapes = buildShapes();
    printf("Built\n");
    myState = State::Rendering;
    printf("BVH: Adding Shapes\n");
    MyBVH bvh;
    for (auto& shape : shapes)
    {
        bvh.addShape(shape);
    }
    printf("BVH: Shapes Added\n");
    // Set up Chunks to Render
    std::array<std::future<void>, numChunks> chunkOLDDoing;
    std::array<std::future<void>, numChunks> chunkBVHDoing;
    std::vector<Bounds> chunksToDo;

    //Bounds currentChunk = {};
    unsigned int x_iter = 0;
    unsigned int y_iter = 0;

    printf("Generating Chunks\n");
    while ((chunk_width * (x_iter + 1)) < width)
    {
        y_iter = 0;
        while ((chunk_height * (y_iter + 1)) < height)
        {
            chunksToDo.emplace_back(
                Vec3f(chunk_width *  x_iter     , chunk_height * y_iter      , 0.f),
                Vec3f(chunk_width * (x_iter + 1), chunk_height * (y_iter + 1), 0.f)
            );
            y_iter++;
        }
        chunksToDo.emplace_back(
            Vec3f(chunk_width* (x_iter), chunk_height* (y_iter), 0.f),
            Vec3f(chunk_width* (x_iter + 1), height, 0.f));

        x_iter++;

        printf("Another X Chunk\n");
    }
    // Add final Column
    {
        y_iter = 0;
        while ((chunk_height * (y_iter + 1)) < height)
        {
            chunksToDo.emplace_back(
                Vec3f(chunk_width * x_iter, chunk_height * y_iter, 0.f),
                Vec3f(width, chunk_height * (y_iter + 1), 0.f)
            );
            y_iter++;
        }
        chunksToDo.emplace_back(
            Vec3f(chunk_width * (x_iter), chunk_height * (y_iter), 0.f),
            Vec3f(width, height, 0.f));
    }


    printf("X left %f, Y left %f\n(x) %d\t(y) %d\n", (width/ chunk_width), (height/ chunk_height), x_iter, y_iter);

    printf("Chunks Generated\n");

    std::atomic<size_t> nextChunkOLDIter = 0;//;
    std::atomic<size_t> nextChunkBVHIter = 0;//;
    const size_t lastIter = chunksToDo.size();

    bool OLDDone = false;
    bool BVHDone = false;

    // Start the First load of Chunks
    for (int i = 0; i < numChunks; i++)
    {
        if (nextChunkOLDIter >= lastIter)
        //if (nextChunkForOLD++ == chunksToDo.end())
        {
            OLDDone = true;
            break;// No More Chunks Left
        }
        chunkOLDDoing[i] = std::async(std::launch::async, renderOldChunk, shapes, chunksToDo[nextChunkOLDIter++]);//*nextChunkForOLD);
    }
    for (int i = 0; i < numChunks; i++)
    {
        if (nextChunkBVHIter >= lastIter)
        {
            BVHDone = true;
            break;// No More Chunks Left
        }
        chunkBVHDoing[i] = std::async(std::launch::async, renderBVHChunk, bvh, chunksToDo[nextChunkBVHIter++]);//*nextChunkForOLD);
    }

    //// Main message loop
    MSG msg = {};
    // Message Data
    auto nextUpdate = 0;
    unsigned lastPixelsLeft, pixelsLeft = totalPixels;
    auto thisTime = lastTime;
    auto firstTime = thisTime;

    bool clearedYet = false;
    bool saved = false;
    // Stop Blinking the Cursor
    printf(CSI "?12l");
    printf(CSI "?25l");
    // Clear Screen
    printf(CSI "2J");
    while (WM_QUIT != msg.message)
    {
        updateWnd(msg);


        if (!clearedYet)
        {
            // Clear Screen
            printf(CSI "2J");
            clearedYet = true;
        }

        // Move back to x=1,y=1
        printf(CSI "1;1H");
        if (myState == State::Rendering)
        {
            clearedYet = false;
            // Status Text
            if (1)
            {
                lastTime = thisTime;
                thisTime = std::chrono::system_clock::now();
                std::chrono::duration<double> deltaTime = (thisTime - lastTime);
                double delta = deltaTime.count();

                lastPixelsLeft = pixelsLeft;
                pixelsLeft = totalPixels - completedPixels;
                auto deltaPixels = (lastPixelsLeft - pixelsLeft);
                auto pixelPerTime = deltaPixels / delta;

                float percentageDone = (completedPixels * 100.f) / (float)totalPixels;

                if (pixelsLeft == 0)
                {
                    myState = State::Done;
                    continue;
                }

                
                printf(
                    "----- Useful Data ----\
                    \nPercentage Done: %02.2f%%\
                    \n%d Pixels left to Trace\
                    \n%08d Pixels Done in %08.2fms\
                    \n%8.2f Pixels per second\n",
                    percentageDone, pixelsLeft, deltaPixels, (delta * 1000.f), pixelPerTime);
                
            }
            if (1)
            {
                printf("----- DO CHUNKS ----\n");
                if (OLDDone)
                {
                    myState = State::Done;
                }
                else
                {
                    bool canStop = true;
                    for (int i = 0; i < numChunks; i++)
                    {
                        if (chunkOLDDoing[i].valid())
                        {
                            if (chunkOLDDoing[i].wait_for(std::chrono::milliseconds(timePerChunk)) == std::future_status::ready)
                            {
                                if (nextChunkOLDIter >= lastIter)
                                //if (nextChunkForOLD++ == chunksToDo.end())
                                {
                                    continue;// No More Chunks Left
                                }
                                chunkOLDDoing[i].get();// Get the Value
                                chunkOLDDoing[i] = std::async(std::launch::async, renderOldChunk, shapes, chunksToDo[nextChunkOLDIter++]);//*nextChunkForOLD);
                            }
                            canStop = false;
                        }
                    }
                    for (int i = 0; i < numChunks; i++)
                    {
                        if (chunkBVHDoing[i].valid())
                        {
                            if (chunkBVHDoing[i].wait_for(std::chrono::milliseconds(timePerChunk)) == std::future_status::ready)
                            {
                                if (nextChunkBVHIter >= lastIter)
                                    //if (nextChunkForOLD++ == chunksToDo.end())
                                {
                                    continue;// No More Chunks Left
                                }
                                chunkBVHDoing[i].get();// Get the Value
                                chunkBVHDoing[i] = std::async(std::launch::async, renderBVHChunk, bvh, chunksToDo[nextChunkBVHIter++]);//*nextChunkForOLD);
                            }
                            canStop = false;
                        }
                    }
                    if (canStop)
                    {
                        OLDDone = true;
                        continue;
                    }
                    //if (pixelsLeft <= 0)
                    //{
                    //    printf("AAA\nAAA\nAAA\nAAA\nAAA\nAAA\nAAA\nAAA\nAAA\nAAA\n");
                    //    OLDDone = true;
                    //    chunksToDo.clear();
                    //    continue;
                    //}
                }
            }
            if (1)
            {
                printf("----- Draw Pixels ----\n");
                // The Main Drawing Loop
                auto hdc = GetDC(hwnd_);
                BlitToDC(hdc);
                ReleaseDC(hwnd_, hdc);
            }
        }
        else
        {
            
            std::chrono::duration<double> deltaTime = (thisTime - firstTime);
            double delta = deltaTime.count();
            auto pixelPerTime = totalPixels / delta;
            printf(
                "----- Useful Data ----\
                \nPercentage Done: 100%%\
                \n0 Pixels left to Trace\
                \n%08d Pixels Done in %08.2fms\
                \n%8.2f Pixels per second\
                \n\n\n", // Clear up the Other Data
                totalPixels, (delta * 1000.f), pixelPerTime);
            

            auto hdc = GetDC(hwnd_);
            BlitToDC(hdc);
            

            if (!saved)
            {
                saveImage(bitmapOLD, hdc);
                saved = true;
            }
            ReleaseDC(hwnd_, hdc);
        }
    }

    return 0;
}