#pragma once

namespace xr
{
  struct Size {
    int width, height;

    Size(int width = 0, int height = 0): 
      width(width), 
      height(height) 
    {
    
    }

    bool operator==(const Size& other) {
      return width == other.width && height == other.height;
    }

    Size& operator * (double val) {
      height = static_cast<int>(height * val);
      width = static_cast<int>(width * val);
      return *this;
    }
  };
}
