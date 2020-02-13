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
#include "Poly/GULPolyUtilityLibrary.h"
#include "DelaunatorVoronoi.h"

void UDelaunatorObject::UpdateFromPoints(const TArray<FVector2D>& InPoints)
{
    Points = InPoints;
    Delaunator.update(Points);

    const TArray<int32>& InTriangles(GetTriangles());
    const TArray<int32>& InHalfEdges(GetHalfEdges());

    const int32 PointCount = Points.Num();

    // Generate hull and boundary data

    Hull.Reset(Delaunator.hull_size);

    {
        int32 e = Delaunator.hull_start;
        do
        {
            Hull.Emplace(e);
        }
        while ((e = Delaunator.hull_next[e]) != Delaunator.hull_start);

        HullIndex.SetNumUninitialized(PointCount);
        FMemory::Memset(
            HullIndex.GetData(),
            ~0,
            HullIndex.Num()*HullIndex.GetTypeSize()
            );

        for (int32 i=0; i<Hull.Num(); ++i)
        {
            HullIndex[Hull[i]] = i;
        }
    }
    

    // Generate Inedges

    // Compute an index from each point to an (arbitrary) incoming halfedge
    // Used to give the first neighbor of each point; for this reason,
    // on the hull we give priority to exterior halfedges

    Inedges.SetNumUninitialized(PointCount);
    FMemory::Memset(Inedges.GetData(), ~0, Inedges.Num()*Inedges.GetTypeSize());

    for (int32 e=0; e<InHalfEdges.Num(); ++e)
    {
        const int32 p = InTriangles[((e%3) == 2) ? e-2 : e+1];

        if (InHalfEdges[e] == -1 || Inedges[p] == -1)
        {
            Inedges[p] = e;
        }
    }

    // Generate boundary flags
    
    BoundaryFlags.Init(false, GetPointCount());

    for (int32 i : Hull)
    {
        BoundaryFlags[i] = true;
    }
}

void UDelaunatorObject::CopyIndices(TArray<int32>& OutTriangles, TArray<int32>& OutHalfEdges)
{
    OutTriangles = Delaunator.triangles;
    OutHalfEdges = Delaunator.halfedges;
}

UDelaunatorValueObject* UDelaunatorObject::CreateDefaultValueObject(
    UObject* Outer,
    FName ValueName,
    TSubclassOf<UDelaunatorValueObject> ValueType
    )
{
    if (! IsValidDelaunatorObject() || ValueName.IsNone())
    {
        return nullptr;
    }

    UDelaunatorValueObject* ValueObject = GetValueObject(ValueName);

    if (! IsValid(ValueObject))
    {
        ValueObject = NewObject<UDelaunatorValueObject>(Outer, ValueType);
        ValueMap.Emplace(ValueName, ValueObject);
    }

    return ValueObject;
}

UDelaunatorValueObject* UDelaunatorObject::CreateDefaultPointValueObject(
    FName ValueName,
    TSubclassOf<UDelaunatorValueObject> ValueType
    )
{
    UDelaunatorValueObject* ValueObject = CreateDefaultValueObject(
        ValueName,
        ValueType
        );

    if (IsValid(ValueObject))
    {
        ValueObject->InitializeValues(GetPointCount());
    }

    return ValueObject;
}

UDelaunatorValueObject* UDelaunatorObject::CreateDefaultTriangleValueObject(
    FName ValueName,
    TSubclassOf<UDelaunatorValueObject> ValueType
    )
{
    UDelaunatorValueObject* ValueObject = CreateDefaultValueObject(
        ValueName,
        ValueType
        );

    if (IsValid(ValueObject))
    {
        ValueObject->InitializeValues(GetTriangleCount());
    }

    return ValueObject;
}

void UDelaunatorObject::FindPointsByValue(
    TArray<int32>& OutPointIndices,
    UDelaunatorCompareOperator* CompareOperator
    )
{
    if (IsValid(CompareOperator))
    {
        CompareOperator->GetResults(OutPointIndices, GetPointCount());
    }
}

void UDelaunatorObject::FindTrianglesByValue(
    TArray<int32>& OutTriangleIndices,
    UDelaunatorCompareOperator* CompareOperator
    )
{
    if (IsValid(CompareOperator))
    {
        CompareOperator->GetResults(OutTriangleIndices, GetTriangleCount());
    }
}

// Query Utility

void UDelaunatorObject::GetTrianglesByPointIndices(
    TArray<int32>& OutTriangles,
    const TArray<int32>& InPointIndices,
    bool bInverseResult
    )
{
    OutTriangles.Reset();

    // Invalid delaunator object, abort
    if (! IsValidDelaunatorObject())
    {
        return;
    }

    const TArray<FVector2D>& InPoints(GetPoints());
    const TArray<int32>& InTriangles(GetTriangles());
    const int32 TriangleCount = GetTriangleCount();

    // Generate valid point index set
    TSet<int32> PointIndexSet(InPointIndices.FilterByPredicate(
        [&InPoints](const int32& PointIndex)
        {
            return InPoints.IsValidIndex(PointIndex);
        } ) );

    // No valid point index, abort
    if (PointIndexSet.Num() < 1)
    {
        return;
    }

    // Gather all triangles that consist of any of the point indices
    if (! bInverseResult)
    {
        for (int32 ti=0; ti<TriangleCount; ++ti)
        {
            int32 i = ti*3;

            if (PointIndexSet.Contains(InTriangles[i  ]) ||
                PointIndexSet.Contains(InTriangles[i+1]) ||
                PointIndexSet.Contains(InTriangles[i+2]))
            {
                OutTriangles.Emplace(ti);
            }
        }
    }
    else
    {
        for (int32 ti=0; ti<TriangleCount; ++ti)
        {
            int32 i = ti*3;

            if (PointIndexSet.Contains(InTriangles[i  ]) ||
                PointIndexSet.Contains(InTriangles[i+1]) ||
                PointIndexSet.Contains(InTriangles[i+2]))
            {
                continue;
            }

            OutTriangles.Emplace(ti);
        }
    }
}

void UDelaunatorObject::GetTrianglesByEdgeIndices(
    TArray<int32>& OutTriangles,
    const TArray<int32>& InPointIndices,
    bool bInverseResult
    )
{
    OutTriangles.Reset();

    // Invalid delaunator object, abort
    if (! IsValidDelaunatorObject())
    {
        return;
    }

    const TArray<FVector2D>& InPoints(GetPoints());
    const TArray<int32>& InTriangles(GetTriangles());
    const int32 TriangleCount = GetTriangleCount();

    // Generate valid point index set
    TSet<int32> PointIndexSet(InPointIndices.FilterByPredicate(
        [&InPoints](const int32& PointIndex)
        {
            return InPoints.IsValidIndex(PointIndex);
        } ) );

    // No valid point index, abort
    if (PointIndexSet.Num() < 1)
    {
        return;
    }

    // Gather all triangles that consist of any of the point indices
    if (! bInverseResult)
    {
        for (int32 ti=0; ti<TriangleCount; ++ti)
        {
            int32 i = ti*3;
            bool bInSet0 = PointIndexSet.Contains(InTriangles[i  ]);
            bool bInSet1 = PointIndexSet.Contains(InTriangles[i+1]);
            bool bInSet2 = PointIndexSet.Contains(InTriangles[i+2]);

            if ((bInSet0 && bInSet1) ||
                (bInSet0 && bInSet2) ||
                (bInSet1 && bInSet2))
            {
                OutTriangles.Emplace(ti);
            }
        }
    }
    else
    {
        for (int32 ti=0; ti<TriangleCount; ++ti)
        {
            int32 i = ti*3;
            bool bInSet0 = PointIndexSet.Contains(InTriangles[i  ]);
            bool bInSet1 = PointIndexSet.Contains(InTriangles[i+1]);
            bool bInSet2 = PointIndexSet.Contains(InTriangles[i+2]);

            if ((bInSet0 && bInSet1) ||
                (bInSet0 && bInSet2) ||
                (bInSet1 && bInSet2))
            {
                continue;
            }

            OutTriangles.Emplace(ti);
        }
    }
}

void UDelaunatorObject::GetPointNeighbours(TArray<int32>& OutNeighbourIndices, int32 PointIndex) const
{
    check(IsValidDelaunatorObject());

    OutNeighbourIndices.Reset();

    const TArray<int32>& InTriangles(GetTriangles());
    const TArray<int32>& InHalfEdges(GetHalfEdges());
    const TArray<int32>& InInedges(GetInedges());

    const int32 e0 = InInedges[PointIndex];

    // coincident point, skip
    if (e0 == -1)
    {
        return;
    }

    // Iterate over point triangles

    int32 e = e0;
    do
    {
        const int32 t = e / 3;
        const int32 f = t * 3;

        OutNeighbourIndices.Emplace(InTriangles[e]);

        e = ((e-f) < 2) ? e+1 : f;

        // Ensure sane triangulation
        check(PointIndex == InTriangles[e]);

        e = InHalfEdges[e];
    }
    while (e != e0 && e != -1);
}

void UDelaunatorObject::GetPointNeighbours(TArray<int32>& OutNeighbourIndices, TArray<int32>& OutNeighbourTriangles, int32 PointIndex) const
{
    check(IsValidDelaunatorObject());

    OutNeighbourIndices.Reset();
    OutNeighbourTriangles.Reset();

    const TArray<int32>& InTriangles(GetTriangles());
    const TArray<int32>& InHalfEdges(GetHalfEdges());
    const TArray<int32>& InInedges(GetInedges());

    const int32 e0 = InInedges[PointIndex];

    // coincident point, skip
    if (e0 == -1)
    {
        return;
    }

    // Iterate over point triangles

    int32 e = e0;
    do
    {
        const int32 t = e / 3;
        const int32 f = t * 3;

        OutNeighbourIndices.Emplace(InTriangles[e]);
        OutNeighbourTriangles.Emplace(t);

        e = ((e-f) < 2) ? e+1 : f;

        // Ensure sane triangulation
        check(PointIndex == InTriangles[e]);

        e = InHalfEdges[e];
    }
    while (e != e0 && e != -1);
}

// Boundary Utility

void UDelaunatorObject::GetPointTrianglesBoundary(
    TArray<int32>& OutTriangleIndices,
    int32 InTrianglePointIndex
    ) const
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

    // Start from initial point index and iterate CCW

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

    // Start from initial point index and iterate CW

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

void UDelaunatorObject::GetPointNeighboursBoundary(
    TArray<int32>& OutNeighbourIndices,
    int32 InTrianglePointIndex
    ) const
{
    const TArray<int32>& InTriangles(GetTriangles());
    const TArray<int32>& InHalfEdges(GetHalfEdges());

    if (! InTriangles.IsValidIndex(InTrianglePointIndex))
    {
        return;
    }

    int32 InitialTriangleIndex = InTrianglePointIndex/3;

    // Add initial triangle point next neighbour

    OutNeighbourIndices.Emplace(
        InTriangles[GetNextTriCorner(InitialTriangleIndex, InTrianglePointIndex)]
        );

    // Start from initial point index and iterate CCW

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

        // Add neighbour point
        OutNeighbourIndices.Emplace(
            InTriangles[GetNextTriCorner(TriangleIndex, PointIndex)]
            );

        // Get corner of next triangle
        HalfEdge = InHalfEdges[PointIndex];
    }

    // All point triangles visited, return
    if (HalfEdge >= 0)
    {
        return;
    }

    // Otherwise, iteration hit boundary edge,
    // reverse iteration from initial point

    int32 ReverseInitialPoint = GetPrevTriCorner(InitialTriangleIndex, InTrianglePointIndex);

    // Add initial triangle point prev neighbour

    OutNeighbourIndices.Emplace(InTriangles[ReverseInitialPoint]);

    // Start from initial point index and iterate CW

    HalfEdge = InHalfEdges[ReverseInitialPoint];

    while (HalfEdge >= 0)
    {
        int32 TriangleIndex = HalfEdge/3;
        int32 PointIndex = HalfEdge;

        // Ensure no cyclic iteration
        check(PointIndex != InTrianglePointIndex);

        int32 PrevPointIndex = GetPrevTriCorner(TriangleIndex, PointIndex);

        // Add neighbour point
        OutNeighbourIndices.Emplace(InTriangles[PrevPointIndex]);

        // Get corner of prev triangle
        HalfEdge = InHalfEdges[PrevPointIndex];
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
    int32 InitialTrianglePoint = GetTrianglePointIndex(PointIndex0);

    // No valid triangle found, abort
    if (InitialTrianglePoint < 0)
    {
        return;
    }

    // Find triangles connected to PointIndex0
    TArray<int32> InitialTriangles;
    GetPointTriangles(InitialTriangles, InitialTrianglePoint);

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

bool UDelaunatorObject::FindPolyBoundaryTriangles(
    TArray<int32>& OutTriangles,
    const TArray<int32>& InPolyPointIndices,
    bool bClosedPoly,
    bool bAllowDirectConnection
    )
{
    OutTriangles.Reset();

    const TArray<FVector2D>& InPoints(GetPoints());
    const TArray<int32>& InTriangles(GetTriangles());

    if (! IsValidDelaunatorObject()                     ||
        (! bClosedPoly && InPolyPointIndices.Num() < 2) ||
        (  bClosedPoly && InPolyPointIndices.Num() < 3))
    {
        return false;
    }

    check(InPoints.IsValidIndex(InPolyPointIndices[0]));
    check(InPoints.IsValidIndex(InPolyPointIndices.Last()));

    const FVector2D& P0(InPoints[InPolyPointIndices[0]]);
    const FVector2D& PN(InPoints[InPolyPointIndices.Last()]);

    TArray<int32> BoundaryTriangles;

    const int32 PolyPointCount = InPolyPointIndices.Num();
    const int32 PointItCount = (! bClosedPoly || P0.Equals(PN))
        ? PolyPointCount-1
        : PolyPointCount;

    // Find all triangles between boundary poly
    for (int32 i=0; i<PointItCount; ++i)
    {
        int32 i0 = i;
        int32 i1 = i+1;

        if (i1 == PolyPointCount)
        {
            i1 = 0;
        }

        int32 pi0 = InPolyPointIndices[i0];
        int32 pi1 = InPolyPointIndices[i1];

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

        // Poly line have direct connection within single triangle, skip
        if (!bAllowDirectConnection && TrianglesBetweenPoints.Num() == 1)
        {
            continue;
        }

        // Add triangle between points as boundary triangles
        BoundaryTriangles.Append(TrianglesBetweenPoints);
    }

    // Find all boundary triangles out of poly
    {
        TArray<FVector2D> PolyPoints;
        TSet<int32> BoundarySet(InPolyPointIndices);

        PolyPoints.SetNumUninitialized(InPolyPointIndices.Num());

        for (int32 i=0; i<InPolyPointIndices.Num(); ++i)
        {
            PolyPoints[i] = InPoints[InPolyPointIndices[i]];
        }

        const int32 TriangleCount = GetTriangleCount();

        for (int32 ti=0; ti<TriangleCount; ++ti)
        {
            int32 i = ti*3;
            int32 pi0 = InTriangles[i  ];
            int32 pi1 = InTriangles[i+1];
            int32 pi2 = InTriangles[i+2];

            // Triangle consists of boundary points
            if (BoundarySet.Contains(pi0) &&
                BoundarySet.Contains(pi1) &&
                BoundarySet.Contains(pi2))
            {
                FVector2D TriCenter =
                    (InPoints[pi0] + InPoints[pi1] + InPoints[pi2]) / 3.f;

                // Triangle center is outside of poly, mark as boundary triangle
                if (! UGULPolyUtilityLibrary::IsPointOnPoly(TriCenter, PolyPoints))
                {
                    BoundaryTriangles.Emplace(ti);
                }
            }
        }
    }

    OutTriangles = TSet<int32>(BoundaryTriangles).Array();

    return true;
}

bool UDelaunatorObject::FindPolyGroupsBoundaryTriangles(
    TArray<int32>& OutTriangles,
    const TArray<FGULIntGroup>& InPolyBoundaryGroups,
    bool bClosedPoly,
    bool bAllowDirectConnection
    )
{
    OutTriangles.Reset();

    const TArray<FVector2D>& InPoints(GetPoints());
    const TArray<int32>& InTriangles(GetTriangles());

    if (! IsValidDelaunatorObject()   ||
        InPolyBoundaryGroups.Num() < 1)
    {
        return false;
    }

    TArray<int32> BoundaryTriangles;

    for (const FGULIntGroup& PolyBoundaryGroup : InPolyBoundaryGroups)
    {
        const TArray<int32>& PolyIndices(PolyBoundaryGroup.Values);

        if ((! bClosedPoly && PolyIndices.Num() < 2) ||
            (  bClosedPoly && PolyIndices.Num() < 3))
        {
            continue;
        }

        check(InPoints.IsValidIndex(PolyIndices[0]));
        check(InPoints.IsValidIndex(PolyIndices.Last()));

        const FVector2D& P0(InPoints[PolyIndices[0]]);
        const FVector2D& PN(InPoints[PolyIndices.Last()]);

        const int32 PolyPointCount = PolyIndices.Num();
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

            int32 pi0 = PolyIndices[i0];
            int32 pi1 = PolyIndices[i1];

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

            // Poly line have direct connection within single triangle, skip
            if (!bAllowDirectConnection && TrianglesBetweenPoints.Num() == 1)
            {
                continue;
            }

            // Add triangle between points as boundary triangles
            BoundaryTriangles.Append(TrianglesBetweenPoints);
        }
    }

    // Remove all triangles out of poly

    TArray<FGULVector2DGroup> BoundaryPolyGroups;
    TSet<int32> BoundarySet;

    // Gather poly boundary points and index set
    for (const FGULIntGroup& PolyBoundaryGroup : InPolyBoundaryGroups)
    {
        const TArray<int32>& PolyIndices(PolyBoundaryGroup.Values);
        const int32 PointCount = PolyIndices.Num();

        BoundaryPolyGroups.AddDefaulted();
        
        TArray<FVector2D>& BoundaryPoints(BoundaryPolyGroups.Last().Points);
        BoundaryPoints.SetNumUninitialized(PointCount);

        for (int32 i=0; i<PointCount; ++i)
        {
            BoundaryPoints[i] = InPoints[PolyIndices[i]];
        }

        BoundarySet.Append(PolyIndices);
    }

    // Find all boundary triangles out of poly

    const int32 TriangleCount = GetTriangleCount();

    for (int32 ti=0; ti<TriangleCount; ++ti)
    {
        int32 i = ti*3;
        int32 pi0 = InTriangles[i  ];
        int32 pi1 = InTriangles[i+1];
        int32 pi2 = InTriangles[i+2];

        // Triangle consists of boundary points
        if (BoundarySet.Contains(pi0) &&
            BoundarySet.Contains(pi1) &&
            BoundarySet.Contains(pi2))
        {
            FVector2D TriCenter =
                (InPoints[pi0] + InPoints[pi1] + InPoints[pi2]) / 3.f;

            for (const FGULVector2DGroup& PolyGroup : BoundaryPolyGroups)
            {
                bool bPointOnPoly = UGULPolyUtilityLibrary::IsPointOnPoly(
                    TriCenter,
                    PolyGroup.Points
                    );

                // Triangle center is outside of poly,
                // mark as boundary triangle
                if (! bPointOnPoly)
                {
                    BoundaryTriangles.Emplace(ti);
                    break;
                }
            }
        }
    }

    OutTriangles = TSet<int32>(BoundaryTriangles).Array();

    return true;
}

void UDelaunatorObject::GeneratePointsDepthValues(
    UDelaunatorValueObject* ValueObject,
    const TArray<int32>& InitialPoints,
    int32 StartDepth,
    FDelaunatorCompareCallback CompareCallback
    ) const
{
    if (! IsValidDelaunatorObject() ||
        ! IsValid(ValueObject)   ||
        ! ValueObject->IsValidElementCount(GetPointCount()))
    {
        return;
    }

    if (! CompareCallback)
    {
        CompareCallback = [](int32 Index){ return true; };
    }

    TQueue<int32> VisitQueue;
    TBitArray<> VisitedSet;

    VisitedSet.Init(false, GetPointCount());

    for (int32 i : InitialPoints)
    {
        if (Points.IsValidIndex(i) && CompareCallback(i))
        {
            VisitedSet[i] = true;
            VisitQueue.Enqueue(i);
            ValueObject->SetValueInt32(i, StartDepth);
        }
    }

    TArray<int32> NeighbourCells;

    while (! VisitQueue.IsEmpty())
    {
        int32 PointIndex;
        VisitQueue.Dequeue(PointIndex);

        int32 NextDepth = ValueObject->GetValueInt32(PointIndex) + 1;

        NeighbourCells.Reset();
        GetPointNeighbours(NeighbourCells, PointIndex);

        for (int32 NeighbourCell : NeighbourCells)
        {
            if (! VisitedSet[NeighbourCell] && CompareCallback(NeighbourCell))
            {
                VisitedSet[NeighbourCell] = true;
                VisitQueue.Enqueue(NeighbourCell);
                ValueObject->SetValueInt32(NeighbourCell, NextDepth);
            }
        }
    }
}

void UDelaunatorObject::GenerateTrianglesDepthValues(
    UDelaunatorValueObject* ValueObject,
    const TArray<int32>& InitialPoints,
    FDelaunatorCompareCallback CompareCallback
    ) const
{
    if (! IsValidDelaunatorObject() ||
        ! IsValid(ValueObject)   ||
        ! ValueObject->IsValidElementCount(GetTriangleCount()))
    {
        return;
    }

    const TArray<int32>& InTriangles(GetTriangles());
    const TArray<int32>& InHalfEdges(GetHalfEdges());

    const int32 TriangleCount = GetTriangleCount();

    TQueue<int32> VisitQueue;
    TBitArray<> VisitedSet;
    TArray<int32> PointTriangles;

    VisitedSet.Init(false, GetTriangleCount());

    for (int32 i : InitialPoints)
    {
        int32 TriPointIndex = GetTrianglePointIndex(i);

        // Invalid input point index, skip
        if (TriPointIndex < 0)
        {
            continue;
        }

        GetPointTriangles(PointTriangles, TriPointIndex);

        for (int32 ti : PointTriangles)
        {
            if (! VisitedSet[ti])
            {
                VisitedSet[ti] = true;
                VisitQueue.Enqueue(ti);
                ValueObject->SetValueInt32(ti, 0);
            }
        }
    }

    while (! VisitQueue.IsEmpty())
    {
        int32 TriangleIndex;
        VisitQueue.Dequeue(TriangleIndex);

        int32 Depth = ValueObject->GetValueInt32(TriangleIndex);
        int32 FlatIndex = TriangleIndex*3;

        for (int32 i=0; i<3; ++i)
        {
            int32 Corner = FlatIndex+i;
            int32 HalfEdge = InHalfEdges[Corner];
            int32 Triangle = HalfEdge/3;

            if (HalfEdge >= 0 && ! VisitedSet[Triangle])
            {
                VisitedSet[Triangle] = true;
                VisitQueue.Enqueue(Triangle);
                ValueObject->SetValueInt32(Triangle, Depth+1);
            }
        }
    }
}

UDelaunatorVoronoi* UDelaunatorObject::GenerateVoronoiDual()
{
    UDelaunatorVoronoi* Voronoi = NewObject<UDelaunatorVoronoi>(this);

    if (IsValid(Voronoi))
    {
        Voronoi->GenerateFrom(this);
    }

    return Voronoi;
}

int32 UDelaunatorObject::FindPoint(const FVector2D& TargetPoint, int32 InitialPoint) const
{
    if (! IsValidDelaunatorObject())
    {
        return -1;
    }

    // No valid initial point specified, default to center point
    if (! Points.IsValidIndex(InitialPoint))
    {
        InitialPoint = GetTriangles()[0];
    }

    // Initial point coincident with target point, return initial point
    if (TargetPoint.Equals(Points[InitialPoint]))
    {
        return InitialPoint;
    }

    // Find all cell closer to target point

    int32 i = InitialPoint;
    int32 c;

    while ((c = FindCloser(i, TargetPoint)) >= 0 && c != i && c != InitialPoint)
    {
        i = c;
    }

    return c;
}

int32 UDelaunatorObject::FindCloser(int32 i, const FVector2D& TargetPoint) const
{
    const TArray<int32>& Triangles(GetTriangles());
    const TArray<int32>& HalfEdges(GetHalfEdges());

    check(IsValidDelaunatorObject());

    if (Inedges[i] == -1)
    {
        return (i+1) % (Points.Num() >> 1);
    }

    const int32 e0 = Inedges[i];
    int32 e = e0;
    int32 c = i;
    float dc = (TargetPoint-Points[i]).SizeSquared();

    do
    {
        int32 t = Triangles[e];
        const float dt = (TargetPoint-Points[t]).SizeSquared();

        if (dt < dc)
        {
            dc = dt;
            c = t;
        }

        e = ((e%3) == 2) ? e-2 : e+1;

        // Ensure sane triangulation
        check(i == Triangles[e]);

        e = HalfEdges[e];

        // Hull point
        if (e == -1)
        {
            e = Hull[(HullIndex[i] + 1) % Hull.Num()];

            if (e != t)
            {
                const float dh = (TargetPoint-Points[e]).SizeSquared();

                if (dh < dc)
                {
                    return e;
                }
            }

            break;
        }
    }
    while (e != e0);

    return c;
}
