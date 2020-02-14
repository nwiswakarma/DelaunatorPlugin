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

#pragma once

#include "CoreMinimal.h"
#include "DelaunatorObject.h"
#include "GULTypes.h"
#include "Poly/GULPolyTypes.h"
#include "DelaunatorVoronoi.generated.h"

UCLASS(BlueprintType)
class DELAUNATORPLUGIN_API UDelaunatorVoronoi : public UObject
{
    GENERATED_BODY()

    TArray<FVector2D> Circumcenters;
    TArray<float> HullVectors;

    UPROPERTY()
    UDelaunatorObject* Delaunator;

public:

    int32 GetCellCount() const;
    const TArray<FVector2D>& GetCircumcenters() const;

    void GetCellPoints(TArray<FVector2D>& OutPoints, int32 CellIndex) const;
    void GetCellPoints(TArray<int32>& OutNeighbourIndices, int32 CellIndex) const;
    void GetCellPoints(TArray<FVector2D>& OutPoints, TArray<int32>& OutNeighbourIndices, int32 CellIndex) const;

    void GetAllCellPoints(TArray<FGULVector2DGroup>& OutPointGroups) const;
    void GetCellPointsByPointIndices(TArray<FGULVector2DGroup>& OutPointGroups, const TArray<int32>& InPointIndices) const;

    bool HasValidDelaunatorObject() const;
    bool IsValidVoronoiObject() const;
    const UDelaunatorObject* GetDelaunay() const;

    void Update();
    void GenerateFrom(UDelaunatorObject* InDelaunator);

    // Query Utility

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get Circumcenters"))
    const TArray<FVector2D>& K2_GetCircumcenters();

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get Cell Points"))
    void K2_GetCellPoints(TArray<FVector2D>& OutPoints, int32 PointIndex);

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get Cell Points And Neighbours"))
    void K2_GetCellPointsAndNeighbours(TArray<FVector2D>& OutPoints, TArray<int32>& OutPointIndices, int32 PointIndex);

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get All Cell Points"))
    void K2_GetAllCellPoints(TArray<FGULVector2DGroup>& OutPointGroups);

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get Cell Points By Point Indices"))
    void K2_GetCellPointsByPointIndices(TArray<FGULVector2DGroup>& OutPointGroups, const TArray<int32>& InPointIndices);

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Find Segment Intersect Cells"))
    void K2_FindSegmentIntersectCells(
        TArray<int32>& OutCells,
        const FVector2D& TargetPoint0,
        const FVector2D& TargetPoint1,
        int32 InitialPoint = -1
        );

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Find Poly Intersect Cells"))
    void K2_FindPolyIntersectCells(
        TArray<int32>& OutCells,
        const TArray<FVector2D>& InPoints,
        int32 InitialPoint = -1
        );

    void FindSegmentIntersectCells(
        TArray<int32>& OutCells,
        const FVector2D& TargetPoint0,
        const FVector2D& TargetPoint1,
        int32 InitialPoint = -1
        ) const;

    void FindPolyIntersectCells(
        TArray<int32>& OutCells,
        const TArray<FVector2D>& InPoints,
        int32 InitialPoint = -1
        ) const;

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Mark Cells Within Indexed Poly Groups"))
    void K2_MarkCellsWithinIndexedPolyGroups(
        UDelaunatorValueObject* ValueObject,
        TArray<FGULIntGroup>& OutBoundaryCellGroups,
        const TArray<FGULIndexedPolyGroup>& InIndexGroups,
        const TArray<FGULVector2DGroup>& InPolyGroups
        );

    void MarkCellsWithinIndexedPolyGroups(
        UDelaunatorValueObject* ValueObject,
        TArray<FGULIntGroup>& OutBoundaryCellGroups,
        const TArray<FGULIndexedPolyGroup>& InIndexGroups,
        const TArray<FGULVector2DGroup>& InPolyGroups
        ) const;

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Find Cell Within Boundary Cells"))
    int32 K2_FindCellWithinBoundaryCells(const TArray<int32>& InBoundaryCells);
    int32 FindCellWithinBoundaryCells(const TArray<int32>& InBoundaryCells) const;

    // Value Utility

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    UDelaunatorValueObject* CreateDefaultCellValueObject(
        FName ValueName,
        TSubclassOf<UDelaunatorValueObject> ValueType
        );
};

FORCEINLINE bool UDelaunatorVoronoi::HasValidDelaunatorObject() const
{
    return IsValid(Delaunator)
        && Delaunator->IsValidDelaunatorObject();
}

FORCEINLINE bool UDelaunatorVoronoi::IsValidVoronoiObject() const
{
    return HasValidDelaunatorObject()
        && Circumcenters.Num()*3 == Delaunator->GetIndexCount();
}

FORCEINLINE const UDelaunatorObject* UDelaunatorVoronoi::GetDelaunay() const
{
    return Delaunator;
}

// Query Utility

FORCEINLINE int32 UDelaunatorVoronoi::GetCellCount() const
{
    return HasValidDelaunatorObject() ? Delaunator->GetPointCount() : 0;
}

FORCEINLINE const TArray<FVector2D>& UDelaunatorVoronoi::GetCircumcenters() const
{
    return Circumcenters;
}

FORCEINLINE const TArray<FVector2D>& UDelaunatorVoronoi::K2_GetCircumcenters()
{
    return GetCircumcenters();
}

FORCEINLINE void UDelaunatorVoronoi::K2_GetCellPoints(TArray<FVector2D>& OutPoints, int32 PointIndex)
{
    if (HasValidDelaunatorObject())
    {
        GetCellPoints(OutPoints, PointIndex);
    }
}

FORCEINLINE void UDelaunatorVoronoi::K2_GetCellPointsAndNeighbours(TArray<FVector2D>& OutPoints, TArray<int32>& OutNeighbourIndices, int32 PointIndex)
{
    if (HasValidDelaunatorObject())
    {
        GetCellPoints(OutPoints, OutNeighbourIndices, PointIndex);
    }
}

FORCEINLINE void UDelaunatorVoronoi::K2_GetAllCellPoints(TArray<FGULVector2DGroup>& OutPointGroups)
{
    GetAllCellPoints(OutPointGroups);
}

FORCEINLINE void UDelaunatorVoronoi::K2_GetCellPointsByPointIndices(TArray<FGULVector2DGroup>& OutPointGroups, const TArray<int32>& InPointIndices)
{
    GetCellPointsByPointIndices(OutPointGroups, InPointIndices);
}

FORCEINLINE_DEBUGGABLE void UDelaunatorVoronoi::GetCellPoints(TArray<int32>& OutNeighbourIndices, int32 CellIndex) const
{
    check(HasValidDelaunatorObject());
    Delaunator->GetPointNeighbours(OutNeighbourIndices, CellIndex);
}

FORCEINLINE_DEBUGGABLE void UDelaunatorVoronoi::GetCellPoints(TArray<FVector2D>& OutPoints, int32 CellIndex) const
{
    check(HasValidDelaunatorObject());

    TArray<int32> NeighbourIndices;
    TArray<int32> NeighbourTriangles;

    Delaunator->GetPointNeighbours(
        NeighbourIndices,
        NeighbourTriangles,
        CellIndex
        );

    OutPoints.SetNumUninitialized(NeighbourIndices.Num());

    for (int32 i=0; i<NeighbourIndices.Num(); ++i)
    {
        OutPoints[i] = Circumcenters[NeighbourTriangles[i]];
    }
}

FORCEINLINE_DEBUGGABLE void UDelaunatorVoronoi::GetCellPoints(TArray<FVector2D>& OutPoints, TArray<int32>& OutNeighbourIndices, int32 CellIndex) const
{
    check(HasValidDelaunatorObject());

    TArray<int32> NeighbourTriangles;

    Delaunator->GetPointNeighbours(
        OutNeighbourIndices,
        NeighbourTriangles,
        CellIndex
        );

    OutPoints.SetNumUninitialized(OutNeighbourIndices.Num());

    for (int32 i=0; i<OutNeighbourIndices.Num(); ++i)
    {
        OutPoints[i] = Circumcenters[NeighbourTriangles[i]];
    }
}

FORCEINLINE void UDelaunatorVoronoi::K2_FindSegmentIntersectCells(
    TArray<int32>& OutCells,
    const FVector2D& TargetPoint0,
    const FVector2D& TargetPoint1,
    int32 InitialPoint
    )
{
    FindSegmentIntersectCells(
        OutCells,
        TargetPoint0,
        TargetPoint1,
        InitialPoint
        );
}

FORCEINLINE void UDelaunatorVoronoi::K2_FindPolyIntersectCells(
    TArray<int32>& OutCells,
    const TArray<FVector2D>& InPoints,
    int32 InitialPoint
    )
{
    FindPolyIntersectCells(OutCells, InPoints, InitialPoint);
}

FORCEINLINE void UDelaunatorVoronoi::K2_MarkCellsWithinIndexedPolyGroups(
    UDelaunatorValueObject* ValueObject,
    TArray<FGULIntGroup>& OutBoundaryCellGroups,
    const TArray<FGULIndexedPolyGroup>& InIndexGroups,
    const TArray<FGULVector2DGroup>& InPolyGroups
    )
{
    MarkCellsWithinIndexedPolyGroups(
        ValueObject,
        OutBoundaryCellGroups,
        InIndexGroups,
        InPolyGroups
        );
}

FORCEINLINE int32 UDelaunatorVoronoi::K2_FindCellWithinBoundaryCells(const TArray<int32>& InBoundaryCells)
{
    return FindCellWithinBoundaryCells(InBoundaryCells);
}
