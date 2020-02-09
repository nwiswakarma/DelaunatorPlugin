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

#include "DelaunatorCompareUtility.h"

UDelaunatorCompareOperatorValueObject* UDelaunatorCompareUtility::CreateUInt8ValueOperator(
    UObject* WorldContextObject,
    uint8 Value
    )
{
    UDelaunatorLiteralValueObject* ValueObject = NewObject<UDelaunatorLiteralValueObject>(WorldContextObject);

    if (! IsValid(ValueObject))
    {
        return nullptr;
    }

    ValueObject->UInt8Value = Value;
    ValueObject->ValueType = EDelaunatorValueType::DELVT_UInt8;

    UDelaunatorCompareOperatorValueObject* OperatorObject = NewObject<UDelaunatorCompareOperatorValueObject>(WorldContextObject);

    if (IsValid(OperatorObject))
    {
        OperatorObject->ValueObject = ValueObject;
    }

    return OperatorObject;
}

UDelaunatorCompareOperatorValueObject* UDelaunatorCompareUtility::CreateInt32ValueOperator(
    UObject* WorldContextObject,
    int32 Value
    )
{
    UDelaunatorLiteralValueObject* ValueObject = NewObject<UDelaunatorLiteralValueObject>(WorldContextObject);

    if (! IsValid(ValueObject))
    {
        return nullptr;
    }

    ValueObject->Int32Value = Value;
    ValueObject->ValueType = EDelaunatorValueType::DELVT_Int32;

    UDelaunatorCompareOperatorValueObject* OperatorObject = NewObject<UDelaunatorCompareOperatorValueObject>(WorldContextObject);

    if (IsValid(OperatorObject))
    {
        OperatorObject->ValueObject = ValueObject;
    }

    return OperatorObject;
}

UDelaunatorCompareOperatorValueObject* UDelaunatorCompareUtility::CreateFloatValueOperator(
    UObject* WorldContextObject,
    float Value
    )
{
    UDelaunatorLiteralValueObject* ValueObject = NewObject<UDelaunatorLiteralValueObject>(WorldContextObject);

    if (! IsValid(ValueObject))
    {
        return nullptr;
    }

    ValueObject->FloatValue = Value;
    ValueObject->ValueType = EDelaunatorValueType::DELVT_Float;

    UDelaunatorCompareOperatorValueObject* OperatorObject = NewObject<UDelaunatorCompareOperatorValueObject>(WorldContextObject);

    if (IsValid(OperatorObject))
    {
        OperatorObject->ValueObject = ValueObject;
    }

    return OperatorObject;
}

UDelaunatorCompareOperatorValueObject* UDelaunatorCompareUtility::CreateValueObjectOperator(
    UObject* WorldContextObject,
    UDelaunatorValueObject* ValueObject
    )
{
    if (! IsValid(ValueObject))
    {
        return nullptr;
    }

    UDelaunatorCompareOperatorValueObject* OperatorObject = NewObject<UDelaunatorCompareOperatorValueObject>(WorldContextObject);

    if (IsValid(OperatorObject))
    {
        OperatorObject->ValueObject = ValueObject;
    }

    return OperatorObject;
}

UDelaunatorCompareOperatorUnary* UDelaunatorCompareUtility::CreateUnaryOperator(
    UObject* WorldContextObject,
    UDelaunatorCompareOperator* InputOperator,
    EDelaunatorUnaryCompareOperation OperationType
    )
{
    UDelaunatorCompareOperatorUnary* OperatorObject = NewObject<UDelaunatorCompareOperatorUnary>(WorldContextObject);

    if (IsValid(OperatorObject))
    {
        OperatorObject->InputOperator = InputOperator;
        OperatorObject->OperationType = OperationType;
    }

    return OperatorObject;
}

UDelaunatorCompareOperatorBinary* UDelaunatorCompareUtility::CreateBinaryOperator(
    UObject* WorldContextObject,
    UDelaunatorCompareOperator* InputOperator0,
    UDelaunatorCompareOperator* InputOperator1,
    EDelaunatorBinaryCompareOperation OperationType
    )
{
    UDelaunatorCompareOperatorBinary* OperatorObject = NewObject<UDelaunatorCompareOperatorBinary>(WorldContextObject);

    if (IsValid(OperatorObject))
    {
        OperatorObject->InputOperator0 = InputOperator0;
        OperatorObject->InputOperator1 = InputOperator1;
        OperatorObject->OperationType = OperationType;
    }

    return OperatorObject;
}
