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
#include "DelaunatorValueObject.generated.h"

class UDelaunatorObject;

UCLASS(BlueprintType, Abstract)
class DELAUNATORPLUGIN_API UDelaunatorValueObject : public UObject
{
    GENERATED_BODY()

    UPROPERTY()
    TWeakObjectPtr<UDelaunatorObject> DelaunatorObject;

public:

    void SetOwner(UDelaunatorObject* InDelaunatorObject);
    virtual void InitializePointValues();
    virtual void InitializeTriangleValues();

    virtual void InitializeValues(int32 ValueCount)
    {
        // Blank Implementation
    }
};

UCLASS()
class DELAUNATORPLUGIN_API UDelaunatorIntValueObject : public UDelaunatorValueObject
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> Values;

    virtual void InitializeValues(int32 ValueCount) override;
};

UCLASS()
class DELAUNATORPLUGIN_API UDelaunatorFloatValueObject : public UDelaunatorValueObject
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> Values;

    virtual void InitializeValues(int32 ValueCount) override;
};
