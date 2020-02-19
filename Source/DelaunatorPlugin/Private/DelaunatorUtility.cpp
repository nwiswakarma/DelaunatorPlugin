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

#include "DelaunatorUtility.h"
#include "delaunator/delaunator.hpp"
#include "DelaunatorObject.h"
#include "GULMathLibrary.h"

UDelaunatorObject* UDelaunatorUtility::GenerateDelaunatorObject(UObject* Outer, const TArray<FVector2D>& InPoints)
{
    if (InPoints.Num() < 3)
    {
        return nullptr;
    }

    UDelaunatorObject* DelaunatorObject = NewObject<UDelaunatorObject>(Outer);

    if (IsValid(DelaunatorObject))
    {
        DelaunatorObject->UpdateFromPoints(InPoints);
    }

    return DelaunatorObject;
}

void UDelaunatorUtility::GenerateDelaunatorTriangles(TArray<FIntVector>& OutTriangles, const TArray<FVector2D>& InPoints)
{
    const float* PointData = reinterpret_cast<const float*>(InPoints.GetData());
    delaunator::Delaunator Delaunator(PointData, InPoints.Num()*2);

    const int32 TriCount = Delaunator.triangles.Num() / 3;

    OutTriangles.SetNumUninitialized(TriCount);

    for (int32 ti=0; ti<TriCount; ++ti)
    {
        int32 i0 = ti*3;
        int32 i1 = i0+1;
        int32 i2 = i0+2;

        OutTriangles[ti] = FIntVector(
            Delaunator.triangles[i0],
            Delaunator.triangles[i1],
            Delaunator.triangles[i2]
            );
    }
}

void UDelaunatorUtility::GenerateDelaunatorIndices(TArray<int32>& OutTriangles, TArray<int32>& OutHalfEdges, const TArray<FVector2D>& InPoints)
{
    const float* PointData = reinterpret_cast<const float*>(InPoints.GetData());
    delaunator::Delaunator Delaunator(PointData, InPoints.Num()*2);

    check(Delaunator.triangles.Num() == Delaunator.halfedges.Num());

    OutTriangles = MoveTemp(Delaunator.triangles);
    OutHalfEdges = MoveTemp(Delaunator.halfedges);
}

void UDelaunatorUtility::GenerateJitteredGridPoints(
    TArray<FVector2D>& OutPoints,
    int32& OutBoundaryPointOffset,
    FBox2D InBounds,
    float MaxDeviation,
    int32 CellCountPerDimension,
    int32 ExpansionCount
    )
{
    OutPoints.Reset();

    if (CellCountPerDimension < 1   ||
        InBounds.Max.X < InBounds.Min.X ||
        InBounds.Max.Y < InBounds.Min.Y)
    {
        return;
    }

    FBox2D Bounds(InBounds);
    float OX = Bounds.Min.X;
    float OY = Bounds.Min.Y;
    float BoundsW = Bounds.Max.X - Bounds.Min.X;
    float BoundsH = Bounds.Max.Y - Bounds.Min.Y;
    float Spacing;

    if (ExpansionCount > 0)
    {
        int32 TargetCellCount = CellCountPerDimension * CellCountPerDimension;

        // Initial spacing between points before jittering
        Spacing = UGULMathLibrary::ScalePrecision(
            FMath::Sqrt(
                BoundsW*BoundsH / static_cast<float>(TargetCellCount)
            ) );

        // Expand dimension

        Bounds += Bounds.ExpandBy(Spacing*ExpansionCount);

        OX = Bounds.Min.X;
        OY = Bounds.Min.Y;
        BoundsW = Bounds.Max.X - Bounds.Min.X;
        BoundsH = Bounds.Max.Y - Bounds.Min.Y;

        int32 n = CellCountPerDimension+ExpansionCount;
        int32 ExpandCellCount = n*n;

        // Spacing between points before jittering
        Spacing = UGULMathLibrary::ScalePrecision(
            FMath::Sqrt(
                BoundsW*BoundsH / static_cast<float>(ExpandCellCount)
            ) );
    }
    else
    {
        int32 TargetCellCount = CellCountPerDimension * CellCountPerDimension;

        // Spacing between points before jittering
        Spacing = UGULMathLibrary::ScalePrecision(
            FMath::Sqrt(
                BoundsW*BoundsH / static_cast<float>(TargetCellCount)
            ) );
    }

    if (Spacing < KINDA_SMALL_NUMBER)
    {
        return;
    }

    // Generate boundary points
    TArray<FVector2D> BoundaryPoints;
    {
        const int32 Offset = FMath::RoundToInt(-1*Spacing);
        const float w = BoundsW - Offset * 2;
        const float h = BoundsH - Offset * 2;
        const int32 NumX = FMath::CeilToInt(w / (Spacing*2.f)) - 1;
        const int32 NumY = FMath::CeilToInt(h / (Spacing*2.f)) - 1;
        const float InvNumX = 1.f/NumX;
        const float InvNumY = 1.f/NumY;

        BoundaryPoints.Reserve((NumX+NumY)*2);

        for (int32 i=0; i<NumX; ++i)
        {
            float x = w*(i+.5f) * InvNumX + Offset;
            BoundaryPoints.Emplace(OX + x       , OY + Offset  );
            BoundaryPoints.Emplace(OX + x       , OY + Offset+h);
        }

        for (int32 i=0; i<NumY; ++i)
        {
            float y = h*(i+.5f) * InvNumY + Offset;
            BoundaryPoints.Emplace(OX + Offset  , OY + y);
            BoundaryPoints.Emplace(OX + Offset+w, OY + y);
        }
    }

    // Generate jittered points
    TArray<FVector2D> JitteredPoints;
    {
        FRandomStream Rand(0);
        // Cell radius
        const float Radius = Spacing / 2.f;
        // Jitter value
        const float JitterAmount = Radius * MaxDeviation;
        const auto GetJitter(
            [&Rand, JitterAmount]()
            {
                return Rand.GetFraction() * 2 * JitterAmount - JitterAmount;
            } );

        JitteredPoints.Reserve(BoundsW*BoundsH);

        for (float y=Radius; y<BoundsH; y+=Spacing)
        for (float x=Radius; x<BoundsW; x+=Spacing)
        {
            const float px = FMath::Min(
                UGULMathLibrary::ScalePrecision(x+GetJitter()),
                BoundsW
                );

            const float py = FMath::Min(
                UGULMathLibrary::ScalePrecision(y+GetJitter()),
                BoundsH
                );

            JitteredPoints.Emplace(OX+px, OY+py);
        }
    }

    OutBoundaryPointOffset = JitteredPoints.Num();

    OutPoints.Reset(JitteredPoints.Num()+BoundaryPoints.Num());
    OutPoints.Append(MoveTemp(JitteredPoints));
    OutPoints.Append(MoveTemp(BoundaryPoints));
}

void UDelaunatorUtility::GenerateJitteredGridPointsUniform(
    TArray<FVector2D>& OutPoints,
    int32& OutBoundaryPointOffset,
    FBox2D InBounds,
    float MaxDeviation,
    int32 CellCountPerDimension,
    int32 ExpansionCount
    )
{
    checkf(false, TEXT("NO IMPLEMENTATION"));

#if 0
    OutPoints.Reset();

    if (CellCountPerDimension < 1   ||
        InBounds.Max.X < InBounds.Min.X ||
        InBounds.Max.Y < InBounds.Min.Y)
    {
        return;
    }

    FBox2D Bounds(InBounds);
    float OX = Bounds.Min.X;
    float OY = Bounds.Min.Y;
    float BoundsW = Bounds.Max.X - Bounds.Min.X;
    float BoundsH = Bounds.Max.Y - Bounds.Min.Y;
    float SpacingX;
    float SpacingY;

    if (ExpansionCount > 0)
    {
        int32 TargetCellCount = CellCountPerDimension * CellCountPerDimension;
        float CountF = static_cast<float>(CellCountPerDimension);

        // Initial spacing between points before jittering
        SpacingX = UGULMathLibrary::ScalePrecision(BoundsW/CountF);
        SpacingY = UGULMathLibrary::ScalePrecision(BoundsH/CountF);

        // Expand dimension

        FVector2D Expansion(SpacingX*ExpansionCount, SpacingY*ExpansionCount);
        Bounds.Min -= Expansion;
        Bounds.Max += Expansion;

        OX = Bounds.Min.X;
        OY = Bounds.Min.Y;
        BoundsW = Bounds.Max.X - Bounds.Min.X;
        BoundsH = Bounds.Max.Y - Bounds.Min.Y;

        float ExpandCountF = static_cast<float>(CellCountPerDimension+ExpansionCount);

        // Spacing between points before jittering
        SpacingX = UGULMathLibrary::ScalePrecision(BoundsW/ExpandCountF);
        SpacingY = UGULMathLibrary::ScalePrecision(BoundsH/ExpandCountF);
    }
    else
    {
        int32 TargetCellCount = CellCountPerDimension * CellCountPerDimension;
        float CountF = static_cast<float>(CellCountPerDimension);

        // Initial spacing between points before jittering
        SpacingX = UGULMathLibrary::ScalePrecision(BoundsW/CountF);
        SpacingY = UGULMathLibrary::ScalePrecision(BoundsH/CountF);
    }

    if (SpacingX < KINDA_SMALL_NUMBER || SpacingY < KINDA_SMALL_NUMBER)
    {
        return;
    }

    // Generate boundary points
    TArray<FVector2D> BoundaryPoints;
    {
        const int32 OffsetX = FMath::RoundToInt(-1*SpacingX);
        const int32 OffsetY = FMath::RoundToInt(-1*SpacingY);
        const float w = BoundsW - OffsetX * 2;
        const float h = BoundsH - OffsetY * 2;
        const int32 NumX = FMath::CeilToInt(w / (SpacingX*2.f)) - 1;
        const int32 NumY = FMath::CeilToInt(h / (SpacingY*2.f)) - 1;
        const float InvNumX = 1.f/NumX;
        const float InvNumY = 1.f/NumY;

        BoundaryPoints.Reserve((NumX+NumY)*2);

        for (int32 i=0; i<NumX; ++i)
        {
            float x = OffsetX + w*(i+.5f) * InvNumX;
            BoundaryPoints.Emplace(OX + x       , OY + OffsetY  );
            BoundaryPoints.Emplace(OX + x       , OY + OffsetY+h);
        }

        for (int32 i=0; i<NumY; ++i)
        {
            float y = OffsetY + h*(i+.5f) * InvNumY;
            BoundaryPoints.Emplace(OX + OffsetX  , OY + y);
            BoundaryPoints.Emplace(OX + OffsetX+w, OY + y);
        }
    }

    // Generate jittered points
    TArray<FVector2D> JitteredPoints;
    {
        FRandomStream Rand(0);
        // Cell radius
        const float RadiusX = SpacingX / 2.f;
        const float RadiusY = SpacingY / 2.f;
        // Jitter value
        const float JitterAmountX = RadiusX * MaxDeviation;
        const float JitterAmountY = RadiusY * MaxDeviation;
        const auto GetJitterX(
            [&Rand, JitterAmountX]()
            {
                return Rand.GetFraction() * 2 * JitterAmountX - JitterAmountX;
            } );
        const auto GetJitterY(
            [&Rand, JitterAmountY]()
            {
                return Rand.GetFraction() * 2 * JitterAmountY - JitterAmountY;
            } );

        JitteredPoints.Reserve(BoundsW*BoundsH);

        for (float y=RadiusY; y<BoundsH; y+=SpacingX)
        for (float x=RadiusX; x<BoundsW; x+=SpacingY)
        {
            const float px = FMath::Min(
                UGULMathLibrary::ScalePrecision(x+GetJitterX()),
                BoundsW
                );

            const float py = FMath::Min(
                UGULMathLibrary::ScalePrecision(y+GetJitterY()),
                BoundsH
                );

            JitteredPoints.Emplace(OX+px, OY+py);
        }
    }

    OutBoundaryPointOffset = JitteredPoints.Num();

    OutPoints.Reset(JitteredPoints.Num()+BoundaryPoints.Num());
    OutPoints.Append(MoveTemp(JitteredPoints));
    OutPoints.Append(MoveTemp(BoundaryPoints));
#endif
}
