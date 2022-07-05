#include "color.h"

namespace xr
{
  Color Color::red = Color(0xff0000);
  Color Color::green = Color(0x00ff00);
  Color Color::blue = Color(0x0000ff);
  Color Color::white = Color(0xffffff);
  Color Color::black = Color(0x0);
  Color Color::yellow = Color(0xffff00);

  Color::Color(int color) :
    b(color & 255),
    g((color >> 8) & 255),
    r((color >> 16) & 255) 
  {

  }

  Color::Color(uint8_t red, uint8_t green, uint8_t blue) :
    r(red),
    g(green),
    b(blue) 
  {

  }

  Color& Color::operator = (const Color& src) {
    r = src.r;
    g = src.g;
    b = src.b;
    return *this;
  }

  bool Color::operator == (const Color& src) {
    return src.r == r && src.g == g && src.b == b;
  }

  bool Color::operator != (const Color& src) {
    return src.r != r || src.g != g || src.b != b;
  }
}
