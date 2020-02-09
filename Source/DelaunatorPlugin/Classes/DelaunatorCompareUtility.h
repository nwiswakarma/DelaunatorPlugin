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
#include "DelaunatorValueObject.h"
#include "DelaunatorCompareUtility.generated.h"

UCLASS()
class DELAUNATORPLUGIN_API UDelaunatorCompareUtility : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintPure, Category="Delaunator", meta=(WorldContext="WorldContextObject"))
    static UDelaunatorCompareOperatorValueObject* CreateUInt8ValueOperator(
        UObject* WorldContextObject,
        uint8 Value
        );

    UFUNCTION(BlueprintPure, Category="Delaunator", meta=(WorldContext="WorldContextObject"))
    static UDelaunatorCompareOperatorValueObject* CreateInt32ValueOperator(
        UObject* WorldContextObject,
        int32 Value
        );

    UFUNCTION(BlueprintPure, Category="Delaunator", meta=(WorldContext="WorldContextObject"))
    static UDelaunatorCompareOperatorValueObject* CreateFloatValueOperator(
        UObject* WorldContextObject,
        float Value
        );

    UFUNCTION(BlueprintPure, Category="Delaunator", meta=(WorldContext="WorldContextObject"))
    static UDelaunatorCompareOperatorValueObject* CreateValueObjectOperator(
        UObject* WorldContextObject,
        UDelaunatorValueObject* ValueObject
        );

    UFUNCTION(BlueprintPure, Category="Delaunator", meta=(WorldContext="WorldContextObject"))
    static UDelaunatorCompareOperatorUnary* CreateUnaryOperator(
        UObject* WorldContextObject,
        UDelaunatorCompareOperator* InputOperator,
        EDelaunatorUnaryCompareOperation OperationType
        );

    UFUNCTION(BlueprintPure, Category="Delaunator", meta=(WorldContext="WorldContextObject"))
    static UDelaunatorCompareOperatorBinary* CreateBinaryOperator(
        UObject* WorldContextObject,
        UDelaunatorCompareOperator* InputOperator0,
        UDelaunatorCompareOperator* InputOperator1,
        EDelaunatorBinaryCompareOperation OperationType
        );
};
