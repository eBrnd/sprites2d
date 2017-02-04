#include <array>
#include <chrono>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <thread>

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

struct HSVColor {
  public:
    HSVColor(float h, float s, float v) : s(s), v(v) {
      while (h > 360)
        h -= 360;
      this->h = h;
    }

    RGBColor to_rgb() const {
      const int hi = h / 60;
      const float f = h / 60 - hi;
      const float p = v * (1 - s);
      const float q = v * (1 - s * f);
      const float t = v * (1 - s * (1 - f));

      float r, g, b;
      switch (hi) {
        case 0: case 6: r = v; g = t; b = p; break;
        case 1:         r = q; g = v; b = p; break;
        case 2:         r = p; g = v; b = t; break;
        case 3:         r = p; g = q; b = v; break;
        case 4:         r = t; g = p; b = v; break;
        case 5:         r = v; g = p; b = q; break;
        default:
          throw std::range_error("hue out of range");
      }

      return RGBColor{
        static_cast<unsigned char>(r * 255),
        static_cast<unsigned char>(g * 255),
        static_cast<unsigned char>(b * 255)
      };
    }

    float& value() { return v; }

  private:
    float h, s, v;
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
    clear();
  }

  void clear() {
    for (auto& row : pixels)
      for (auto& p : row)
        p = {{ 0, 0, 0 }};
  }


  std::string serialize() const {
    std::ostringstream oss;
    int index = 0;
    for (auto& row : pixels)
      for (auto& p : row) {
        oss << index++ << " : " << (int)p.color.r << "\n";
        oss << index++ << " : " << (int)p.color.g << "\n";
        oss << index++ << " : " << (int)p.color.b << "\n";
      }
    oss << "\n";
    return oss.str();
  }

  void put(int x, int y, const RGBColor& color) {
    if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT)
      return;

    pixels[y][x].color += color;
  }
};

class Sprite {
  public:
    virtual void render(Alma& a) const = 0;
    virtual bool update() = 0;
};

class Drop : public Sprite {
  private:
    int x, y;
    float radius;
    RGBColor c;

  public:
    Drop(int x, int y, const HSVColor c)
      : x(x), y(y), radius(0), c(c.to_rgb()) { }

    virtual void render(Alma& a) const {
      int inside = (int)radius;
      float interp = (radius - inside);
      // Draw border
      for (int xp = -inside - 1; xp <= inside + 1; xp++)
        for (int yp = -inside - 1; yp <= inside + 1; yp++)
          a.put(x + xp, y + yp, c * interp);

      // Draw inside
      for (int xp = -inside; xp <= inside; xp++)
        for (int yp = -inside; yp <= inside; yp++)
          a.put(x + xp, y + yp, c * (1 - interp));
    }

    virtual bool update() {
      radius += 0.08;
      if (radius > 3)
        c *= 0.9;

      return radius < 6;
    }
};

int main(int, char**) {
  Alma a;
  std::list<std::shared_ptr<Sprite>> sprites;
  sprites.push_back(std::shared_ptr<Sprite>(new Drop(3, 4, { 100, 1, 0.8 })));
  sprites.push_back(std::shared_ptr<Sprite>(new Drop(5, 0, { 200, 1, 0.8 })));
  sprites.push_back(std::shared_ptr<Sprite>(new Drop(8, 8, { 300, 1, 0.8 })));

  for (;;) {
    auto start = std::chrono::steady_clock::now();

    for (auto& s : sprites)
      s->render(a);

    for (auto spr_it = sprites.begin(); spr_it != sprites.end(); ) {
      if ((*spr_it)->update())
        spr_it++;
      else
        spr_it = sprites.erase(spr_it);
    }

    std::cout << a.serialize();
    a.clear();

    std::cerr << "Rendering " << sprites.size() << " sprites took " <<
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start
          ).count()
        << "ms." << std::endl; // Frame " << FC << std::endl;
    std::this_thread::sleep_until(start + std::chrono::milliseconds(40));
  }

  return 0;
}
