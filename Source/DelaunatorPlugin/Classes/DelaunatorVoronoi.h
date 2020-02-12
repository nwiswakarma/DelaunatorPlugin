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

    void GetCellPoints(TArray<FVector2D>& OutPoints, int32 TrianglePointIndex) const;
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
    void K2_GetCellPoints(TArray<FVector2D>& OutPoints, int32 TrianglePointIndex);

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get All Cell Points"))
    void K2_GetAllCellPoints(TArray<FGULVector2DGroup>& OutPointGroups);

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Get Cell Points By Point Indices"))
    void K2_GetCellPointsByPointIndices(TArray<FGULVector2DGroup>& OutPointGroups, const TArray<int32>& InPointIndices);

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

inline void UDelaunatorVoronoi::GetCellPoints(TArray<FVector2D>& OutPoints, int32 TrianglePointIndex) const
{
    OutPoints.Reset();

    if (! HasValidDelaunatorObject())
    {
        return;
    }

    const TArray<int32>& InTriangles(Delaunator->GetTriangles());
    const TArray<int32>& InHalfEdges(Delaunator->GetHalfEdges());
    const TArray<int32>& InInedges(Delaunator->GetHalfEdges());

    const int32 t0 = InTriangles[TrianglePointIndex];
    const int32 e0 = InInedges[TrianglePointIndex];

    // coincident point
    if (e0 == -1)
    {
        return;
    }

    // Iterate over point triangles

    int32 e = e0;
    do
    {
        const int32 t = e / 3;
        OutPoints.Emplace(Circumcenters[t]);

        e = ((e%3) == 2) ? e-2 : e+1;

        if (t0 != InTriangles[e])
        {
            // Bad triangulation
            break;
        }

        e = InHalfEdges[e];
    }
    while (e != e0 && e != -1);
}

FORCEINLINE const TArray<FVector2D>& UDelaunatorVoronoi::K2_GetCircumcenters()
{
    return GetCircumcenters();
}

FORCEINLINE void UDelaunatorVoronoi::K2_GetCellPoints(TArray<FVector2D>& OutPoints, int32 TrianglePointIndex)
{
    GetCellPoints(OutPoints, TrianglePointIndex);
}

FORCEINLINE void UDelaunatorVoronoi::K2_GetAllCellPoints(TArray<FGULVector2DGroup>& OutPointGroups)
{
    GetAllCellPoints(OutPointGroups);
}

FORCEINLINE void UDelaunatorVoronoi::K2_GetCellPointsByPointIndices(TArray<FGULVector2DGroup>& OutPointGroups, const TArray<int32>& InPointIndices)
{
    GetCellPointsByPointIndices(OutPointGroups, InPointIndices);
}
