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

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DelaunatorCompareOperator.h"
#include "DelaunatorObject.h"
#include "DelaunatorValueObject.h"
#include "DelaunatorVoronoi.h"
#include "DelaunatorValueUtility.generated.h"

UCLASS()
class DELAUNATORPLUGIN_API UDelaunatorValueUtility : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    static bool IsValidDelaunay(UDelaunatorObject* Delaunator);
    static bool IsValidVoronoi(UDelaunatorVoronoi* Voronoi);

public:

    // Delaunay Utility

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Generate Points Depth Values"))
    static void GeneratePointsDepthValues(
        UDelaunatorObject* Delaunator,
        UDelaunatorValueObject* ValueObject,
        const TArray<int32>& InitialPoints,
        int32 StartDepth,
        UDelaunatorCompareOperatorLogic* CompareOperator = nullptr
        );

    // Voronoi Utility

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Find Segment Intersect Cells"))
    static void FindSegmentIntersectCells(
        UDelaunatorVoronoi* Voronoi,
        TArray<int32>& OutCells,
        const FVector2D& TargetPoint0,
        const FVector2D& TargetPoint1,
        int32 InitialPoint = -1
        );

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Find Poly Intersect Cells"))
    static void FindPolyIntersectCells(
        UDelaunatorVoronoi* Voronoi,
        TArray<int32>& OutCells,
        const TArray<FVector2D>& InPolyPoints,
        int32 InitialPoint = -1
        );

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Mark Cells Within Indexed Poly Groups"))
    static void MarkCellsWithinIndexedPolyGroups(
        UDelaunatorVoronoi* Voronoi,
        UDelaunatorValueObject* ValueObject,
        TArray<FGULIntGroup>& OutBoundaryCellGroups,
        const TArray<FGULIndexedPolyGroup>& InIndexGroups,
        const TArray<FGULVector2DGroup>& InPolyGroups
        );
};

FORCEINLINE bool UDelaunatorValueUtility::IsValidDelaunay(UDelaunatorObject* Delaunator)
{
    return IsValid(Delaunator) && Delaunator->IsValidDelaunatorObject();
}

FORCEINLINE bool UDelaunatorValueUtility::IsValidVoronoi(UDelaunatorVoronoi* Voronoi)
{
    return IsValid(Voronoi) && Voronoi->IsValidVoronoiObject();
}
