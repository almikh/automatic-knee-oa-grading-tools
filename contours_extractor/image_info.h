#pragma once
#include "image.h"

namespace xr
{
    namespace info
    {
      /* ограничивает ли все изображение рамка цвета framecolor */
      bool hasFrame(const Image& image, uint8_t frame_color = 0);

      /* €вл€етс€ ли изображение 'пустым', т.е. закрашено только цветом backColor */
      bool isEmpty(const Image& image, uint8_t back_color = 0);

      /* количество смежных с (х, у) пикселей такого же цвета*/
      int neighborsNumber(const Image& image, int x, int y);

      /* дл€ бинарного изображени€: пиксели белого цвета, смежные с (x, y) */
      std::vector<point_t> neighborsPixel(const Image& image, int x, int y);

      /* дл€ бинарного изображени€: индекс смещени€ дл€ пикселей белого цвета (ip::dx, ip::dy), смежных с (x, y) */
      std::vector<int> neighborsPixelIndices(const Image& image, int x, int y);

      /* €вл€етс€ ли смежным дл€ (х, у) указанный цвет color */
      bool isAdjacentColor(const Image& image, int x, int y, uint8_t color);

      /* кол-во смежных с (х, у) пикселей указанного цвета color */
      int countAdjacentColor(const Image& image, int x, int y, uint8_t color);

      /* €вл€етс€ ли смежным дл€ (х, у) указанный цвет (4-смежн.) */
      bool isAdjacentColor4w(const Image& image, int x, int y, uint8_t color);

      /* число св€занных с указ. пикселем пикселей того же цвета */
      int numberOfAssociatedPixels(const Image& image, int x, int y, Connectivity way = Connectivity::Eight, int limit = Int::max());
    }
}
