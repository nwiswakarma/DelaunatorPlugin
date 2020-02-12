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
#include "delaunator/delaunator.hpp"
#include "DelaunatorValueObject.h"
#include "DelaunatorCompareOperator.h"
#include "GULTypes.h"
#include "Geom/GULGeometryUtilityLibrary.h"
#include "DelaunatorObject.generated.h"

class UDelaunatorVoronoi;

UCLASS(BlueprintType)
class DELAUNATORPLUGIN_API UDelaunatorObject : public UObject
{
    GENERATED_BODY()

    delaunator::Delaunator Delaunator;

    TArray<FVector2D> Points;
    TArray<int32> Hull;
    TArray<int32> HullIndex;
    TArray<int32> Inedges;
    TBitArray<> BoundaryFlags;

    UPROPERTY()
    TMap<FName, UDelaunatorValueObject*> ValueMap;

    UPROPERTY()
    TMap<FName, FGULIntGroup> IndexGroupMap;

    static int32 GetNextTriCorner(int32 CornerIndex);
    static int32 GetNextTriCorner(int32 TriangleIndex, int32 PointIndex);

    static int32 GetPrevTriCorner(int32 CornerIndex);
    static int32 GetPrevTriCorner(int32 TriangleIndex, int32 PointIndex);

    static bool HasSegmentIntersection(
        const FVector2D& Point0,
        const FVector2D& Point1,
        const FVector2D& SegmentPoint0,
        const FVector2D& SegmentPoint1
        );

    int32 FindCornerIndex(int32 TriangleIndex, int32 PointIndex) const;

    void GetNeighbourTrianglePointIndex(
        int32& OutNextIndex,
        int32& OutPrevIndex,
        int32 TriangleIndex,
        int32 PointIndex
        ) const;

    int32 FindSegmentClippingSide(
        const FVector2D& SegmentPoint0,
        const FVector2D& SegmentPoint1,
        int32 TriangleIndex,
        int32 PointIndex
        ) const;

    bool HasOppositeSideSegmentIntersection(
        const FVector2D& SegmentPoint0,
        const FVector2D& SegmentPoint1,
        int32 TriangleIndex,
        int32 PointIndex
        ) const;

    void GetPointTrianglesBoundary(TArray<int32>& OutTriangleIndices, int32 InTrianglePointIndex) const;
    void GetPointTrianglesNonBoundary(TArray<int32>& OutTriangleIndices, int32 InTrianglePointIndex) const;

    void GetPointNeighboursBoundary(TArray<int32>& OutNeighbourIndices, int32 InTrianglePointIndex) const;
    void GetPointNeighboursNonBoundary(TArray<int32>& OutNeighbourIndices, int32 InTrianglePointIndex) const;

public:

    const TArray<FVector2D>& GetPoints() const;
    const TArray<int32>& GetTriangles() const;
    const TArray<int32>& GetHalfEdges() const;
    const TArray<int32>& GetInedges() const;
    const TArray<int32>& GetHull() const;
    const TBitArray<>& GetBoundaryFlags() const;

    void GetTriangleIndices(TArray<int32>& OutIndices, const TArray<int32>& InFilterTriangles) const;
    void GetTriangleIndicesFlat(TArray<int32>& OutIndices, const TArray<int32>& InFilterTriangles) const;

    int32 GetTrianglePointIndex(int32 InPointIndex) const;
    void GetPointTriangles(TArray<int32>& OutTriangleIndices, int32 InTrianglePointIndex) const;
    void GetPointNeighbours(TArray<int32>& OutNeighbourIndices, int32 InTrianglePointIndex) const;

    void GetTriangleCenters(TArray<FVector2D>& OutTriangleCenters, const TArray<int32>& InTargetTriangles) const;

    void GetTriangleCircumcenters(TArray<FVector2D>& OutTriangleCenters, const TArray<int32>& InTargetTriangles) const;

    void GeneratePointsDepthValues(
        UDelaunatorValueObject* ValueObject,
        const TArray<int32>& InitialPoints,
        FDelaunatorCompareCallback CompareCallback = nullptr
        ) const;

    void GenerateTrianglesDepthValues(
        UDelaunatorValueObject* ValueObject,
        const TArray<int32>& InitialPoints,
        FDelaunatorCompareCallback CompareCallback = nullptr
        ) const;

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    bool IsValidDelaunatorObject() const;

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    void UpdateFromPoints(const TArray<FVector2D>& InPoints);

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    void CopyIndices(TArray<int32>& OutTriangles, TArray<int32>& OutHalfEdges);

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    int32 GetPointCount() const;

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    int32 GetIndexCount() const;

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    int32 GetTriangleCount() const;

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get Points"))
    const TArray<FVector2D>& K2_GetPoints();

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get Triangles"))
    const TArray<int32>& K2_GetTriangles();

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get Half-Edges"))
    const TArray<int32>& K2_GetHalfEdges();

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get Hull"))
    const TArray<int32>& K2_GetHull();

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get Triangles As Int Vectors"))
    void K2_GetTrianglesAsIntVectors(TArray<FIntVector>& OutTriangles);

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get Triangle Indices"))
    void K2_GetTriangleIndices(TArray<int32>& OutIndices, const TArray<int32>& InFilterTriangles);

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get Triangle Indices (By Flat Indices)"))
    void K2_GetTriangleIndicesFlat(TArray<int32>& OutIndices, const TArray<int32>& InFilterTriangles);

    // Value Generation

    UDelaunatorValueObject* CreateDefaultValueObject(
        UObject* Outer,
        FName ValueName,
        TSubclassOf<UDelaunatorValueObject> ValueType
        );

    FORCEINLINE UDelaunatorValueObject* CreateDefaultValueObject(
        FName ValueName,
        TSubclassOf<UDelaunatorValueObject> ValueType
        )
    {
        return CreateDefaultValueObject(this, ValueName, ValueType);
    }

    UFUNCTION(BlueprintPure, Category="Delaunator")
    UDelaunatorValueObject* GetValueObject(FName ValueName);

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    UDelaunatorValueObject* CreateDefaultPointValueObject(
        FName ValueName,
        TSubclassOf<UDelaunatorValueObject> ValueType
        );

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    UDelaunatorValueObject* CreateDefaultTriangleValueObject(
        FName ValueName,
        TSubclassOf<UDelaunatorValueObject> ValueType
        );

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    void FindPointsByValue(
        TArray<int32>& OutPointIndices,
        UDelaunatorCompareOperator* CompareOperator
        );

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    void FindTrianglesByValue(
        TArray<int32>& OutTriangleIndices,
        UDelaunatorCompareOperator* CompareOperator
        );

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Generate Points Depth Values"))
    void K2_GeneratePointsDepthValues(
        UDelaunatorValueObject* ValueObject,
        const TArray<int32>& InitialPoints,
        UDelaunatorCompareOperatorLogic* CompareOperator = nullptr
        );

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Generate Triangles Depth Values"))
    void K2_GenerateTrianglesDepthValues(
        UDelaunatorValueObject* ValueObject,
        const TArray<int32>& InitialPoints,
        UDelaunatorCompareOperatorLogic* CompareOperator = nullptr
        );

    // Triangles & Points Query

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    void GetTrianglesByPointIndices(
        TArray<int32>& OutTriangles,
        const TArray<int32>& InPointIndices,
        bool bInverseResult = false
        );

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    void GetTrianglesByEdgeIndices(
        TArray<int32>& OutTriangles,
        const TArray<int32>& InPointIndices,
        bool bInverseResult = false
        );

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    void FilterUniquePointIndices(
        TArray<int32>& OutPointIndices,
        const TArray<int32>& InPointIndices
        );

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    void GetHullBoundaryTriangles(TArray<int32>& OutTriangles);

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get Point Triangles"))
    void K2_GetPointTriangles(TArray<int32>& OutTriangleIndices, int32 InTrianglePointIndex);

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get Point Neighbours"))
    void K2_GetPointNeighbours(TArray<int32>& OutNeighbourIndices, int32 InTrianglePointIndex);

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get Triangle Centers"))
    void K2_GetTriangleCenters(TArray<FVector2D>& OutTriangleCenters, const TArray<int32>& InTargetTriangles);

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get Triangle Circumcenters"))
    void K2_GetTriangleCircumcenters(TArray<FVector2D>& OutTriangleCenters, const TArray<int32>& InTargetTriangles);

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get Triangle Point Index (Single)"))
    int32 K2_GetTrianglePointIndex(int32 InPointIndex) const;

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Find Point"))
    int32 K2_FindPoint(const FVector2D& TargetPoint, int32 InitialTrianglePointIndex = 0);
    int32 FindPoint(const FVector2D& TargetPoint, int32 InitialTrianglePointIndex = 0) const;
    int32 FindCloser(int32 i, const FVector2D& TargetPoint) const;

    // Boundary Utility

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    void FindTrianglesBetweenPoints(
        TArray<int32>& OutTriangleIndices,
        int32 PointIndex0,
        int32 PointIndex1
        );

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    bool FindBoundaryPoints(
        TArray<int32>& OutPointIndices,
        const TArray<int32>& InBoundaryTriangles,
        int32 BoundaryPoint0,
        int32 BoundaryPoint1
        );

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    bool FindPolyBoundaryTriangles(
        TArray<int32>& OutTriangles,
        const TArray<int32>& InPolyPointIndices,
        bool bClosedPoly = false,
        bool bAllowDirectConnection = false
        );

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    bool FindPolyGroupsBoundaryTriangles(
        TArray<int32>& OutTriangles,
        const TArray<FGULIntGroup>& InPolyBoundaryGroups,
        bool bClosedPoly = false,
        bool bAllowDirectConnection = false
        );

    // Voronoi Utility

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    UDelaunatorVoronoi* GenerateVoronoiDual();
};

FORCEINLINE bool UDelaunatorObject::IsValidDelaunatorObject() const
{
    return Points.Num() >= 3
        && Hull.Num() >= 3
        && Delaunator.triangles.Num() >= 3
        && Delaunator.triangles.Num() == Delaunator.halfedges.Num()
        && Inedges.Num() == Points.Num();
}

FORCEINLINE int32 UDelaunatorObject::GetPointCount() const
{
    return Points.Num();
}

FORCEINLINE int32 UDelaunatorObject::GetIndexCount() const
{
    return Delaunator.triangles.Num();
}

FORCEINLINE int32 UDelaunatorObject::GetTriangleCount() const
{
    return GetIndexCount()/3;
}

FORCEINLINE const TArray<FVector2D>& UDelaunatorObject::GetPoints() const
{
    return Points;
}

FORCEINLINE const TArray<int32>& UDelaunatorObject::GetTriangles() const
{
    return Delaunator.triangles;
}

FORCEINLINE const TArray<int32>& UDelaunatorObject::GetHalfEdges() const
{
    return Delaunator.halfedges;
}

FORCEINLINE const TArray<int32>& UDelaunatorObject::GetInedges() const
{
    return Inedges;
}

FORCEINLINE const TArray<int32>& UDelaunatorObject::GetHull() const
{
    return Hull;
}

FORCEINLINE const TBitArray<>& UDelaunatorObject::GetBoundaryFlags() const
{
    return BoundaryFlags;
}

inline void UDelaunatorObject::GetTriangleIndices(TArray<int32>& OutIndices, const TArray<int32>& InFilterTriangles) const
{
    if (IsValidDelaunatorObject())
    {
        OutIndices.Reserve(OutIndices.Num()+InFilterTriangles.Num()*3);

        const TArray<int32>& InTriangles(GetTriangles());

        for (int32 ti : InFilterTriangles)
        {
            int32 i = ti*3;

            if (InTriangles.IsValidIndex(i))
            {
                OutIndices.Emplace(InTriangles[i  ]);
                OutIndices.Emplace(InTriangles[i+1]);
                OutIndices.Emplace(InTriangles[i+2]);
            }
        }
    }
}

inline void UDelaunatorObject::GetTriangleIndicesFlat(TArray<int32>& OutIndices, const TArray<int32>& InFilterTriangles) const
{
    if (IsValidDelaunatorObject())
    {
        OutIndices.Reserve(OutIndices.Num()+InFilterTriangles.Num()*3);

        const TArray<int32>& InTriangles(GetTriangles());

        for (int32 i : InFilterTriangles)
        {
            if (InTriangles.IsValidIndex(i))
            {
                OutIndices.Emplace(InTriangles[i  ]);
                OutIndices.Emplace(InTriangles[i+1]);
                OutIndices.Emplace(InTriangles[i+2]);
            }
        }
    }
}

FORCEINLINE int32 UDelaunatorObject::GetTrianglePointIndex(int32 InPointIndex) const
{
    return (IsValidDelaunatorObject() && Points.IsValidIndex(InPointIndex))
        ? GetHalfEdges()[GetInedges()[InPointIndex]]
        : -1;
}

inline void UDelaunatorObject::GetPointTrianglesNonBoundary(TArray<int32>& OutTriangleIndices, int32 InTrianglePointIndex) const
{
    const TArray<int32>& InTriangles(GetTriangles());
    const TArray<int32>& InHalfEdges(GetHalfEdges());

    check(IsValidDelaunatorObject());
    check(InTriangles.IsValidIndex(InTrianglePointIndex));

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

    // Ensure boundary edge is not visited
    check(HalfEdge >= 0);
}

inline void UDelaunatorObject::GetPointNeighboursNonBoundary(TArray<int32>& OutNeighbourIndices, int32 InTrianglePointIndex) const
{
    const TArray<int32>& InTriangles(GetTriangles());
    const TArray<int32>& InHalfEdges(GetHalfEdges());

    check(IsValidDelaunatorObject());
    check(InTriangles.IsValidIndex(InTrianglePointIndex));

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

    // Ensure boundary edge is not visited
    check(HalfEdge >= 0);
}

FORCEINLINE void UDelaunatorObject::GetPointTriangles(TArray<int32>& OutTriangleIndices, int32 InTrianglePointIndex) const
{
    if (! IsValidDelaunatorObject() ||
        ! GetTriangles().IsValidIndex(InTrianglePointIndex))
    {
        return;
    }

    int32 PointIndex = GetTriangles()[InTrianglePointIndex];

    check(GetTriangles().IsValidIndex(PointIndex));

    if (BoundaryFlags[PointIndex])
    {
        GetPointTrianglesBoundary(OutTriangleIndices, InTrianglePointIndex);
    }
    else
    {
        GetPointTrianglesNonBoundary(OutTriangleIndices, InTrianglePointIndex);
    }
}

FORCEINLINE void UDelaunatorObject::GetPointNeighbours(TArray<int32>& OutNeighbourIndices, int32 InTrianglePointIndex) const
{
    if (! IsValidDelaunatorObject() ||
        ! GetTriangles().IsValidIndex(InTrianglePointIndex))
    {
        return;
    }

    int32 PointIndex = GetTriangles()[InTrianglePointIndex];

    check(GetTriangles().IsValidIndex(PointIndex));

    if (BoundaryFlags[PointIndex])
    {
        GetPointNeighboursBoundary(OutNeighbourIndices, InTrianglePointIndex);
    }
    else
    {
        GetPointNeighboursNonBoundary(OutNeighbourIndices, InTrianglePointIndex);
    }
}

inline void UDelaunatorObject::GetTriangleCenters(TArray<FVector2D>& OutTriangleCenters, const TArray<int32>& InTargetTriangles) const
{
    if (! IsValidDelaunatorObject())
    {
        return;
    }

    const TArray<int32>& InTriangles(GetTriangles());

    OutTriangleCenters.Reserve(InTriangles.Num());

    for (int32 ti : InTargetTriangles)
    {
        int32 i = ti*3;

        if (InTriangles.IsValidIndex(i))
        {
            const FVector2D& P0(Points[InTriangles[i  ]]);
            const FVector2D& P1(Points[InTriangles[i+1]]);
            const FVector2D& P2(Points[InTriangles[i+2]]);
            OutTriangleCenters.Emplace((P0+P1+P2)/3.f);
        }
    }
}

inline void UDelaunatorObject::GetTriangleCircumcenters(TArray<FVector2D>& OutTriangleCircumcenters, const TArray<int32>& InTargetTriangles) const
{
    if (! IsValidDelaunatorObject())
    {
        return;
    }

    const TArray<int32>& InTriangles(GetTriangles());

    OutTriangleCircumcenters.Reserve(InTriangles.Num());

    for (int32 ti : InTargetTriangles)
    {
        int32 i = ti*3;

        if (InTriangles.IsValidIndex(i))
        {
            const FVector2D& P0(Points[InTriangles[i  ]]);
            const FVector2D& P1(Points[InTriangles[i+1]]);
            const FVector2D& P2(Points[InTriangles[i+2]]);

            const FVector2D P01 = P1 - P0;
            const FVector2D P02 = P2 - P0;

            const float bl = P01.SizeSquared();
            const float cl = P02.SizeSquared();
            const float d = P01.X * P02.Y - P01.Y * P02.X;

            const float x = P0.X + (P02.Y * bl - P01.Y * cl) * .5f / d;
            const float y = P0.Y + (P01.X * cl - P02.X * bl) * .5f / d;

            OutTriangleCircumcenters.Emplace(x, y);
        }
    }
}

FORCEINLINE const TArray<FVector2D>& UDelaunatorObject::K2_GetPoints()
{
    return GetPoints();
}

FORCEINLINE const TArray<int32>& UDelaunatorObject::K2_GetTriangles()
{
    return GetTriangles();
}

FORCEINLINE const TArray<int32>& UDelaunatorObject::K2_GetHalfEdges()
{
    return GetHalfEdges();
}

FORCEINLINE const TArray<int32>& UDelaunatorObject::K2_GetHull()
{
    return GetHull();
}

inline void UDelaunatorObject::K2_GetTrianglesAsIntVectors(TArray<FIntVector>& OutTriangles)
{
    if (IsValidDelaunatorObject())
    {
        const int32 TriangleCount = GetTriangleCount();

        OutTriangles.SetNumUninitialized(TriangleCount);

        FMemory::Memcpy(
            OutTriangles.GetData(),
            GetTriangles().GetData(),
            TriangleCount*OutTriangles.GetTypeSize()
            );
    }
}

// Query Utility

FORCEINLINE void UDelaunatorObject::FilterUniquePointIndices(
    TArray<int32>& OutPointIndices,
    const TArray<int32>& InPointIndices
    )
{
    OutPointIndices = TSet<int32>(InPointIndices).Array();
}

FORCEINLINE void UDelaunatorObject::GetHullBoundaryTriangles(TArray<int32>& OutTriangles)
{
    if (IsValidDelaunatorObject())
    {
        GetTrianglesByEdgeIndices(OutTriangles, GetHull());
    }
}

FORCEINLINE void UDelaunatorObject::K2_GetPointTriangles(TArray<int32>& OutTriangleIndices, int32 InTrianglePointIndex)
{
    OutTriangleIndices.Reset();
    GetPointTriangles(OutTriangleIndices, InTrianglePointIndex);
}

FORCEINLINE void UDelaunatorObject::K2_GetPointNeighbours(TArray<int32>& OutNeighbourIndices, int32 InTrianglePointIndex)
{
    OutNeighbourIndices.Reset();
    GetPointNeighbours(OutNeighbourIndices, InTrianglePointIndex);
}

FORCEINLINE void UDelaunatorObject::K2_GetTriangleCenters(TArray<FVector2D>& OutTriangleCenters, const TArray<int32>& InTargetTriangles)
{
    OutTriangleCenters.Reset();
    GetTriangleCenters(OutTriangleCenters, InTargetTriangles);
}

FORCEINLINE void UDelaunatorObject::K2_GetTriangleCircumcenters(TArray<FVector2D>& OutTriangleCircumcenters, const TArray<int32>& InTargetTriangles)
{
    OutTriangleCircumcenters.Reset();
    GetTriangleCircumcenters(OutTriangleCircumcenters, InTargetTriangles);
}

FORCEINLINE int32 UDelaunatorObject::K2_GetTrianglePointIndex(int32 InPointIndex) const
{
    return GetTrianglePointIndex(InPointIndex);
}

FORCEINLINE int32 UDelaunatorObject::K2_FindPoint(const FVector2D& TargetPoint, int32 InitialTrianglePointIndex)
{
    return FindPoint(TargetPoint, InitialTrianglePointIndex);
}

inline void UDelaunatorObject::K2_GetTriangleIndices(TArray<int32>& OutIndices, const TArray<int32>& InFilterTriangles)
{
    OutIndices.Reset(InFilterTriangles.Num()*3);

    GetTriangleIndices(OutIndices, InFilterTriangles);

    OutIndices.Shrink();
}

inline void UDelaunatorObject::K2_GetTriangleIndicesFlat(TArray<int32>& OutIndices, const TArray<int32>& InFilterTriangles)
{
    OutIndices.Reset(InFilterTriangles.Num()*3);

    GetTriangleIndicesFlat(OutIndices, InFilterTriangles);

    OutIndices.Shrink();
}

// Value Object Utility

FORCEINLINE UDelaunatorValueObject* UDelaunatorObject::GetValueObject(FName ValueName)
{
    UDelaunatorValueObject** ValueObjectPtr = ValueMap.Find(ValueName);
    return ValueObjectPtr && IsValid(*ValueObjectPtr)
        ? *ValueObjectPtr
        : nullptr;
}

FORCEINLINE void UDelaunatorObject::K2_GeneratePointsDepthValues(
    UDelaunatorValueObject* ValueObject,
    const TArray<int32>& InitialPoints,
    UDelaunatorCompareOperatorLogic* CompareOperator
    )
{
    FDelaunatorCompareCallback CompareCallback(nullptr);

    if (IsValid(CompareOperator))
    {
        CompareCallback = CompareOperator->GetOperator();
    }

    GeneratePointsDepthValues(ValueObject, InitialPoints, CompareCallback);
}

FORCEINLINE void UDelaunatorObject::K2_GenerateTrianglesDepthValues(
    UDelaunatorValueObject* ValueObject,
    const TArray<int32>& InitialPoints,
    UDelaunatorCompareOperatorLogic* CompareOperator
    )
{
    FDelaunatorCompareCallback CompareCallback(nullptr);

    if (IsValid(CompareOperator))
    {
        CompareCallback = CompareOperator->GetOperator();
    }

    GenerateTrianglesDepthValues(ValueObject, InitialPoints, CompareCallback);
}

// Internal Utility

FORCEINLINE int32 UDelaunatorObject::GetNextTriCorner(int32 CornerIndex)
{
    return CornerIndex<2 ? CornerIndex+1 : 0;
}

FORCEINLINE int32 UDelaunatorObject::GetNextTriCorner(int32 TriangleIndex, int32 PointIndex)
{
    int32 FlatTriIndex = TriangleIndex*3;
    return FlatTriIndex+GetNextTriCorner(PointIndex-FlatTriIndex);
}

FORCEINLINE int32 UDelaunatorObject::GetPrevTriCorner(int32 CornerIndex)
{
    return CornerIndex>0 ? CornerIndex-1 : 2;
}

FORCEINLINE int32 UDelaunatorObject::GetPrevTriCorner(int32 TriangleIndex, int32 PointIndex)
{
    int32 FlatTriIndex = TriangleIndex*3;
    return FlatTriIndex+GetPrevTriCorner(PointIndex-FlatTriIndex);
}

FORCEINLINE bool UDelaunatorObject::HasSegmentIntersection(
    const FVector2D& Point0,
    const FVector2D& Point1,
    const FVector2D& SegmentPoint0,
    const FVector2D& SegmentPoint1
    )
{
    return UGULGeometryUtility::SegmentIntersection2DFast(
        Point0,
        Point1,
        SegmentPoint0,
        SegmentPoint1
        );
}

FORCEINLINE int32 UDelaunatorObject::FindCornerIndex(int32 TriangleIndex, int32 PointIndex) const
{
    const TArray<int32>& InTriangles(GetTriangles());

    for (int32 s=0, i=TriangleIndex*3; s<3; ++s)
    {
        if (InTriangles[i+s] == PointIndex)
        {
            return i+s;
        }
    }

    return -1;
}

FORCEINLINE void UDelaunatorObject::GetNeighbourTrianglePointIndex(int32& OutNextIndex, int32& OutPrevIndex, int32 TriangleIndex, int32 PointIndex) const
{
    int32 CornerIndex = FindCornerIndex(TriangleIndex, PointIndex);
    OutNextIndex = GetNextTriCorner(TriangleIndex, CornerIndex);
    OutPrevIndex = GetPrevTriCorner(TriangleIndex, CornerIndex);
}

inline int32 UDelaunatorObject::FindSegmentClippingSide(
    const FVector2D& SegmentPoint0,
    const FVector2D& SegmentPoint1,
    int32 TriangleIndex,
    int32 PointIndex
    ) const
{
    const TArray<FVector2D>& InPoints(GetPoints());
    const TArray<int32>& InTriangles(GetTriangles());

    int32 i0 = PointIndex;
    int32 i1 = GetNextTriCorner(TriangleIndex, i0);

    for (int32 i=0; i<3; ++i)
    {
        const FVector2D& P0(InPoints[InTriangles[i0]]);
        const FVector2D& P1(InPoints[InTriangles[i1]]);

        if (HasSegmentIntersection(P0, P1, SegmentPoint0, SegmentPoint1))
        {
            return i0;
        }

        i0 = i1;
        i1 = GetNextTriCorner(TriangleIndex, i0);
    }

    return -1;
}

FORCEINLINE bool UDelaunatorObject::HasOppositeSideSegmentIntersection(
    const FVector2D& SegmentPoint0,
    const FVector2D& SegmentPoint1,
    int32 TriangleIndex,
    int32 PointIndex
    ) const
{
    const TArray<FVector2D>& InPoints(GetPoints());
    const TArray<int32>& InTriangles(GetTriangles());

    int32 i0 = GetPrevTriCorner(TriangleIndex, PointIndex);
    int32 i1 = GetNextTriCorner(TriangleIndex, PointIndex);

    const FVector2D& P0(InPoints[InTriangles[i0]]);
    const FVector2D& P1(InPoints[InTriangles[i1]]);

    return HasSegmentIntersection(P0, P1, SegmentPoint0, SegmentPoint1);
}
