#pragma once
#include "image.h"

namespace xr
{
    namespace info
    {
      /* ������������ �� ��� ����������� ����� ����� framecolor */
      bool hasFrame(const Image& image, uint8_t frame_color = 0);

      /* �������� �� ����������� '������', �.�. ��������� ������ ������ backColor */
      bool isEmpty(const Image& image, uint8_t back_color = 0);

      /* ���������� ������� � (�, �) �������� ������ �� �����*/
      int neighborsNumber(const Image& image, int x, int y);

      /* ��� ��������� �����������: ������� ������ �����, ������� � (x, y) */
      std::vector<point_t> neighborsPixel(const Image& image, int x, int y);

      /* ��� ��������� �����������: ������ �������� ��� �������� ������ ����� (ip::dx, ip::dy), ������� � (x, y) */
      std::vector<int> neighborsPixelIndices(const Image& image, int x, int y);

      /* �������� �� ������� ��� (�, �) ��������� ���� color */
      bool isAdjacentColor(const Image& image, int x, int y, uint8_t color);

      /* ���-�� ������� � (�, �) �������� ���������� ����� color */
      int countAdjacentColor(const Image& image, int x, int y, uint8_t color);

      /* �������� �� ������� ��� (�, �) ��������� ���� (4-�����.) */
      bool isAdjacentColor4w(const Image& image, int x, int y, uint8_t color);

      /* ����� ��������� � ����. �������� �������� ���� �� ����� */
      int numberOfAssociatedPixels(const Image& image, int x, int y, Connectivity way = Connectivity::Eight, int limit = Int::max());
    }
}
