#pragma once

#ifdef DELAUNATOR_HEADER_ONLY
#define INLINE inline
#else
#define INLINE
#endif

#include <limits>
#include <vector>
#include "CoreMinimal.h"

namespace delaunator {

//typedef double FReal;
typedef float FReal;
typedef int32 FIndex;

constexpr FIndex INVALID_INDEX =
    //(std::numeric_limits<FIndex>::max)();
    -1;

//class Point
//{
//public:
//    Point(FReal x, FReal y) : m_x(x), m_y(y)
//    {}
//    Point() : m_x(0), m_y(0)
//    {}
//
//
//    FReal x() const
//    { return m_x; }
//
//    FReal y() const
//    { return m_y; }
//
//private:
//    FReal m_x;
//    FReal m_y;
//};

class Delaunator {

public:
    //std::vector<FReal> const& coords;
    const FReal* coords;
    const FIndex coords_num;

    std::vector<FIndex> triangles;
    std::vector<FIndex> halfedges;
    std::vector<FIndex> hull_prev;
    std::vector<FIndex> hull_next;
    std::vector<FIndex> hull_tri;
    FIndex hull_start;

    //INLINE Delaunator(std::vector<FReal> const& in_coords);
    INLINE Delaunator(const FReal* in_coords, FIndex in_coords_num);
    INLINE void update();
    INLINE FReal get_hull_area();

private:
    std::vector<FIndex> m_hash;
    //Point m_center;
    FReal m_centerx;
    FReal m_centery;
    FIndex m_hash_size;
    std::vector<FIndex> m_edge_stack;

    INLINE FIndex legalize(FIndex a);
    INLINE FIndex hash_key(FReal x, FReal y) const;
    INLINE FIndex add_triangle(
        FIndex i0,
        FIndex i1,
        FIndex i2,
        FIndex a,
        FIndex b,
        FIndex c);
    INLINE void link(FIndex a, FIndex b);
};

} //namespace delaunator

#undef INLINE
