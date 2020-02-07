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

void UDelaunatorUtility::GenerateDelaunatorObject(TArray<FIntVector>& OutTriangles, const TArray<FVector2D>& InPoints)
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

    const SIZE_T TypeSize = OutTriangles.GetTypeSize();
    const int32 IndexNum = Delaunator.triangles.Num();

    //OutTriangles.SetNumUninitialized(IndexNum);
    //OutHalfEdges.SetNumUninitialized(IndexNum);

    //FMemory::Memcpy(OutTriangles.GetData(), Delaunator.triangles.GetData(), IndexNum*TypeSize);
    //FMemory::Memcpy(OutHalfEdges.GetData(), Delaunator.halfedges.GetData(), IndexNum*TypeSize);

    OutTriangles = Delaunator.triangles;
    OutHalfEdges = Delaunator.halfedges;
}
