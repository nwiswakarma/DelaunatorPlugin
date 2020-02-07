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

typedef float FReal;
typedef int32 FIndex;

constexpr FIndex INVALID_INDEX =
    //(std::numeric_limits<FIndex>::max)();
    -1;

template <typename FArrayType, typename FValueType> struct arr
{
    inline static FIndex Num(const FArrayType& InArray)
    {
        return InArray.Num();
    };

    inline static void Reserve(FArrayType& InArray, FIndex InSize)
    {
        return InArray.Reserve(InSize);
    };

    inline static void SetNum(FArrayType& InArray, FIndex InSize)
    {
        return InArray.SetNum(InSize);
    };

    inline static void Emplace(FArrayType& InArray, const FValueType& InValue)
    {
        return InArray.Emplace(InValue);
    };
};

class Delaunator
{

public:
    //const FReal* coords;
    //const FIndex coords_num;
    TArrayView<const FReal> coords;

    TArray<FIndex> triangles;
    TArray<FIndex> halfedges;
    TArray<FIndex> hull_prev;
    TArray<FIndex> hull_next;
    TArray<FIndex> hull_tri;
    FIndex hull_start;

    INLINE Delaunator(const FReal* in_coords, FIndex in_coords_num);
    INLINE void update();
    INLINE FReal get_hull_area();

private:
    TArray<FIndex> m_hash;
    FReal m_centerx;
    FReal m_centery;
    FIndex m_hash_size;
    TArray<FIndex> m_edge_stack;

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
