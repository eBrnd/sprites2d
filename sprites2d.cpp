#include <sstream>
#include <array>
#include <iostream>

struct RGBColor {
  unsigned char r, g, b;

  RGBColor& operator+=(const RGBColor& o) {
    r = r + o.r > 255 ? 255 : r + o.r;
    g = g + o.g > 255 ? 255 : g + o.g;
    b = b + o.b > 255 ? 255 : b + o.b;
    return *this;
  }

  RGBColor& operator*=(const float f) {
    r = r * f;
    g = g * f;
    b = b * f;
    return *this;
  }

  friend RGBColor operator*(const RGBColor& c, float f) {
    RGBColor res = c;
    res *= f;
    return res;
  }
};

struct Pixel {
  RGBColor color;
};

static const int WIDTH = 12;
static const int HEIGHT = 9;

class Alma {
  std::array<std::array<Pixel, WIDTH>, HEIGHT> pixels;

public:
  Alma() {
    for (auto& row : pixels)
      for (auto& p : row)
        p = {{ 0, 0, 0 }};
  }

  std::string serialize() const {
    std::ostringstream oss;
    int index = 0;
    for (auto& row : pixels)
      for (auto& p : row) {
        oss << index++ << " : " << p.color.r << "\n";
        oss << index++ << " : " << p.color.g << "\n";
        oss << index++ << " : " << p.color.b << "\n";
      }
    oss << "\n";
    return oss.str();
  }
};

int main(int, char**) {
  Alma a;
  std::cout << a.serialize();
  return 0;
}
