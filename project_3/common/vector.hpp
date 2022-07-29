#pragma once

#include <cmath>
#include <ostream>

struct Vector {
    double x, y;

    inline Vector() : x(0.0f), y(0.0f) {}
    inline Vector(double x, double y) : x(x), y(y) {}
    inline Vector(double angle) : x(cos(angle)), y(sin(angle)) {}

    inline double lengthSqr() const {
        return x * x + y * y;
    }

    inline double length() const {
        return sqrt(lengthSqr());
    }

    inline Vector operator +(const Vector other) const {
        return { x + other.x, y + other.y };
    }

    inline Vector operator -(const Vector other) const {
        return { x - other.x, y - other.y };
    }

    inline Vector operator /(double a) const {
        return { x / a, y / a };
    }

    inline Vector operator *(double a) const {
        return { x * a, y * a };
    }

    bool operator ==(const Vector other) const {
        return x == other.x && y == other.y;
    }

    bool operator !=(const Vector other) const {
        return x != other.x || y != other.y;
    }

    friend std::ostream& operator <<(std::ostream& os, Vector vector);
};

struct vector_hash
{
    std::size_t operator()(const Vector& v) const
    {
      using std::size_t;
      using std::hash;

      return (hash<double>()(v.x) ^ hash<double>()(v.y));
    }
};

inline std::ostream& operator <<(std::ostream& os, Vector vector) {
    os << '(' << vector.x << ", " << vector.y << ')';
    return os;
}

inline double orientation(const Vector a, const Vector b, const Vector c) {
    // Uitgeschreven en gemeenschappelijke factoren samengenomen uit die determinant formule.
    // De mogelijke floating point fout kan geen invloed hebben op de correctheid.
    return a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y);
}
