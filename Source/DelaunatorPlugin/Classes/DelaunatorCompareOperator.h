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
#include "DelaunatorValueObject.h"
#include "DelaunatorCompareOperator.generated.h"

class UDelaunatorObject;

typedef TFunction<bool(int32)> FDelaunatorCompareCallback;

UENUM(BlueprintType)
enum class EDelaunatorUnaryCompareOperation : uint8
{
    DELUCO_Unknown,
    DELUCO_IsNonZero,
    DELUCO_IsZero,
    DELUCO_Not,
    DELUCO_Max
};

UENUM(BlueprintType)
enum class EDelaunatorBinaryCompareOperation : uint8
{
    DELBCO_Unknown,
    DELBCO_Equal,
    DELBCO_NotEqual,
    DELBCO_And,
    DELBCO_Or,
    DELBCO_GreaterThan,
    DELBCO_GreaterThanOrEqual,
    DELBCO_LessThan,
    DELBCO_LessThanOrEqual,
    DELBCO_Max
};

UCLASS(BlueprintType, Abstract)
class DELAUNATORPLUGIN_API UDelaunatorCompareOperator : public UObject
{
    GENERATED_BODY()

protected:

    FORCEINLINE virtual void GetResultsImpl(TArray<int32>& OutPointIndices, int32 ElementCount)
    {
        // Blank Implementation
    }

    FORCEINLINE virtual int32 GetResultImpl(int32 ElementCount)
    {
        // Blank Implementation
        return -1;
    }

public:

    FORCEINLINE virtual bool InitializeOperator(int32 ElementCount)
    {
        return false;
    }

    void GetResults(TArray<int32>& OutPointIndices, int32 ElementCount);
    int32 GetResult(int32 ElementCount);
};

UCLASS()
class DELAUNATORPLUGIN_API UDelaunatorCompareOperatorValueObject : public UDelaunatorCompareOperator
{
    GENERATED_BODY()

public:

    UPROPERTY()
    UDelaunatorValueObject* ValueObject;

    virtual bool InitializeOperator(int32 ElementCount);
};

UCLASS()
class DELAUNATORPLUGIN_API UDelaunatorCompareOperatorLogic : public UDelaunatorCompareOperator
{
    GENERATED_BODY()

protected:

    FDelaunatorCompareCallback Operator;

    virtual void GetResultsImpl(TArray<int32>& OutPointIndices, int32 ElementCount) override;

    virtual int32 GetResultImpl(int32 ElementCount) override;

public:

    FORCEINLINE bool Compare(int32 Index) const
    {
        return Operator(Index);
    }

    FORCEINLINE const FDelaunatorCompareCallback& GetOperator() const
    {
        return Operator;
    }
};

UCLASS()
class DELAUNATORPLUGIN_API UDelaunatorCompareOperatorUnary : public UDelaunatorCompareOperatorLogic
{
    GENERATED_BODY()

public:

    UPROPERTY()
    UDelaunatorCompareOperator* InputOperator;

    UPROPERTY()
    EDelaunatorUnaryCompareOperation OperationType;

    virtual bool InitializeOperator(int32 ElementCount);
};

UCLASS()
class DELAUNATORPLUGIN_API UDelaunatorCompareOperatorBinary : public UDelaunatorCompareOperatorLogic
{
    GENERATED_BODY()

public:

    UPROPERTY()
    UDelaunatorCompareOperator* InputOperator0;

    UPROPERTY()
    UDelaunatorCompareOperator* InputOperator1;

    UPROPERTY()
    EDelaunatorBinaryCompareOperation OperationType;

    virtual bool InitializeOperator(int32 ElementCount);
};
