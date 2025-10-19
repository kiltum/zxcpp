#include "ula.hpp"
#include <cstring>
#include <algorithm>
#include <iostream>

// Constructor
ULA::ULA(Memory *mem) : memory(mem)
{

    // Allocate screen buffer (352x288 to accommodate borders)
    screenBuffer = new uint32_t[352 * 288];

    // Initialize state variables
    clock = 0;
    line = 0;
    flash = false;
    flashCnt = 0;
    frameCnt = 0;
    borderColor = 9;
    horClock = 0;

    colors[0] = 0xFF000000;  // Black
    colors[1] = 0xFF0000C0;  // Blue
    colors[2] = 0xFFC00000;  // Red
    colors[3] = 0xFFC000C0;  // Magenta
    colors[4] = 0xFF00C000;  // Green
    colors[5] = 0xFF00C0C0;  // Cyan
    colors[6] = 0xFFC0C000;  // Yellow
    colors[7] = 0xFFC0C0C0;  // White
    colors[8] = 0xFF000000;  // Black (bright)
    colors[9] = 0xFF0000FF;  // Bright Blue
    colors[10] = 0xFFFF0000; // Bright Red
    colors[11] = 0xFFFF00FF; // Bright Magenta
    colors[12] = 0xFF00FF00; // Bright Green
    colors[13] = 0xFFFFFF00; // Bright Cyan
    colors[14] = 0xFF00FFFF; // Bright Yellow
    colors[15] = 0xFFFFFFFF; // Bright White

    // Initialize pixel mapping tables
    initializePixelMapping();

    for (int i = 0; i < 352 * 288; i++)
    {
        screenBuffer[i] = colors[0];
    }
}

// Destructor
ULA::~ULA()
{
    delete[] screenBuffer;
}

// Initialize pixel mapping tables
void ULA::initializePixelMapping()
{
    for (int i = 0; i < 224; i++)
    {
        pixelMapStart[i] = static_cast<int>(static_cast<float>(i) * 320.0f / 224.0f);
        pixelMapEnd[i] = static_cast<int>(static_cast<float>(i + 1) * 320.0f / 224.0f);
    }
}

// Check if this handler can handle the specified port
bool ULA::canHandlePort(uint16_t port)
{
    // ULA typically handles port 0xFE for border color and keyboard input
    return (port & 0xFF) == 0xFE;
}

// Read a byte from the specified port
uint8_t ULA::readPort(uint16_t port)
{
    // ULA handles port 0xFE for keyboard input and other functions
    if ((port & 0xFF) == 0xFE)
    {
        // For now, return all keys released (bits set to 1)
        // We'll implement proper keyboard handling later
        return 0xFF;
    }

    return 0;
}

// Write a byte to the specified port
void ULA::writePort(uint16_t port, uint8_t value)
{
    if ((port & 0xFF) == 0xFE)
    {
        borderColor = value & 0x07;
        // Sound output handling (we'll implement this later)
        // For now, just ignore sound output to avoid port access issues
    }
}

// Get screen buffer
uint32_t *ULA::getScreenBuffer()
{
    return screenBuffer;
}

// 3560 - flyback
// *----------------------------* 48*224 = 10752
// *   V48                      *
// *  +----------------------*  * 3560 + 10752 + 24 = 14336 till first byte
// *  !V192                  !  *
// *24!     H128             !24* - 48 retrace = 224
// *  !                      !  * 192 * 224 = 43008
// *  +----------------------+  * 14336 + 43008 = 57344 till right corner
// *   v48                      * 48 * 224 = 10752
// *----------------------------* 68072 on bottom right corner
// 1816 overscan.
// 3560 + 10752 + 43008 + 10752 + 1816 = 69888
// Visible Width = (24+128+24) * 2 = 352
// Visible Height = (48+192+48) = 288

// Process a single ULA tick
// Returns internal clock state. 
int ULA::oneTick()
{
    // Process one clock tick
    clock++;
    int delay =0;

    if (clock <= 3560)
    { // we are on flyback
        return clock;
    }

    if (clock > 68072 && clock < 69888)
    { // we are on over scan
        return clock;
    }

    // Check if we've completed a frame
    if (clock >= 69888)
    {
        // Reset counters for next frame
        clock = 0;
        line = 0;

        // Increment frame counter for flash timing
        frameCnt++;

        // Handle flash counter (every 16 frames)
        if (frameCnt >= 16)
        {
            flash = !flash;
            flashCnt = (flashCnt + 1) & 0x0F;
            frameCnt = 0;
        }
        return 0;
    }

    line = (clock - 3560) / 224;

    // now lets draw screen
    if (horClock <= 24)
    { // beam on left border
        drawPixel(borderColor);
    }
    if (horClock > 23 && horClock <= (24 + 128))
    {
        if (line <= 47 || line > (192 + 48)) // its up border, still no need to access screen
        {
            drawPixel(borderColor);
        }
        else
        {
            int x = (horClock - 24) * 2;
            int y = line - 48;
            //printf("%d %d\n", x, y);
            // screenBuffer[(y + 48) * 352 + x + 48] = getPixelColorFast(x, y);
            // screenBuffer[(y + 48) * 352 + x + 48 + 1] = getPixelColorFast(x + 1, y);
            screenBuffer[(y + 48) * 352 + x + 48] = colors[rand() % 15];
            screenBuffer[(y + 48) * 352 + x + 48 + 1] = colors[rand() % 15];
        }
    }

    if (horClock > (24 + 128) && horClock <= (24 + 128 + 24))
    { // beam on right border
        drawPixel(borderColor);
    }

    horClock++;
    if (horClock > 223)
    { // beam end line and return back
        horClock = 0;
        // printf("---- %d %d \n", clock, clock-3560);
    }

    // printf("%d %d %d \n", clock, horClock, line);
    return clock;
}

// Draw the current pixel
void ULA::drawPixel(int color)
{
    // printf("%d %d \n", line, horClock);
    //  One tick = 2 pixel to draw
    screenBuffer[line * 352 + (horClock * 2 - 1)] = colors[color];
    screenBuffer[line * 352 + horClock * 2] = colors[color];
}

// Get pixel color based on ZX Spectrum video memory layout
uint32_t ULA::getPixelColorFast(uint8_t x, uint8_t y)
{
    // ZX Spectrum video memory layout:
    // - Screen bitmap: 0x4000-0x57FF (6144 bytes)
    // - Attributes: 0x5800-0x5AFF (768 bytes)

    // Calculate bitmap address
    // Formula: 0x4000 + (y & 0xC0) << 5 + (y & 0x07) << 8 + (y & 0x38) << 2 + (x >> 3)
    uint16_t addr = 0x4000 + ((y & 0xC0) << 5) + ((y & 0x07) << 8) + ((y & 0x38) << 2) + (x >> 3);

    // Read bitmap byte
    uint8_t bitmap = memory->ReadByte(addr);

    // Calculate attribute address
    // Formula: 0x5800 + (y >> 3) * 32 + (x >> 3)
    uint16_t attrAddr = 0x5800 + ((y >> 3) * 32) + (x >> 3);

    // Read attribute byte
    // Bits 0-2: Ink color
    // Bits 3-5: Paper color
    // Bit 6: Bright flag
    // Bit 7: Flash flag
    uint8_t attr = memory->ReadByte(attrAddr);

    // Get bit value for this pixel
    uint8_t bit = 0x80 >> (x & 0x07);

    // Determine ink and paper colors
    uint8_t ink, paper;
    if ((attr & 0x80) != 0 && flash)
    {                             // Flash bit set and flash active
        ink = (attr >> 3) & 0x07; // Paper color as ink
        paper = attr & 0x07;      // Ink color as paper
    }
    else
    {
        ink = attr & 0x07;          // Normal ink color
        paper = (attr >> 3) & 0x07; // Normal paper color
    }

    // Apply bright flag
    if ((attr & 0x40) != 0)
    {
        ink |= 0x08;
        paper |= 0x08;
    }

    // Select color based on bit value
    uint8_t colorIndex;
    if ((bitmap & bit) != 0)
    {
        colorIndex = ink;
    }
    else
    {
        colorIndex = paper;
    }

    // Return pre-calculated color
    return colors[colorIndex];
}

// Reset ULA state
void ULA::reset()
{
    clock = 0;
    line = 0;
    flash = false;
    flashCnt = 0;
    frameCnt = 0;
    borderColor = 0;

    // Reinitialize screen with black border
    uint32_t blackColor = colors[0];
    for (int i = 0; i < 320 * 240; i++)
    {
        screenBuffer[i] = blackColor;
    }
}
