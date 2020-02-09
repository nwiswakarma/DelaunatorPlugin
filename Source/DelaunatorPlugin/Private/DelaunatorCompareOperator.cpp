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

#include "DelaunatorCompareOperator.h"

void UDelaunatorCompareOperator::GetResults(TArray<int32>& OutPointIndices, int32 ElementCount)
{
    OutPointIndices.Reset(ElementCount);

    bool bValidOperator = InitializeOperator(ElementCount);

    if (bValidOperator)
    {
        GetResultsImpl(OutPointIndices, ElementCount);
    }

    OutPointIndices.Shrink();
}

bool UDelaunatorCompareOperatorValueObject::InitializeOperator(int32 ElementCount)
{
    return IsValid(ValueObject)
        && ValueObject->IsValidElementCount(ElementCount);
}

void UDelaunatorCompareOperatorLogic::GetResultsImpl(TArray<int32>& OutPointIndices, int32 ElementCount)
{
    for (int32 i=0; i<ElementCount; ++i)
    {
        if (Operator(i))
        {
            OutPointIndices.Emplace(i);
        }
    }
}

bool UDelaunatorCompareOperatorUnary::InitializeOperator(int32 ElementCount)
{
    // Invalid input operator, abort
    if (! IsValid(InputOperator))
    {
        return false;
    }

    // Initialize input operator

    bool bValidInputOperator = InputOperator->InitializeOperator(ElementCount);

    if (! bValidInputOperator)
    {
        return false;
    }

    // Initialize operator for value object operator
    if (InputOperator->IsA<UDelaunatorCompareOperatorValueObject>())
    {
        UDelaunatorCompareOperatorValueObject* ValueOperator(
            Cast<UDelaunatorCompareOperatorValueObject>(InputOperator)
            );

        UDelaunatorValueObject* ValueObject = ValueOperator->ValueObject;
        EDelaunatorValueType ValueType = ValueObject->GetValueType();

        switch (ValueType)
        {
            case EDelaunatorValueType::DELVT_UInt8:
            switch (OperationType)
            {
                case EDelaunatorUnaryCompareOperation::DELUCO_IsNonZero:
                    Operator = [ValueObject](int32 Index)
                    {
                        return ValueObject->GetValueUInt8(Index) != 0;
                    };
                    break;

                case EDelaunatorUnaryCompareOperation::DELUCO_IsZero:
                    Operator = [ValueObject](int32 Index)
                    {
                        return ValueObject->GetValueUInt8(Index) == 0;
                    };
                    break;

                case EDelaunatorUnaryCompareOperation::DELUCO_Not:
                    Operator = [ValueObject](int32 Index)
                    {
                        return ! ValueObject->GetValueUInt8(Index);
                    };
                    break;
            }
            break;

            case EDelaunatorValueType::DELVT_Int32:
            switch (OperationType)
            {
                case EDelaunatorUnaryCompareOperation::DELUCO_IsNonZero:
                    Operator = [ValueObject](int32 Index)
                    {
                        return ValueObject->GetValueInt32(Index) != 0;
                    };
                    break;

                case EDelaunatorUnaryCompareOperation::DELUCO_IsZero:
                    Operator = [ValueObject](int32 Index)
                    {
                        return ValueObject->GetValueInt32(Index) == 0;
                    };
                    break;

                case EDelaunatorUnaryCompareOperation::DELUCO_Not:
                    Operator = [ValueObject](int32 Index)
                    {
                        return ! ValueObject->GetValueInt32(Index);
                    };
                    break;
            }
            break;

            case EDelaunatorValueType::DELVT_Float:
            switch (OperationType)
            {
                case EDelaunatorUnaryCompareOperation::DELUCO_IsNonZero:
                    Operator = [ValueObject](int32 Index)
                    {
                        return ! FMath::IsNearlyZero(
                            ValueObject->GetValueFloat(Index)
                            );
                    };
                    break;

                case EDelaunatorUnaryCompareOperation::DELUCO_IsZero:
                    Operator = [ValueObject](int32 Index)
                    {
                        return FMath::IsNearlyZero(
                            ValueObject->GetValueFloat(Index)
                            );
                    };
                    break;

                case EDelaunatorUnaryCompareOperation::DELUCO_Not:
                    Operator = [ValueObject](int32 Index)
                    {
                        return ! FMath::IsNearlyZero(
                            ValueObject->GetValueFloat(Index)
                            );
                    };
                    break;
            }
            break;
        }
    }
    // Initialize operator for logic operator
    else
    if (InputOperator->IsA<UDelaunatorCompareOperatorLogic>())
    {
        UDelaunatorCompareOperatorLogic* LogicOperator(
            Cast<UDelaunatorCompareOperatorLogic>(InputOperator)
            );

        switch (OperationType)
        {
            case EDelaunatorUnaryCompareOperation::DELUCO_Not:
                Operator = [LogicOperator](int32 Index)
                {
                    return ! LogicOperator->Operator(Index);
                };
                break;
        }
    }

    return !!Operator;
}

bool UDelaunatorCompareOperatorBinary::InitializeOperator(int32 ElementCount)
{
    // Invalid input operator, abort
    if (! IsValid(InputOperator0) ||
        ! IsValid(InputOperator1))
    {
        return false;
    }

    // Initialize input operator 0

    bool bValidInputOperator0 = InputOperator0->InitializeOperator(ElementCount);

    if (! bValidInputOperator0)
    {
        return false;
    }

    // Initialize input operator 1

    bool bValidInputOperator1 = InputOperator1->InitializeOperator(ElementCount);

    if (! bValidInputOperator1)
    {
        return false;
    }

    typedef EDelaunatorBinaryCompareOperation ECompOp;

    // Initialize operator for two value object operator
    if (InputOperator0->IsA<UDelaunatorCompareOperatorValueObject>() &&
        InputOperator1->IsA<UDelaunatorCompareOperatorValueObject>())
    {
        UDelaunatorCompareOperatorValueObject* ValueOperator0(
            Cast<UDelaunatorCompareOperatorValueObject>(InputOperator0)
            );

        UDelaunatorCompareOperatorValueObject* ValueOperator1(
            Cast<UDelaunatorCompareOperatorValueObject>(InputOperator1)
            );

        UDelaunatorValueObject* vo0 = ValueOperator0->ValueObject;
        UDelaunatorValueObject* vo1 = ValueOperator1->ValueObject;

        EDelaunatorValueType ValueType0 = vo0->GetValueType();
        EDelaunatorValueType ValueType1 = vo1->GetValueType();

        // Both value objects must have the same value type, otherwise, abort
        if (ValueType0 != ValueType1)
        {
            return false;
        }

        switch (ValueType0)
        {
            case EDelaunatorValueType::DELVT_UInt8:
            switch (OperationType)
            {
                case ECompOp::DELBCO_Equal:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueUInt8(Index)
                            == vo1->GetValueUInt8(Index);
                    };
                    break;

                case ECompOp::DELBCO_NotEqual:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueUInt8(Index)
                            != vo1->GetValueUInt8(Index);
                    };
                    break;

                case ECompOp::DELBCO_And:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueUInt8(Index)
                            && vo1->GetValueUInt8(Index);
                    };
                    break;

                case ECompOp::DELBCO_Or:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueUInt8(Index)
                            || vo1->GetValueUInt8(Index);
                    };
                    break;

                case ECompOp::DELBCO_GreaterThan:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueUInt8(Index)
                            >  vo1->GetValueUInt8(Index);
                    };
                    break;

                case ECompOp::DELBCO_GreaterThanOrEqual:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueUInt8(Index)
                            >= vo1->GetValueUInt8(Index);
                    };
                    break;

                case ECompOp::DELBCO_LessThan:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueUInt8(Index)
                            <  vo1->GetValueUInt8(Index);
                    };
                    break;

                case ECompOp::DELBCO_LessThanOrEqual:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueUInt8(Index)
                            <= vo1->GetValueUInt8(Index);
                    };
                    break;
            }
            break;

            case EDelaunatorValueType::DELVT_Int32:
            switch (OperationType)
            {
                case ECompOp::DELBCO_Equal:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueInt32(Index)
                            == vo1->GetValueInt32(Index);
                    };
                    break;

                case ECompOp::DELBCO_NotEqual:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueInt32(Index)
                            != vo1->GetValueInt32(Index);
                    };
                    break;

                case ECompOp::DELBCO_And:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueInt32(Index)
                            && vo1->GetValueInt32(Index);
                    };
                    break;

                case ECompOp::DELBCO_Or:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueInt32(Index)
                            || vo1->GetValueInt32(Index);
                    };
                    break;

                case ECompOp::DELBCO_GreaterThan:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueInt32(Index)
                            >  vo1->GetValueInt32(Index);
                    };
                    break;

                case ECompOp::DELBCO_GreaterThanOrEqual:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueInt32(Index)
                            >= vo1->GetValueInt32(Index);
                    };
                    break;

                case ECompOp::DELBCO_LessThan:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueInt32(Index)
                            <  vo1->GetValueInt32(Index);
                    };
                    break;

                case ECompOp::DELBCO_LessThanOrEqual:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueInt32(Index)
                            <= vo1->GetValueInt32(Index);
                    };
                    break;
            }
            break;

            case EDelaunatorValueType::DELVT_Float:
            switch (OperationType)
            {
                case ECompOp::DELBCO_Equal:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return FMath::IsNearlyEqual(
                            vo0->GetValueFloat(Index),
                            vo1->GetValueFloat(Index)
                            );
                    };
                    break;

                case ECompOp::DELBCO_NotEqual:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return ! FMath::IsNearlyEqual(
                            vo0->GetValueFloat(Index),
                            vo1->GetValueFloat(Index)
                            );
                    };
                    break;

                case ECompOp::DELBCO_And:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueFloat(Index)
                            && vo1->GetValueFloat(Index);
                    };
                    break;

                case ECompOp::DELBCO_Or:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueFloat(Index)
                            || vo1->GetValueFloat(Index);
                    };
                    break;

                case ECompOp::DELBCO_GreaterThan:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueFloat(Index)
                            >  vo1->GetValueFloat(Index);
                    };
                    break;

                case ECompOp::DELBCO_GreaterThanOrEqual:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueFloat(Index)
                            >= vo1->GetValueFloat(Index);
                    };
                    break;

                case ECompOp::DELBCO_LessThan:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueFloat(Index)
                            <  vo1->GetValueFloat(Index);
                    };
                    break;

                case ECompOp::DELBCO_LessThanOrEqual:
                    Operator = [vo0, vo1](int32 Index)
                    {
                        return vo0->GetValueFloat(Index)
                            <= vo1->GetValueFloat(Index);
                    };
                    break;
            }
            break;
        }
    }
    // Initialize operator for two logic operator
    else
    if (InputOperator0->IsA<UDelaunatorCompareOperatorLogic>() &&
        InputOperator1->IsA<UDelaunatorCompareOperatorLogic>())
    {
        UDelaunatorCompareOperatorLogic* lo0(
            Cast<UDelaunatorCompareOperatorLogic>(InputOperator0)
            );

        UDelaunatorCompareOperatorLogic* lo1(
            Cast<UDelaunatorCompareOperatorLogic>(InputOperator1)
            );

        switch (OperationType)
        {
            case ECompOp::DELBCO_Equal:
                Operator = [lo0, lo1](int32 Index)
                {
                    return lo0->Operator(Index)
                        == lo1->Operator(Index);
                };
                break;

            case ECompOp::DELBCO_NotEqual:
                Operator = [lo0, lo1](int32 Index)
                {
                    return lo0->Operator(Index)
                        != lo1->Operator(Index);
                };
                break;

            case ECompOp::DELBCO_And:
                Operator = [lo0, lo1](int32 Index)
                {
                    return lo0->Operator(Index)
                        && lo1->Operator(Index);
                };
                break;

            case ECompOp::DELBCO_Or:
                Operator = [lo0, lo1](int32 Index)
                {
                    return lo0->Operator(Index)
                        || lo1->Operator(Index);
                };
                break;
        }
    }

    return !!Operator;
}
