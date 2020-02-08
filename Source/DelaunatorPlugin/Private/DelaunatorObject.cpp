////////////////////////////////////////////////////////////////////////////////
//
// MIT License
// 
// Copyright (c) 2018-2019 Nuraga Wiswakarma
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
////////////////////////////////////////////////////////////////////////////////
// 

#include "DelaunatorObject.h"

void UDelaunatorObject::UpdateFromPoints(const TArray<FVector2D>& InPoints)
{
    Points = InPoints;
    Delaunator.update(Points);
}

void UDelaunatorObject::CopyIndices(TArray<int32>& OutTriangles, TArray<int32>& OutHalfEdges)
{
    OutTriangles = Delaunator.triangles;
    OutHalfEdges = Delaunator.halfedges;
}

UDelaunatorValueObject* UDelaunatorObject::CreateDefaultPointValueObject(
    UObject* Outer,
    FName ValueName,
    TSubclassOf<UDelaunatorValueObject> ValueType
    )
{
    if (! IsValidDelaunatorObject() || ValueName.IsNone())
    {
        return nullptr;
    }

    UDelaunatorValueObject* ValueObject = GetPointValueObject(ValueName);

    if (IsValid(ValueObject))
    {
        ValueObject->SetOwner(this);
        ValueObject->InitializePointValues();
    }
    else
    {
        ValueObject = NewObject<UDelaunatorValueObject>(Outer, ValueType);
        ValueObject->SetOwner(this);
        ValueObject->InitializePointValues();
        PointValueMap.Emplace(ValueName, ValueObject);
    }

    return ValueObject;
}

UDelaunatorValueObject* UDelaunatorObject::CreateDefaultTriangleValueObject(
    UObject* Outer,
    FName ValueName,
    TSubclassOf<UDelaunatorValueObject> ValueType
    )
{
    if (! IsValidDelaunatorObject() || ValueName.IsNone())
    {
        return nullptr;
    }

    UDelaunatorValueObject* ValueObject = GetTriangleValueObject(ValueName);

    if (IsValid(ValueObject))
    {
        ValueObject->SetOwner(this);
        ValueObject->InitializeTriangleValues();
    }
    else
    {
        ValueObject = NewObject<UDelaunatorValueObject>(Outer, ValueType);
        ValueObject->SetOwner(this);
        ValueObject->InitializeTriangleValues();
        TriangleValueMap.Emplace(ValueName, ValueObject);
    }

    return ValueObject;
}

// Query Utility

void UDelaunatorObject::FindPointTriangles(
    TArray<int32>& OutTriangleIndices,
    int32 InTrianglePointIndex
    )
{
    const TArray<int32>& InTriangles(GetTriangles());
    const TArray<int32>& InHalfEdges(GetHalfEdges());

    if (! InTriangles.IsValidIndex(InTrianglePointIndex))
    {
        return;
    }

    int32 InitialTriangleIndex = InTrianglePointIndex/3;

    // Add initial triangle
    OutTriangleIndices.Emplace(InitialTriangleIndex);

    // Start from initial point index and iterate CW

    int32 HalfEdge = InHalfEdges[InTrianglePointIndex];

    while (HalfEdge >= 0)
    {
        int32 TriangleIndex = HalfEdge/3;
        int32 PointIndex = GetNextTriCorner(TriangleIndex, HalfEdge);

        // All point triangles visited, break
        if (PointIndex == InTrianglePointIndex)
        {
            break;
        }

        // Add next triangle
        OutTriangleIndices.Emplace(TriangleIndex);

        // Get corner of next triangle
        HalfEdge = InHalfEdges[PointIndex];
    }

    // All point triangles visited, return
    if (HalfEdge >= 0)
    {
        return;
    }

    // Start from initial point index and iterate CCW

    HalfEdge = InHalfEdges[GetPrevTriCorner(InitialTriangleIndex, InTrianglePointIndex)];

    while (HalfEdge >= 0)
    {
        int32 TriangleIndex = HalfEdge/3;
        int32 PointIndex = HalfEdge;

        // Ensure no cyclic iteration
        check(PointIndex != InTrianglePointIndex);

        // Add prev triangle
        OutTriangleIndices.Emplace(TriangleIndex);

        // Get corner of prev triangle
        HalfEdge = InHalfEdges[GetPrevTriCorner(TriangleIndex, PointIndex)];
    }
}

void UDelaunatorObject::FindTrianglesBetweenPoints(
    TArray<int32>& OutTriangleIndices,
    int32 PointIndex0,
    int32 PointIndex1
    )
{
    OutTriangleIndices.Reset();

    const TArray<FVector2D>& InPoints(GetPoints());
    const TArray<int32>& InTriangles(GetTriangles());
    const TArray<int32>& InHalfEdges(GetHalfEdges());

    if (! IsValidDelaunatorObject()          ||
        ! InPoints.IsValidIndex(PointIndex0) ||
        ! InPoints.IsValidIndex(PointIndex1))
    {
        return;
    }

    // Find initial triangle

    const int32 TriCount = InTriangles.Num() / 3;
    int32 InitialTrianglePoint = -1;

    for (int32 ti=0; ti<TriCount; ++ti)
    {
        int32 pi = ti*3;
        int32 piN = pi+3;

        for (; pi<piN; ++pi)
        {
            if (InTriangles[pi] == PointIndex0)
            {
                InitialTrianglePoint = pi;
                break;
            }
        }

        if (InitialTrianglePoint >= 0)
        {
            break;
        }
    }

    // No valid triangle found, abort
    if (InitialTrianglePoint < 0)
    {
        return;
    }

    // Find triangles connected to PointIndex0
    TArray<int32> InitialTriangles;
    FindPointTriangles(InitialTriangles, InitialTrianglePoint);

    check(InitialTriangles.Num() > 0);

    // Search for target point on initial triangles.
    // If found, add the triangle then return.
    for (int32 ti : InitialTriangles)
    {
        if (FindCornerIndex(ti, PointIndex1) >= 0)
        {
            OutTriangleIndices.Emplace(ti);
            return;
        }
    }

    // Find initial segment intersecting triangle

    const FVector2D& P0(InPoints[PointIndex0]);
    const FVector2D& P1(InPoints[PointIndex1]);

    int32* StartTriangle = InitialTriangles.FindByPredicate(
        [&](const int32 ti)
        {
            int32 pi = FindCornerIndex(ti, PointIndex0);
            return pi >= 0
                ? HasOppositeSideSegmentIntersection(P0, P1, ti, pi)
                : false;
        } );

    // No triangle side segment intersection, abort
    if (! StartTriangle)
    {
        return;
    }

    enum { SEARCH_LIMIT = 100 };

    int32 pi = FindCornerIndex(*StartTriangle, PointIndex0);

    for (int32 i=0; i<SEARCH_LIMIT; ++i)
    {
        int32 ti = pi/3;

        OutTriangleIndices.Emplace(ti);

        if (FindCornerIndex(ti, PointIndex1) >= 0)
        {
            break;
        }

        pi = GetNextTriCorner(ti, pi);
        pi = FindSegmentClippingSide(P0, P1, ti, pi);

        //if (pi < 0)
        //{
        //    break;
        //}
        check(pi >= 0);

        pi = InHalfEdges[pi];

        //if (pi < 0)
        //{
        //    break;
        //}
        check(pi >= 0);
    }
}

bool UDelaunatorObject::FindBoundaryPoints(
    TArray<int32>& OutPointIndices,
    const TArray<int32>& InBoundaryTriangles,
    int32 BoundaryPoint0,
    int32 BoundaryPoint1
    )
{
    OutPointIndices.Reset();

    const TArray<FVector2D>& InPoints(GetPoints());
    const TArray<int32>& InTriangles(GetTriangles());

    if (! IsValidDelaunatorObject()             ||
        ! InPoints.IsValidIndex(BoundaryPoint0) ||
        ! InPoints.IsValidIndex(BoundaryPoint1) ||
        BoundaryPoint0 == BoundaryPoint1        ||
        InBoundaryTriangles.Num() < 1)
    {
        return false;
    }

    // Single boundary triangle, check if both boundary point is in the triangle
    if (InBoundaryTriangles.Num() == 1)
    {
        int32 ti = InBoundaryTriangles[0];
        int32 pi0 = FindCornerIndex(ti, BoundaryPoint0);
        int32 pi1 = FindCornerIndex(ti, BoundaryPoint1);

        // Both boundary points is in triangle, add as output indices then return
        if (pi0 >=0 && pi1 >= 0)
        {
            int32 iNext = GetNextTriCorner(ti, pi0);
            int32 iPrev = GetPrevTriCorner(ti, pi0);
            int32 iMid = iNext == pi1 ? iPrev : iNext;

            OutPointIndices.Emplace(BoundaryPoint0);
            OutPointIndices.Emplace(InTriangles[iMid]);
            OutPointIndices.Emplace(BoundaryPoint1);

            return true;
        }
        // Triangle does not contain both boundary points, abort
        else
        {
            return false;
        }
    }

    const FVector2D& BP0(InPoints[BoundaryPoint0]);
    const FVector2D& BP1(InPoints[BoundaryPoint1]);
    const FVector2D DBP01 = BP1-BP0;
    const FVector2D CBP01(-DBP01.Y, DBP01.X);

    TArray<int32> PointIndices;

    int32 It = BoundaryPoint0;
    int32 ti = InBoundaryTriangles[0];
    int32 pi = FindCornerIndex(ti, It);

    if (pi < 0)
    {
        return false;
    }

    PointIndices.Emplace(It);

    for (int32 i=0; i<(InBoundaryTriangles.Num()-1); ++i)
    {
        ti = InBoundaryTriangles[i];
        pi = FindCornerIndex(ti, It);

        if (pi < 0)
        {
            return false;
        }

        int32 iNext = GetNextTriCorner(ti, pi);
        int32 iPrev = GetPrevTriCorner(ti, pi);

        const FVector2D& PNext(InPoints[InTriangles[iNext]]);
        const FVector2D& PPrev(InPoints[InTriangles[iPrev]]);

        const float DotNext = (PNext-BP0) | CBP01;
        const float DotPrev = (PPrev-BP0) | CBP01;

        if (DotNext >= 0.f && DotPrev >= 0.f)
        {
            if (DotNext > DotPrev)
            {
                It = InTriangles[iNext];
            }
            else
            {
                It = InTriangles[iPrev];
            }
        }
        else
        if (DotNext >= 0.f)
        {
            It = InTriangles[iNext];
        }
        else
        if (DotPrev >= 0.f)
        {
            It = InTriangles[iPrev];
        }

        if (PointIndices.Num() == 0 || PointIndices.Last() != It)
        {
            PointIndices.Emplace(It);
        }
    }

    It = BoundaryPoint1;

    // Check if last triangle have BoundaryPoint1
    if (FindCornerIndex(InBoundaryTriangles.Last(), It) >= 0)
    {
        PointIndices.Emplace(BoundaryPoint1);
        OutPointIndices = MoveTemp(PointIndices);

        return true;
    }

    return false;
}

bool UDelaunatorObject::FindPolyIntersectTriangles(
    TArray<int32>& OutTriangles,
    const TArray<int32>& InPolyPoints,
    bool bClosedPoly
    )
{
    OutTriangles.Reset();

    const TArray<FVector2D>& InPoints(GetPoints());
    const TArray<int32>& InTriangles(GetTriangles());
    const TArray<int32>& InHalfEdges(GetHalfEdges());

    if (! IsValidDelaunatorObject()               ||
        (! bClosedPoly && InPolyPoints.Num() < 2) ||
        (  bClosedPoly && InPolyPoints.Num() < 3))
    {
        return false;
    }

    check(InPoints.IsValidIndex(InPolyPoints[0]));
    check(InPoints.IsValidIndex(InPolyPoints.Last()));

    const FVector2D& P0(InPoints[InPolyPoints[0]]);
    const FVector2D& PN(InPoints[InPolyPoints.Last()]);

    TArray<int32> BoundaryTriangles;

    const int32 PolyPointCount = InPolyPoints.Num();
    const int32 PointItCount = (! bClosedPoly || P0.Equals(PN))
        ? PolyPointCount-1
        : PolyPointCount;

    for (int32 i=0; i<PointItCount; ++i)
    {
        int32 i0 = i;
        int32 i1 = i+1;

        if (i1 == PolyPointCount)
        {
            i1 = 0;
        }

        int32 pi0 = InPolyPoints[i0];
        int32 pi1 = InPolyPoints[i1];

        // Invalid input points, abort
        if (! InPoints.IsValidIndex(pi0) ||
            ! InPoints.IsValidIndex(pi1))
        {
            return false;
        }

        // Find triangles between polyline

        TArray<int32> TrianglesBetweenPoints;

        FindTrianglesBetweenPoints(
            TrianglesBetweenPoints,
            pi0,
            pi1
            );

        // Only single triangle between points, no replacement required
        if (TrianglesBetweenPoints.Num() == 1)
        {
            continue;
        }

        // Add triangle between points as boundary triangles

        BoundaryTriangles.Append(TrianglesBetweenPoints);
    }

    OutTriangles = TSet<int32>(BoundaryTriangles).Array();

#if 0
    // Assign input triangle indices as initial output indices

    OutTriangles = InTriangles;

    // Generate list of triangle indices to remove

    // Sort descending
    BoundaryTriangles.Sort(
        [](const int32& i0, const int32& i1)
        {
            return i1 < i0;
        } );

    // Remove triangles unique
    {
        int32 LastRemoved = -1;
        int32 RemoveCount = 0;

        for (int32 i : BoundaryTriangles)
        {
            if (i != LastRemoved)
            {
                LastRemoved = i;
                ++RemoveCount;

                OutTriangles.RemoveAtSwap(i*3, 3, false);
            }
        }
    }

    // Remove all triangles out of poly
    {
        TArray<FVector2D> PolyPoints;
        TSet<int32> BoundarySet(InPolyPoints);

        PolyPoints.SetNumUninitialized(InPolyPoints.Num());

        for (int32 i=0; i<InPolyPoints.Num(); ++i)
        {
            PolyPoints[i] = InPoints[InPolyPoints[i]];
        }

        int32 It = 0;

        while (It < OutTriangles.Num())
        {
            int32 pi0 = OutTriangles[It  ];
            int32 pi1 = OutTriangles[It+1];
            int32 pi2 = OutTriangles[It+2];

            if (BoundarySet.Contains(pi0) &&
                BoundarySet.Contains(pi1) &&
                BoundarySet.Contains(pi2))
            {
                FVector2D TriCenter =
                    (InPoints[pi0] + InPoints[pi1] + InPoints[pi2]) / 3.f;

                if (! UGULPolyUtilityLibrary::IsPointOnPoly(TriCenter, PolyPoints))
                {
                    OutTriangles.RemoveAtSwap(It, 3, false);
                    continue;
                }
            }

            It += 3;
        }
    }
#endif

    return true;
}
