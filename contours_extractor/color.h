#pragma once
#include <stdint.h>

namespace xr
{
  struct Color {
    uint8_t r, g, b;

    Color(int color = 0x0);
    Color(uint8_t r, uint8_t g, uint8_t b);

    Color& operator = (const Color& src);
    bool operator == (const Color& src);
    bool operator != (const Color& src);

    uint8_t toGray() const {
      return static_cast<uint8_t>(r*0.114 + g*0.587 + b*0.299);
    }

    static Color red;
    static Color green;
    static Color blue;
    static Color white;
    static Color black;
    static Color yellow;
  };
}
