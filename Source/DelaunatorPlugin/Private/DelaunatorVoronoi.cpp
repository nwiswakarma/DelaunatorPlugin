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

#include "DelaunatorVoronoi.h"
#include "Geom/GULGeometryUtilityLibrary.h"

void UDelaunatorVoronoi::Update()
{
    if (! HasValidDelaunatorObject())
    {
        return;
    }

    const TArray<FVector2D>& InPoints(Delaunator->GetPoints());
    const TArray<int32>& InTriangles(Delaunator->GetTriangles());
    const TArray<int32>& InHull(Delaunator->GetHull());

    const int32 TriangleCount = InTriangles.Num()/3;

    // Compute circumcenters
    
    Circumcenters.SetNumUninitialized(TriangleCount);

    for (int32 ti=0; ti<TriangleCount; ++ti)
    {
        int32 i = ti*3;
        int32 i0 = InTriangles[i  ];
        int32 i1 = InTriangles[i+1];
        int32 i2 = InTriangles[i+2];
        const FVector2D& P0(InPoints[i0]);
        const FVector2D& P1(InPoints[i1]);
        const FVector2D& P2(InPoints[i2]);

        const float dx = P1.X - P0.X;
        const float dy = P1.Y - P0.Y;
        const float ex = P2.X - P0.X;
        const float ey = P2.Y - P0.Y;
        const float bl = dx * dx + dy * dy;
        const float cl = ex * ex + ey * ey;
        const float ab = (dx * ey - dy * ex) * 2;

        float x, y;

        if (!ab)
        {
            // Degenerate case (collinear diagram)
            x = (P0.X + P2.X) / 2.f - KINDA_SMALL_NUMBER * ey;
            y = (P0.Y + P2.Y) / 2.f + KINDA_SMALL_NUMBER * ex;
        }
        else
        if (FMath::Abs(ab) < KINDA_SMALL_NUMBER)
        {
            // Almost equal points (degenerate triangle)
            x = (P0.X + P2.X) / 2.f;
            y = (P0.Y + P2.Y) / 2.f;
        }
        else
        {
            const float d = 1.f / ab;
            x = P0.X + (ey * bl - dy * cl) * d;
            y = P0.Y + (dx * cl - ex * bl) * d;
        }

        Circumcenters[ti] = FVector2D(x, y);
    }

    // Compute exterior cell rays.

    HullVectors.SetNumUninitialized(InHull.Num()*4);

    int32 h = InHull.Num()-1;
    int32 pi0;
    int32 pi1 = h * 4;

    FVector2D P0;
    FVector2D P1 = InPoints[InHull[h]];

    for (int32 i=0; i<InHull.Num(); ++i)
    {
        h = i;
        pi0 = pi1;
        pi1 = h*4;
        P0 = P1;
        P1 = InPoints[InHull[h]];
        HullVectors[pi0+2] = HullVectors[pi1  ] = P0.Y - P1.Y;
        HullVectors[pi0+3] = HullVectors[pi1+1] = P1.X - P0.X;
    }
}

void UDelaunatorVoronoi::GenerateFrom(UDelaunatorObject* InDelaunator)
{
    if (! IsValid(InDelaunator) ||
        ! InDelaunator->IsValidDelaunatorObject())
    {
        return;
    }

    Delaunator = InDelaunator;

    Update();
}

UDelaunatorValueObject* UDelaunatorVoronoi::CreateDefaultCellValueObject(
    FName ValueName,
    TSubclassOf<UDelaunatorValueObject> ValueType
    )
{
    if (! IsValidVoronoiObject() || ValueName.IsNone())
    {
        return nullptr;
    }

    UDelaunatorValueObject* ValueObject = Delaunator->CreateDefaultValueObject(
        this,
        ValueName,
        ValueType
        );

    if (IsValid(ValueObject))
    {
        ValueObject->InitializeValues(GetCellCount());
    }

    return ValueObject;
}

void UDelaunatorVoronoi::GetAllCellPoints(TArray<FGULVector2DGroup>& OutPointGroups) const
{
    if (HasValidDelaunatorObject())
    {
        const int32 PointCount = Delaunator->GetPointCount();

        OutPointGroups.SetNum(PointCount);

        for (int32 i=0; i<PointCount; ++i)
        {
            GetCellPoints(OutPointGroups[i].Points, i);
        }
    }
}

void UDelaunatorVoronoi::GetCellPointsByPointIndices(TArray<FGULVector2DGroup>& OutPointGroups, const TArray<int32>& InPointIndices) const
{
    if (HasValidDelaunatorObject())
    {
        const int32 PointCount = InPointIndices.Num();

        OutPointGroups.SetNum(PointCount);

        for (int32 i=0; i<PointCount; ++i)
        {
            GetCellPoints(
                OutPointGroups[i].Points,
                InPointIndices[i]
                );
        }
    }
}

void UDelaunatorVoronoi::FindSegmentIntersectCells(
    TArray<int32>& OutCells,
    const FVector2D& TargetPoint0,
    const FVector2D& TargetPoint1,
    int32 InitialPoint
    ) const
{
    OutCells.Reset();

    if (! IsValidVoronoiObject())
    {
        return;
    }

    const int32 CellIndex = Delaunator->FindPoint(TargetPoint0, InitialPoint);

    // Invalid starting point, abort
    if (CellIndex < 0)
    {
        return;
    }

    OutCells.Emplace(CellIndex);

    // Coincident segment points, return
    if ((TargetPoint1-TargetPoint0).SizeSquared() < KINDA_SMALL_NUMBER)
    {
        return;
    }

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

        GetCellPoints(CellPoints, NeighbourCells, NextIndex);

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

void UDelaunatorVoronoi::FindPolyIntersectCells(
    TArray<int32>& OutCells,
    const TArray<FVector2D>& InPoints,
    int32 InitialPoint
    ) const
{
    OutCells.Reset();

    const int32 PointCount = InPoints.Num();

    if (! IsValidVoronoiObject() || InPoints.Num() < 3)
    {
        return;
    }

    // Find initial cell

    int32 InitialCell = Delaunator->FindPoint(InPoints[0], InitialPoint);

    // Invalid starting point, abort
    if (InitialCell < 0)
    {
        return;
    }

    // Find initial target points for either open / closed input poly points

    int32 EndPolyIt = InPoints[0].Equals(InPoints.Last())
        ? PointCount-1
        : PointCount;

    FVector2D TargetPoint0;
    FVector2D TargetPoint1(InPoints[EndPolyIt-1]);

    OutCells.Emplace(InitialCell);

    // Get poly segments intersecting cells
    for (int32 pi=0; pi<EndPolyIt; ++pi)
    {
        TargetPoint0 = TargetPoint1;
        TargetPoint1 = InPoints[pi];

        // Coincident segment points, skip
        if ((TargetPoint1-TargetPoint0).SizeSquared() < KINDA_SMALL_NUMBER)
        {
            continue;
        }

        // Find poly segment intersecting cells

        TArray<int32> SegmentCells;

        FindSegmentIntersectCells(
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

void UDelaunatorVoronoi::MarkCellsWithinIndexedPolyGroups(
    UDelaunatorValueObject* ValueObject,
    TArray<FGULIntGroup>& OutBoundaryCellGroups,
    const TArray<FGULIndexedPolyGroup>& InIndexGroups,
    const TArray<FGULVector2DGroup>& InPolyGroups
    ) const
{
    OutBoundaryCellGroups.Reset();

    if (! IsValidVoronoiObject() ||
        InIndexGroups.Num() < 1  ||
        InPolyGroups.Num() < 1)
    {
        return;
    }

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

        FindPolyIntersectCells(BoundaryCells, InPolyGroups[pgi].Points);

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
                    Delaunator->PointFillVisit(
                        NeighbourCell,
                        &MarkedCells,
                        MarkCallback
                        );
                }
            }
        }
    }
}

int32 UDelaunatorVoronoi::FindCellWithinBoundaryCells(const TArray<int32>& InBoundaryCells) const
{
    if (! IsValidVoronoiObject() || InBoundaryCells.Num() < 3)
    {
        return -1;
    }

    const TArray<FVector2D>& InPoints(Delaunator->GetPoints());
    const TSet<int32> BoundaryCellSet(InBoundaryCells);

    const int32 EndCellIt = InBoundaryCells.Num()-1;

    for (int32 i=1; i<EndCellIt; ++i)
    {
        int32 i0 = InBoundaryCells[i-1];
        int32 i1 = InBoundaryCells[i  ];
        int32 i2 = InBoundaryCells[i+1];

        // Skip invalid boundary cell index
        if (! InPoints.IsValidIndex(i0) ||
            ! InPoints.IsValidIndex(i1) ||
            ! InPoints.IsValidIndex(i2))
        {
            continue;
        }

        const FVector2D& P0(InPoints[i0]);
        const FVector2D& P1(InPoints[i1]);
        const FVector2D& P2(InPoints[i2]);

        const FVector2D D01(-(P1.Y-P0.Y), (P1.X-P0.X));
        const FVector2D D12(-(P2.Y-P1.Y), (P2.X-P1.X));

        TArray<int32> NeighbourCells;
        GetCellPoints(NeighbourCells, i1);

        // Find cell neighbour within boundary cells
        for (int32 ni : NeighbourCells)
        {
            // Skip boundary cell
            if (BoundaryCellSet.Contains(ni))
            {
                continue;
            }

            const FVector2D& PC(InPoints[ni]);

            const FVector2D D0C(PC-P0);
            const FVector2D D1C(PC-P1);

            if ((D0C|D01) > 0.f && (D1C|D12) > 0.f)
            {
                return ni;
            }
        }
    }

    return -1;
}
