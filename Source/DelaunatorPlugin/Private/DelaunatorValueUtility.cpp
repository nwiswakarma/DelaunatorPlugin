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

#include "DelaunatorValueUtility.h"
#include "Geom/GULGeometryUtilityLibrary.h"
#include "Poly/GULPolyUtilityLibrary.h"

void UDelaunatorValueUtility::PointFillVisit(
    UDelaunatorObject* Delaunator,
    int32 InitialPoint,
    const TBitArray<>* InVisitedFlags,
    TFunction<void(int32)> InVisitCallback
    )
{
    if (! IsValidDelaunay(Delaunator) ||
        ! Delaunator->GetPoints().IsValidIndex(InitialPoint))
    {
        return;
    }

    const int32 PointCount = Delaunator->GetPointCount();

    // Generate initial visited flags

    TBitArray<> VisitedFlags;

    if (InVisitedFlags && InVisitedFlags->Num() == PointCount)
    {
        VisitedFlags = *InVisitedFlags;
    }
    else
    {
        VisitedFlags.Init(false, PointCount);
    }

    // Generate visit callback

    TFunction<void(int32)> VisitCallback;

    if (InVisitCallback)
    {
        VisitCallback = [&VisitedFlags, &InVisitCallback](int32 Index)
            {
                VisitedFlags[Index] = true;
                InVisitCallback(Index);
            };
    }
    else
    {
        VisitCallback = [&VisitedFlags](int32 Index)
            {
                VisitedFlags[Index] = true;
            };
    }

    TQueue<int32> VisitQueue;

    VisitQueue.Enqueue(InitialPoint);
    VisitCallback(InitialPoint);

    TArray<int32> NeighbourCells;

    while (! VisitQueue.IsEmpty())
    {
        int32 PointIndex;
        VisitQueue.Dequeue(PointIndex);

        NeighbourCells.Reset();
        Delaunator->GetPointNeighbours(NeighbourCells, PointIndex);

        for (int32 NeighbourCell : NeighbourCells)
        {
            if (! VisitedFlags[NeighbourCell])
            {
                VisitQueue.Enqueue(NeighbourCell);
                VisitCallback(NeighbourCell);
            }
        }
    }
}

void UDelaunatorValueUtility::ExpandPointValueVisit(
    UDelaunatorObject* Delaunator,
    const TArray<int32>& InInitialIndices,
    TFunctionRef<void(int32)> InitialValueCallback,
    TFunctionRef<bool(int32)> ExpandFilterCallback,
    TFunctionRef<void(int32,int32)> ExpandValueCallback,
    const TBitArray<>* InVisitedFlags
    )
{
    if (! IsValidDelaunay(Delaunator))
    {
        return;
    }

    const TArray<FVector2D>& Points(Delaunator->GetPoints());
    const int32 PointCount = Points.Num();

    // Generate initial visited flags

    TBitArray<> VisitedFlags;

    if (InVisitedFlags && InVisitedFlags->Num() == PointCount)
    {
        VisitedFlags = *InVisitedFlags;
    }
    else
    {
        VisitedFlags.Init(false, PointCount);
    }

    // Generate initial visit queue

    TQueue<int32> VisitQueue;

    for (int32 i : InInitialIndices)
    {
        if (Points.IsValidIndex(i))
        {
            VisitedFlags[i] = true;
            VisitQueue.Enqueue(i);
            InitialValueCallback(i);
        }
    }

    // Expand values

    TArray<int32> NeighbourPoints;

    while (! VisitQueue.IsEmpty())
    {
        int32 PointIndex;
        VisitQueue.Dequeue(PointIndex);

        NeighbourPoints.Reset();
        Delaunator->GetPointNeighbours(NeighbourPoints, PointIndex);

        for (int32 ni : NeighbourPoints)
        {
            if (! VisitedFlags[ni] && ExpandFilterCallback(ni))
            {
                VisitedFlags[ni] = true;
                VisitQueue.Enqueue(ni);
                ExpandValueCallback(PointIndex, ni);
            }
        }
    }
}

void UDelaunatorValueUtility::GeneratePointsDepthValues(
    UDelaunatorObject* Delaunator,
    UDelaunatorValueObject* ValueObject,
    const TArray<int32>& InInitialPoints,
    int32 StartDepth,
    UDelaunatorCompareOperatorLogic* CompareOperator
    )
{
    if (! IsValid(ValueObject)        ||
        ! IsValidDelaunay(Delaunator) ||
        ! ValueObject->IsValidElementCount(Delaunator->GetPointCount()))
    {
        return;
    }

    const int32 PointCount = Delaunator->GetPointCount();

    FDelaunatorCompareCallback ExpandFilterCallback(nullptr);

    // Get compare callback from compare operator
    if (IsValid(CompareOperator))
    {
        if (CompareOperator->InitializeOperator(PointCount))
        {
            ExpandFilterCallback = CompareOperator->GetOperator();
        }
    }

    // Invalid compare callback, generate default
    if (! ExpandFilterCallback)
    {
        ExpandFilterCallback = [](int32 Index){ return true; };
    }

    TFunction<void(int32)> InitialValueCallback(
        [ValueObject, StartDepth](int32 Index)
        {
            ValueObject->SetValueInt32(Index, StartDepth);
        } );

    TFunction<void(int32,int32)> ExpandValueCallback(
        [ValueObject, StartDepth](int32 Index, int32 NeighbourIndex)
        {
            int32 NextDepth = ValueObject->GetValueInt32(Index) + 1;
            ValueObject->SetValueInt32(NeighbourIndex, NextDepth);
        } );

    ExpandPointValueVisit(
        Delaunator,
        InInitialPoints,
        InitialValueCallback,
        ExpandFilterCallback,
        ExpandValueCallback
        );
}

void UDelaunatorValueUtility::ExpandPointValues(
    UDelaunatorObject* Delaunator,
    UDelaunatorValueObject* ValueObject,
    const TArray<int32>& InInitialPoints,
    UDelaunatorCompareOperatorLogic* CompareOperator
    )
{
    if (! IsValid(ValueObject)        ||
        ! IsValidDelaunay(Delaunator) ||
        ! ValueObject->IsValidElementCount(Delaunator->GetPointCount()))
    {
        return;
    }

    const int32 PointCount = Delaunator->GetPointCount();

    FDelaunatorCompareCallback ExpandFilterCallback(nullptr);

    // Get compare callback from compare operator
    if (IsValid(CompareOperator))
    {
        if (CompareOperator->InitializeOperator(PointCount))
        {
            ExpandFilterCallback = CompareOperator->GetOperator();
        }
    }

    // Invalid compare callback, generate default
    if (! ExpandFilterCallback)
    {
        ExpandFilterCallback = [](int32 Index){ return true; };
    }

    TFunction<void(int32,int32)> ExpandValueCallback(
        [ValueObject](int32 Index, int32 NeighbourIndex)
        {
            int32 Value = ValueObject->GetValueInt32(Index);
            ValueObject->SetValueInt32(NeighbourIndex, Value);
        } );

    ExpandPointValueVisit(
        Delaunator,
        InInitialPoints,
        [](int32 i){},
        ExpandFilterCallback,
        ExpandValueCallback
        );
}

void UDelaunatorValueUtility::GetRandomFilteredPointsWithinRadius(
    UDelaunatorObject* Delaunator,
    TArray<int32>& OutPointIndices,
    int32 RandomSeed,
    const TArray<int32>& InPointIndices,
    float InRadiusBetweenPoints,
    int32 MaxOutputCount,
    UDelaunatorCompareOperatorLogic* CompareOperator
    )
{
    OutPointIndices.Reset();

    if (! IsValidDelaunay(Delaunator) ||
        InPointIndices.Num() < 1)
    {
        return;
    }

    const TArray<FVector2D>& Points(Delaunator->GetPoints());
    const int32 PointCount = Points.Num();

    FDelaunatorCompareCallback CompareCallback(nullptr);

    // Get compare callback from compare operator
    if (IsValid(CompareOperator))
    {
        if (CompareOperator->InitializeOperator(PointCount))
        {
            CompareCallback = CompareOperator->GetOperator();
        }
    }

    // Invalid compare callback, generate default
    if (! CompareCallback)
    {
        CompareCallback = [](int32 Index){ return true; };
    }

    TArray<int32> CandidatePointIndices(InPointIndices);

    float FilterRadius = FMath::Max(0.f, InRadiusBetweenPoints);
    float FilterRadiusSq = FilterRadius*FilterRadius;

    FRandomStream Rand(RandomSeed);

    while (CandidatePointIndices.Num() > 0)
    {
        int32 RandomIndex = Rand.RandHelper(CandidatePointIndices.Num());
        int32 CandidatePointIndex = CandidatePointIndices[RandomIndex];

        CandidatePointIndices.RemoveAtSwap(RandomIndex, 1, false);

        // Skip invalid candidate point index
        if (! Points.IsValidIndex(CandidatePointIndex) ||
            ! CompareCallback(CandidatePointIndex))
        {
            continue;
        }

        // Check whether candidate point
        // is outside of other output points filter radius

        const FVector2D& CandidatePoint(Points[CandidatePointIndex]);
        bool bIsValidPoint = true;

        for (int32 pi : OutPointIndices)
        {
            if ((CandidatePoint-Points[pi]).SizeSquared() < FilterRadiusSq)
            {
                bIsValidPoint = false;
                break;
            }
        }

        if (bIsValidPoint)
        {
            OutPointIndices.Emplace(CandidatePointIndex);
        }

        // Max output reached, break
        if (MaxOutputCount > 0 && OutPointIndices.Num() >= MaxOutputCount)
        {
            break;
        }
    }
}

void UDelaunatorValueUtility::FindSegmentIntersectCells(
    UDelaunatorVoronoi* Voronoi,
    TArray<int32>& OutCells,
    const FVector2D& TargetPoint0,
    const FVector2D& TargetPoint1,
    int32 InitialPoint
    )
{
    OutCells.Reset();

    if (! IsValidVoronoi(Voronoi) ||
        (TargetPoint1-TargetPoint0).SizeSquared() < KINDA_SMALL_NUMBER)
    {
        return;
    }

    UDelaunatorObject* Delaunator = Voronoi->GetDelaunay();

    // Find initial point cell index
    const int32 CellIndex = Delaunator->FindPoint(TargetPoint0, InitialPoint);

    // Invalid starting point, abort
    if (CellIndex < 0)
    {
        return;
    }

    OutCells.Emplace(CellIndex);

    TArray<FVector2D> CellPoints;
    TArray<int32> NeighbourCells;

    int32 PrevIndex = -1;
    int32 NextIndex = CellIndex;

    enum { SEARCH_LIMIT = 1000 };

    for (int32 It=0; It<SEARCH_LIMIT; ++It)
    {
        CellPoints.Reset();
        NeighbourCells.Reset();

        // Get cell half-edge segments and corresponding neighbours

        Voronoi->GetCellPoints(CellPoints, NeighbourCells, NextIndex);

        if (CellPoints.Num() < 2)
        {
            break;
        }

        check(CellPoints.Num() == NeighbourCells.Num());

        // Find cell segments intersection towards target point

        FVector2D P0;
        FVector2D P1 = CellPoints.Last();
        int32 CurrentIndex = NextIndex;

        NextIndex = -1;

        for (int32 ci=0; ci<CellPoints.Num(); ++ci)
        {
            P0 = P1;
            P1 = CellPoints[ci];

            // Skip previous cell
            if (NeighbourCells[ci] == PrevIndex)
            {
                continue;
            }

            if (UGULGeometryUtility::SegmentIntersection2DFast(
                P0,
                P1,
                TargetPoint0,
                TargetPoint1
                ) )
            {
                NextIndex = NeighbourCells[ci];
                break;
            }
        }

        PrevIndex = CurrentIndex;

        // No intersection found, break
        if (NextIndex < 0)
        {
            break;
        }

        OutCells.Emplace(NextIndex);
    }
}

void UDelaunatorValueUtility::FindPolyIntersectCells(
    UDelaunatorVoronoi* Voronoi,
    TArray<int32>& OutCells,
    const TArray<FVector2D>& InPolyPoints,
    int32 InitialPoint
    )
{
    OutCells.Reset();

    const int32 PolyPointCount = InPolyPoints.Num();

    if (! IsValidVoronoi(Voronoi) ||
        PolyPointCount < 3)
    {
        return;
    }

    UDelaunatorObject* Delaunator = Voronoi->GetDelaunay();

    // Find initial cell

    int32 InitialCell = Delaunator->FindPoint(InPolyPoints[0], InitialPoint);

    // Invalid starting point, abort
    if (InitialCell < 0)
    {
        return;
    }

    // Find initial target points for either open / closed input poly points

    int32 EndPolyIt = InPolyPoints[0].Equals(InPolyPoints.Last())
        ? PolyPointCount-1
        : PolyPointCount;

    FVector2D TargetPoint0;
    FVector2D TargetPoint1(InPolyPoints[EndPolyIt-1]);

    OutCells.Emplace(InitialCell);

    // Get poly segments intersecting cells
    for (int32 pi=0; pi<EndPolyIt; ++pi)
    {
        TargetPoint0 = TargetPoint1;
        TargetPoint1 = InPolyPoints[pi];

        // Coincident segment points, skip
        if ((TargetPoint1-TargetPoint0).SizeSquared() < KINDA_SMALL_NUMBER)
        {
            continue;
        }

        // Find poly segment intersecting cells

        TArray<int32> SegmentCells;

        FindSegmentIntersectCells(
            Voronoi,
            SegmentCells,
            TargetPoint0,
            TargetPoint1,
            OutCells.Last()
            );

        if (SegmentCells.Num() > 0)
        {
            // Remove last output cell if it is the same
            // as the first segment cell
            if (OutCells.Last() == SegmentCells[0])
            {
                OutCells.Pop(false);
            }

            // Append segment cells as output
            OutCells.Append(MoveTemp(SegmentCells));
        }
    }
}

void UDelaunatorValueUtility::MarkCellsWithinIndexedPolyGroups(
    UDelaunatorVoronoi* Voronoi,
    UDelaunatorValueObject* ValueObject,
    TArray<FGULIntGroup>& OutBoundaryCellGroups,
    const TArray<FGULIndexedPolyGroup>& InIndexGroups,
    const TArray<FGULVector2DGroup>& InPolyGroups
    )
{
    OutBoundaryCellGroups.Reset();

    if (! IsValidVoronoi(Voronoi) ||
        InIndexGroups.Num() < 1   ||
        InPolyGroups.Num() < 1)
    {
        return;
    }

    UDelaunatorObject* Delaunator = Voronoi->GetDelaunay();

    const TArray<FVector2D>& Points(Delaunator->GetPoints());
    const int32 CellCount = Points.Num();

    // Marked cell flags
    TBitArray<> MarkedCells;
    MarkedCells.Init(false, CellCount);

    // Get mark cell callback from value object

    TFunction<void(int32)> MarkCallback;

    if (IsValid(ValueObject) &&
        ValueObject->IsValidElementCount(CellCount))
    {
        MarkCallback = [ValueObject, &MarkedCells](int32 Index)
            {
                ValueObject->SetValueUInt8(Index, 1);
                MarkedCells[Index] = true;
            };
    }
    // Value object is invalid, use default mark callback
    else
    {
        MarkCallback = [&MarkedCells](int32 Index)
            {
                MarkedCells[Index] = true;
            };
    }

    // Generate boundary cells

    const int32 PolyGroupCount = InPolyGroups.Num();

    OutBoundaryCellGroups.Reset(PolyGroupCount);
    OutBoundaryCellGroups.SetNum(PolyGroupCount);

    for (int32 pgi=0; pgi<PolyGroupCount; ++pgi)
    {
        TArray<int32>& BoundaryCells(OutBoundaryCellGroups[pgi].Values);

        FindPolyIntersectCells(Voronoi, BoundaryCells, InPolyGroups[pgi].Points);

        // Mark boundary cells
        for (int32 BoundaryCell : BoundaryCells)
        {
            MarkCallback(BoundaryCell);
        }
    }

    // Visit all boundary cell neighbours within indexed poly groups

    for (const FGULIntGroup& BoundaryCellGroup : OutBoundaryCellGroups)
    {
        const TArray<int32>& BoundaryCells(BoundaryCellGroup.Values);
        TArray<int32> NeighbourCells;

        for (int32 BoundaryCell : BoundaryCells)
        {
            NeighbourCells.Reset();
            Delaunator->GetPointNeighbours(NeighbourCells, BoundaryCell);

            for (int32 NeighbourCell : NeighbourCells)
            {
                // Skip visited cells
                if (MarkedCells[NeighbourCell])
                {
                    continue;
                }

                // Mark cell visit without set value on value object
                MarkedCells[NeighbourCell] = true;

                // Check whether neighbour cell is on poly
                if (UGULPolyUtilityLibrary::IsPointOnPoly(
                    Points[NeighbourCell],
                    InIndexGroups,
                    InPolyGroups
                    ) )
                {
                    // Point fill cell with marked cells as boundary
                    PointFillVisit(
                        Delaunator,
                        NeighbourCell,
                        &MarkedCells,
                        MarkCallback
                        );
                }
            }
        }
    }
}

void UDelaunatorValueUtility::GetCellsOuterPoints(
    UDelaunatorVoronoi* Voronoi,
    TArray<FVector2D>& OutPoints,
    const TArray<int32>& InCells
    )
{
    OutPoints.Reset();

    if (! IsValidVoronoi(Voronoi) ||
        InCells.Num() < 1)
    {
        return;
    }

    UDelaunatorObject* Delaunator = Voronoi->GetDelaunay();

    const TArray<int32>& Triangles(Delaunator->GetTriangles());
    const TArray<int32>& HalfEdges(Delaunator->GetHalfEdges());
    const TArray<int32>& Inedges(Delaunator->GetInedges());
    const TArray<FVector2D>& Circumcenters(Voronoi->GetCircumcenters());

    const int32 CellCount = InCells.Num();

    for (int32 i=0; i<CellCount; ++i)
    {
        const int32 i0 = InCells[i];
        const int32 i1 = InCells[(i+1) % CellCount];

#if 0
        TArray<int32> NeighbourIndices;
        TArray<int32> NeighbourTriangles;

        Delaunator->GetPointNeighbours(NeighbourIndices, NeighbourTriangles, i0);

        const int32 nn = NeighbourIndices.Num();

        for (int32 j=0; j<nn; ++j)
        {
            int32 ni = NeighbourIndices[j];

            if (ni == i1)
            {
                int32 t0 = NeighbourTriangles[j];
                int32 t1 = NeighbourTriangles[j>0?j-1:nn-1];
                OutPoints.Emplace(Circumcenters[t0]);
                OutPoints.Emplace(Circumcenters[t1]);
                break;
            }
        }
#else
        const int32 e0 = Inedges[i0];

        // coincident point, skip
        if (e0 == -1)
        {
            continue;
        }

        // Iterate over point triangles

        int32 e = e0;
        do
        {
            const int32 t = e / 3;
            const int32 f = t * 3;

            if (Triangles[e] == i1)
            {
                const FVector2D& P0(Delaunator->GetPoints()[i0]);
                const FVector2D& P1(Delaunator->GetPoints()[i1]);

                FVector2D P01 = P1-P0;
                FVector2D P0T = Circumcenters[t]-P0;

                P01.Set(-P01.Y, P01.X);

                int32 e1 = ((e-f) < 2) ? e+1 : f;

                // Prev point
                OutPoints.Emplace(Circumcenters[HalfEdges[e ]/3]);

                // Curr point
                OutPoints.Emplace(Circumcenters[t]);

                // Next point
                OutPoints.Emplace(Circumcenters[HalfEdges[e1]/3]);

                break;
            }

            e = ((e-f) < 2) ? e+1 : f;

            // Ensure sane triangulation
            check(i0 == Triangles[e]);

            e = HalfEdges[e];
        }
        while (e != e0 && e != -1);
#endif

        break;
    }
}
