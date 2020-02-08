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
#include "GULTypes.h"
#include "Geom/GULGeometryUtilityLibrary.h"
#include "DelaunatorObject.generated.h"

UCLASS(BlueprintType)
class DELAUNATORPLUGIN_API UDelaunatorObject : public UObject
{
    GENERATED_BODY()

    TArray<FVector2D> Points;
    delaunator::Delaunator Delaunator;

    UPROPERTY()
    TMap<FName, UDelaunatorValueObject*> PointValueMap;

    UPROPERTY()
    TMap<FName, UDelaunatorValueObject*> TriangleValueMap;

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

    int32 FindCornerIndex(int32 TriangleIndex, int32 PointIndex);

    int32 FindSegmentClippingSide(
        const FVector2D& SegmentPoint0,
        const FVector2D& SegmentPoint1,
        int32 TriangleIndex,
        int32 PointIndex
        );

    bool HasOppositeSideSegmentIntersection(
        const FVector2D& SegmentPoint0,
        const FVector2D& SegmentPoint1,
        int32 TriangleIndex,
        int32 PointIndex
        );

public:

    const TArray<FVector2D>& GetPoints() const;
    const TArray<int32>& GetTriangles() const;
    const TArray<int32>& GetHalfEdges() const;

    void FindPointTriangles(
        TArray<int32>& OutTriangleIndices,
        int32 InTrianglePointIndex
        );

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

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get Triangles As Int Vectors"))
    void K2_GetTrianglesAsIntVectors(TArray<FIntVector>& OutTriangles);

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get Filtered Triangles"))
    void K2_GetFilteredTriangles(TArray<int32>& OutTriangles, const TArray<int32>& FilterTriangles);

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    UDelaunatorValueObject* GetPointValueObject(FName ValueName);

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    UDelaunatorValueObject* GetTriangleValueObject(FName ValueName);

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    UDelaunatorValueObject* CreateDefaultPointValueObject(
        UObject* Outer,
        FName ValueName,
        TSubclassOf<UDelaunatorValueObject> ValueType
        );

    UFUNCTION(BlueprintCallable, Category="Delaunator")
    UDelaunatorValueObject* CreateDefaultTriangleValueObject(
        UObject* Outer,
        FName ValueName,
        TSubclassOf<UDelaunatorValueObject> ValueType
        );

    UFUNCTION(BlueprintCallable)
    void FindTrianglesBetweenPoints(
        TArray<int32>& OutTriangleIndices,
        int32 PointIndex0,
        int32 PointIndex1
        );

    UFUNCTION(BlueprintCallable)
    bool FindBoundaryPoints(
        TArray<int32>& OutPointIndices,
        const TArray<int32>& InBoundaryTriangles,
        int32 BoundaryPoint0,
        int32 BoundaryPoint1
        );

    UFUNCTION(BlueprintCallable)
    bool FindPolyBoundaryTriangles(
        TArray<int32>& OutTriangles,
        const TArray<int32>& InPolyPointIndices,
        bool bClosedPoly
        );

    UFUNCTION(BlueprintCallable)
    bool FindPolyGroupsBoundaryTriangles(
        TArray<int32>& OutTriangles,
        const TArray<FGULIntGroup>& InPolyBoundaryGroups,
        bool bClosedPoly
        );
};

FORCEINLINE bool UDelaunatorObject::IsValidDelaunatorObject() const
{
    return Points.Num() >= 3
        && Delaunator.triangles.Num() >= 3
        && Delaunator.triangles.Num() == Delaunator.halfedges.Num();
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

inline void UDelaunatorObject::K2_GetFilteredTriangles(TArray<int32>& OutTriangles, const TArray<int32>& FilterTriangles)
{
    if (IsValidDelaunatorObject())
    {
        const TArray<int32>& InTriangles(GetTriangles());
        bool bValidFilters = true;

        OutTriangles.Reset(FilterTriangles.Num()*3);

        for (int32 ti : FilterTriangles)
        {
            int32 i = ti*3;

            if (! InTriangles.IsValidIndex(i))
            {
                bValidFilters = false;
                break;
            }

            OutTriangles.Emplace(InTriangles[i  ]);
            OutTriangles.Emplace(InTriangles[i+1]);
            OutTriangles.Emplace(InTriangles[i+2]);
        }

        if (! bValidFilters)
        {
            OutTriangles.Empty();
        }
    }
}

FORCEINLINE UDelaunatorValueObject* UDelaunatorObject::GetPointValueObject(FName ValueName)
{
    UDelaunatorValueObject** ValueObjectPtr = PointValueMap.Find(ValueName);
    return ValueObjectPtr && IsValid(*ValueObjectPtr)
        ? *ValueObjectPtr
        : nullptr;
}

FORCEINLINE UDelaunatorValueObject* UDelaunatorObject::GetTriangleValueObject(FName ValueName)
{
    UDelaunatorValueObject** ValueObjectPtr = TriangleValueMap.Find(ValueName);
    return ValueObjectPtr && IsValid(*ValueObjectPtr)
        ? *ValueObjectPtr
        : nullptr;
}

// Internal Utility

FORCEINLINE int32 UDelaunatorObject::GetNextTriCorner(int32 CornerIndex)
{
    return CornerIndex<2 ? CornerIndex+1 : 0;
}

FORCEINLINE int32 UDelaunatorObject::GetNextTriCorner(int32 TriangleIndex, int32 PointIndex)
{
    int32 FlatIndex = TriangleIndex*3;
    return FlatIndex+GetNextTriCorner(PointIndex-FlatIndex);
}

FORCEINLINE int32 UDelaunatorObject::GetPrevTriCorner(int32 CornerIndex)
{
    return CornerIndex>0 ? CornerIndex-1 : 2;
}

FORCEINLINE int32 UDelaunatorObject::GetPrevTriCorner(int32 TriangleIndex, int32 PointIndex)
{
    int32 FlatIndex = TriangleIndex*3;
    return FlatIndex+GetPrevTriCorner(PointIndex-FlatIndex);
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

FORCEINLINE int32 UDelaunatorObject::FindCornerIndex(int32 TriangleIndex, int32 PointIndex)
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

inline int32 UDelaunatorObject::FindSegmentClippingSide(
    const FVector2D& SegmentPoint0,
    const FVector2D& SegmentPoint1,
    int32 TriangleIndex,
    int32 PointIndex
    )
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
    )
{
    const TArray<FVector2D>& InPoints(GetPoints());
    const TArray<int32>& InTriangles(GetTriangles());

    int32 i0 = GetPrevTriCorner(TriangleIndex, PointIndex);
    int32 i1 = GetNextTriCorner(TriangleIndex, PointIndex);

    const FVector2D& P0(InPoints[InTriangles[i0]]);
    const FVector2D& P1(InPoints[InTriangles[i1]]);

    return HasSegmentIntersection(P0, P1, SegmentPoint0, SegmentPoint1);
}
