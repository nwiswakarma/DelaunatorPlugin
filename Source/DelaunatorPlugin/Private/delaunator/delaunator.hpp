#pragma once

#ifdef DELAUNATOR_HEADER_ONLY
#define INLINE inline
#else
#define INLINE
#endif

#include "CoreMinimal.h"

namespace delaunator
{

typedef float FReal;
typedef int32 FIndex;

constexpr FIndex INVALID_INDEX =
    //(std::numeric_limits<FIndex>::max)();
    -1;

constexpr FReal EPSILON = KINDA_SMALL_NUMBER;

FReal sum(const TArray<FReal>& x);

FIndex fast_mod(const FIndex i, const FIndex c);

FReal dist(
    const FReal ax,
    const FReal ay,
    const FReal bx,
    const FReal by
    );

bool orient(
    const FReal px,
    const FReal py,
    const FReal qx,
    const FReal qy,
    const FReal rx,
    const FReal ry
    );

FReal circumradius(
    const FReal ax,
    const FReal ay,
    const FReal bx,
    const FReal by,
    const FReal cx,
    const FReal cy
    );

void circumcenter(
    FReal& centerx,
    FReal& centery,
    const FReal ax,
    const FReal ay,
    const FReal bx,
    const FReal by,
    const FReal cx,
    const FReal cy
    );

bool in_circle(
    const FReal ax,
    const FReal ay,
    const FReal bx,
    const FReal by,
    const FReal cx,
    const FReal cy,
    const FReal px,
    const FReal py
    );

bool check_pts_equal(FReal x1, FReal y1, FReal x2, FReal y2);

FReal pseudo_angle(const FReal dx, const FReal dy);

struct compare
{
    TArrayView<const FReal> coords;
    TArray<FReal> dists;

    compare(
        const TArrayView<const FReal>& in_coords,
        FReal centerx,
        FReal centery
        );

    bool operator()(FIndex i, FIndex j) const;
};

class Delaunator
{

public:
    TArrayView<const FReal> coords;

    TArray<FIndex> triangles;
    TArray<FIndex> halfedges;
    TArray<FIndex> hull_prev;
    TArray<FIndex> hull_next;
    TArray<FIndex> hull_tri;
    FIndex hull_start;
    FIndex hull_size;

    INLINE Delaunator();
    INLINE Delaunator(const FReal* in_coords, FIndex in_coords_num);
    INLINE void update();
    void update(const TArray<FVector2D>& InPoints);
    FReal get_hull_area() const;

private:
    TArray<FIndex> m_hash;
    FReal m_centerx;
    FReal m_centery;
    FIndex m_hash_size;
    TArray<FIndex> m_edge_stack;

    FIndex hash_key(FReal x, FReal y) const;
    void link(FIndex a, FIndex b);
    FIndex add_triangle(
        FIndex i0,
        FIndex i1,
        FIndex i2,
        FIndex a,
        FIndex b,
        FIndex c);

    INLINE FIndex legalize(FIndex a);
};

} //namespace delaunator

namespace delaunator
{

// Kahan and Babuska summation, Neumaier variant; accumulates less FP error
inline FReal sum(const TArray<FReal>& x)
{
    FReal sum = x[0];
    FReal err = 0.0;

    for (FIndex i = 1; i < x.Num(); i++)
    {
        const FReal k = x[i];
        const FReal m = sum + k;
        err += FMath::Abs(sum) >= FMath::Abs(k) ? sum - m + k : k - m + sum;
        sum = m;
    }
    return sum + err;
}

//@see https://stackoverflow.com/questions/33333363/built-in-mod-vs-custom-mod-function-improve-the-performance-of-modulus-op/33333636#33333636
FORCEINLINE FIndex fast_mod(const FIndex i, const FIndex c)
{
    return i >= c ? i % c : i;
}

FORCEINLINE FReal dist(
    const FReal ax,
    const FReal ay,
    const FReal bx,
    const FReal by
    )
{
    const FReal dx = ax - bx;
    const FReal dy = ay - by;
    return dx * dx + dy * dy;
}

FORCEINLINE bool orient(
    const FReal px,
    const FReal py,
    const FReal qx,
    const FReal qy,
    const FReal rx,
    const FReal ry
    )
{
    return (qy - py) * (rx - qx) - (qx - px) * (ry - qy) < 0.0;
}

FORCEINLINE FReal circumradius(
    const FReal ax,
    const FReal ay,
    const FReal bx,
    const FReal by,
    const FReal cx,
    const FReal cy
    )
{
    const FReal dx = bx - ax;
    const FReal dy = by - ay;
    const FReal ex = cx - ax;
    const FReal ey = cy - ay;

    const FReal bl = dx * dx + dy * dy;
    const FReal cl = ex * ex + ey * ey;
    const FReal d = dx * ey - dy * ex;

    const FReal x = (ey * bl - dy * cl) * 0.5 / d;
    const FReal y = (dx * cl - ex * bl) * 0.5 / d;

    if ((bl > 0.0 || bl < 0.0) && (cl > 0.0 || cl < 0.0) && (d > 0.0 || d < 0.0))
    {
        return x * x + y * y;
    }
    else
    {
        return TNumericLimits<FReal>::Max();
    }
}

FORCEINLINE void circumcenter(
    FReal& centerx,
    FReal& centery,
    const FReal ax,
    const FReal ay,
    const FReal bx,
    const FReal by,
    const FReal cx,
    const FReal cy
    )
{
    const FReal dx = bx - ax;
    const FReal dy = by - ay;
    const FReal ex = cx - ax;
    const FReal ey = cy - ay;

    const FReal bl = dx * dx + dy * dy;
    const FReal cl = ex * ex + ey * ey;
    const FReal d = dx * ey - dy * ex;

    const FReal x = ax + (ey * bl - dy * cl) * 0.5 / d;
    const FReal y = ay + (dx * cl - ex * bl) * 0.5 / d;

    centerx = x;
    centery = y;
}

FORCEINLINE bool in_circle(
    const FReal ax,
    const FReal ay,
    const FReal bx,
    const FReal by,
    const FReal cx,
    const FReal cy,
    const FReal px,
    const FReal py
    )
{
    const FReal dx = ax - px;
    const FReal dy = ay - py;
    const FReal ex = bx - px;
    const FReal ey = by - py;
    const FReal fx = cx - px;
    const FReal fy = cy - py;

    const FReal ap = dx * dx + dy * dy;
    const FReal bp = ex * ex + ey * ey;
    const FReal cp = fx * fx + fy * fy;

    return (dx * (ey * cp - bp * fy) -
            dy * (ex * cp - bp * fx) +
            ap * (ex * fy - ey * fx)) < 0.0;
}

FORCEINLINE bool check_pts_equal(FReal x1, FReal y1, FReal x2, FReal y2)
{
    return FMath::Abs(x1 - x2) <= EPSILON &&
           FMath::Abs(y1 - y2) <= EPSILON;
}

// monotonically increases with real angle,
// but doesn't need expensive trigonometry
inline FReal pseudo_angle(const FReal dx, const FReal dy)
{
    const FReal p = dx / (FMath::Abs(dx) + FMath::Abs(dy));
    return (dy > 0.0 ? 3.0 - p : 1.0 + p) / 4.0; // [0..1)
}

inline compare::compare(
    const TArrayView<const FReal>& in_coords,
    FReal centerx,
    FReal centery
    )
    : coords(in_coords)
{
    FIndex n = coords.Num() / 2;
    dists.Reserve(n);
    FReal const *xcoord = coords.GetData();
    FReal const *ycoord = coords.GetData()+1;
    while (n--)
    {
        dists.Emplace(dist(*xcoord, *ycoord, centerx, centery));
        xcoord += 2;
        ycoord += 2;
    }
}

FORCEINLINE bool compare::operator()(FIndex i, FIndex j) const
{
    const FReal diff1 = dists[i] - dists[j];
    const FReal diff2 = coords[2 * i] - coords[2 * j];
    const FReal diff3 = coords[2 * i + 1] - coords[2 * j + 1];

    //ABELL - Not sure why we're not just checking != 0 here.
    if (diff1 > 0.0 || diff1 < 0.0)
    {
        return diff1 < 0;
    }
    else
    if (diff2 > 0.0 || diff2 < 0.0)
    {
        return diff2 < 0;
    }
    else
    {
        return diff3 < 0;
    }
}

inline FReal Delaunator::get_hull_area() const
{
    TArray<FReal> hull_area;
    FIndex e = hull_start;

    do
    {
        hull_area.Emplace((coords[2 * e] - coords[2 * hull_prev[e]]) * (coords[2 * e + 1] + coords[2 * hull_prev[e] + 1]));
        e = hull_next[e];
    }
    while (e != hull_start);

    return sum(hull_area);
}

FORCEINLINE FIndex Delaunator::hash_key(const FReal x, const FReal y) const
{
    const FReal dx = x - m_centerx;
    const FReal dy = y - m_centery;
    return fast_mod(
        FMath::FloorToInt(pseudo_angle(dx, dy) * static_cast<FReal>(m_hash_size)),
        m_hash_size
        );
}

FORCEINLINE void Delaunator::link(const FIndex a, const FIndex b)
{
    FIndex s = halfedges.Num();

    if (a == s)
    {
        halfedges.Emplace(b);
    }
    else
    if (a < s)
    {
        halfedges[a] = b;
    }
    else
    {
        checkf(false, TEXT("Cannot link edge"));
    }

    if (b != INVALID_INDEX)
    {
        FIndex s2 = halfedges.Num();

        if (b == s2)
        {
            halfedges.Emplace(a);
        }
        else
        if (b < s2)
        {
            halfedges[b] = a;
        }
        else
        {
            checkf(false, TEXT("Cannot link edge"));
        }
    }
}

FORCEINLINE FIndex Delaunator::add_triangle(
    FIndex i0,
    FIndex i1,
    FIndex i2,
    FIndex a,
    FIndex b,
    FIndex c
    )
{
    FIndex t = triangles.Num();
    triangles.Emplace(i0);
    triangles.Emplace(i1);
    triangles.Emplace(i2);
    link(t, a);
    link(t + 1, b);
    link(t + 2, c);
    return t;
}

}; // namespace delaunator

#undef INLINE
