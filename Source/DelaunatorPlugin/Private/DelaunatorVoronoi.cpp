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

void UDelaunatorVoronoi::GetAllCellPoints(TArray<FGULVector2DGroup>& OutPointGroups) const
{
    if (! HasValidDelaunatorObject())
    {
        return;
    }

    const TArray<int32>& InTrianglePointIndices(Delaunator->GetTrianglePointIndices());
    const int32 PointCount = Delaunator->GetPointCount();

    OutPointGroups.SetNum(PointCount);

    for (int32 i=0; i<PointCount; ++i)
    {
        GetCellPoints(OutPointGroups[i].Points, InTrianglePointIndices[i]);
    }
}
