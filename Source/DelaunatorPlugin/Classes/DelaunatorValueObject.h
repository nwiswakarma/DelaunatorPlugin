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

UENUM(BlueprintType)
enum class EDelaunatorValueType : uint8
{
    DELVT_Unknown,
    DELVT_UInt8,
    DELVT_Int32,
    DELVT_Float,
    DELVT_Max
};

template<typename InContainerType, typename InValueType>
class TDelaunatorValueData
{
public:

    typedef InContainerType ContainerType;
    typedef InValueType     ValueType;

    ContainerType Values;

    // Set default values

    // Array
    template<typename InputContainerType = ContainerType>
    FORCEINLINE typename TEnableIf< !TAreTypesEqual<InputContainerType, TBitArray<>>::Value >::Type SetValues(int32 ValueCount)
    {
        Values.Reset(ValueCount);
        Values.SetNumZeroed(ValueCount);
    }

    // Bit Array
    template<typename InputContainerType = ContainerType>
    FORCEINLINE typename TEnableIf<  TAreTypesEqual<InputContainerType, TBitArray<>>::Value >::Type SetValues(int32 ValueCount)
    {
        Values.Init(false, ValueCount);
    }

    // Set uniform values

    // Array
    template<typename InputContainerType = ContainerType>
    FORCEINLINE typename TEnableIf< !TAreTypesEqual<InputContainerType, TBitArray<>>::Value >::Type SetUniformValue(const ValueType& InValue)
    {
        for (ValueType& Value : Values)
        {
            Value = InValue;
        }
    }

    // Bit Array
    template<typename InputContainerType = ContainerType>
    FORCEINLINE typename TEnableIf<  TAreTypesEqual<InputContainerType, TBitArray<>>::Value >::Type SetUniformValue(const ValueType& InValue)
    {
        Values.Init(InValue, Values.Num());
    }

    // Set values by indices

    FORCEINLINE void SetValues(const TArray<int32>& InIndices, const ValueType& InValue)
    {
        for (int32 PointIndex : InIndices)
        {
            Values[PointIndex] = InValue;
        }

    }
};

UCLASS(BlueprintType, Abstract)
class DELAUNATORPLUGIN_API UDelaunatorValueObject : public UObject
{
    GENERATED_BODY()

public:

    FORCEINLINE virtual void InitializeValues(int32 ValueCount)
    {
        // Blank Implementation
    }

    FORCEINLINE virtual bool IsValidElementCount(int32 InElementCount) const
    {
        return GetElementCount() == InElementCount;
    }

    FORCEINLINE virtual int32 GetElementCount() const
    {
        return -1;
    }

    FORCEINLINE virtual EDelaunatorValueType GetValueType() const
    {
        return EDelaunatorValueType::DELVT_Unknown;
    }

    FORCEINLINE virtual uint8 GetValueUInt8(int32 Index) const
    {
        return 0;
    }

    FORCEINLINE virtual int32 GetValueInt32(int32 Index) const
    {
        return 0;
    }

    FORCEINLINE virtual float GetValueFloat(int32 Index) const
    {
        return 0.f;
    }

    FORCEINLINE virtual void SetValueUInt8(int32 Index, uint8 InValue)
    {
        // Blank Implementation
    }

    FORCEINLINE virtual void SetValueInt32(int32 Index, int32 InValue)
    {
        // Blank Implementation
    }

    FORCEINLINE virtual void SetValueFloat(int32 Index, float InValue)
    {
        // Blank Implementation
    }
};

UCLASS()
class DELAUNATORPLUGIN_API UDelaunatorLiteralValueObject : public UDelaunatorValueObject
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 UInt8Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Int32Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FloatValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDelaunatorValueType ValueType;

    FORCEINLINE virtual bool IsValidElementCount(int32 InElementCount) const override
    {
        return true;
    }

    FORCEINLINE virtual int32 GetElementCount() const override
    {
        return 1;
    }

    FORCEINLINE virtual EDelaunatorValueType GetValueType() const override
    {
        return ValueType;
    }

    FORCEINLINE virtual uint8 GetValueUInt8(int32 Index) const override
    {
        return UInt8Value;
    }

    FORCEINLINE virtual int32 GetValueInt32(int32 Index) const override
    {
        return Int32Value;
    }

    FORCEINLINE virtual float GetValueFloat(int32 Index) const override
    {
        return FloatValue;
    }

    FORCEINLINE virtual void SetValueUInt8(int32 Index, uint8 InValue) override
    {
        UInt8Value = InValue;
    }

    FORCEINLINE virtual void SetValueInt32(int32 Index, int32 InValue) override
    {
        Int32Value = InValue;
    }

    FORCEINLINE virtual void SetValueFloat(int32 Index, float InValue) override
    {
        FloatValue = InValue;
    }
};

UCLASS()
class DELAUNATORPLUGIN_API UDelaunatorBitFlagsValueObject : public UDelaunatorValueObject, public TDelaunatorValueData<TBitArray<>, bool>
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Set Uniform Value"))
    void K2_SetUniformValue(bool InValue);

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Set Values By Indices"))
    void K2_SetValuesByIndices(const TArray<int32>& InIndices, bool InValue);

    FORCEINLINE virtual void InitializeValues(int32 ValueCount) override
    {
        SetValues(ValueCount);
    }

    FORCEINLINE virtual int32 GetElementCount() const override
    {
        return Values.Num();
    }

    FORCEINLINE virtual EDelaunatorValueType GetValueType() const
    {
        return EDelaunatorValueType::DELVT_UInt8;
    }

    FORCEINLINE virtual uint8 GetValueUInt8(int32 Index) const override
    {
        return Values[Index] ? 1 : 0;
    }

    FORCEINLINE virtual int32 GetValueInt32(int32 Index) const override
    {
        return Values[Index] ? 1 : 0;
    }

    FORCEINLINE virtual void SetValueUInt8(int32 Index, uint8 InValue) override
    {
        Values[Index] = static_cast<bool>(InValue);
    }

    FORCEINLINE virtual void SetValueInt32(int32 Index, int32 InValue) override
    {
        Values[Index] = static_cast<bool>(InValue);
    }
};

UCLASS()
class DELAUNATORPLUGIN_API UDelaunatorIntValueObject : public UDelaunatorValueObject, public TDelaunatorValueData<TArray<int32>, int32>
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Set Uniform Value"))
    void K2_SetUniformValue(int32 InValue);

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Set Values By Indices"))
    void K2_SetValuesByIndices(const TArray<int32>& InIndices, int32 InValue);

    FORCEINLINE virtual void InitializeValues(int32 ValueCount) override
    {
        SetValues(ValueCount);
    }

    FORCEINLINE virtual int32 GetElementCount() const override
    {
        return Values.Num();
    }

    FORCEINLINE virtual EDelaunatorValueType GetValueType() const
    {
        return EDelaunatorValueType::DELVT_Int32;
    }

    FORCEINLINE virtual uint8 GetValueUInt8(int32 Index) const override
    {
        return static_cast<uint8>(Values[Index]);
    }

    FORCEINLINE virtual int32 GetValueInt32(int32 Index) const override
    {
        return Values[Index];
    }

    FORCEINLINE virtual void SetValueUInt8(int32 Index, uint8 InValue) override
    {
        Values[Index] = static_cast<int32>(InValue);
    }

    FORCEINLINE virtual void SetValueInt32(int32 Index, int32 InValue) override
    {
        Values[Index] = InValue;
    }
};

UCLASS()
class DELAUNATORPLUGIN_API UDelaunatorFloatValueObject : public UDelaunatorValueObject, public TDelaunatorValueData<TArray<float>, float>
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Set Uniform Value"))
    void K2_SetUniformValue(float InValue);

    UFUNCTION(BlueprintCallable, Category="Delaunator", meta=(DisplayName="Set Values By Indices"))
    void K2_SetValuesByIndices(const TArray<int32>& InIndices, float InValue);

    FORCEINLINE virtual void InitializeValues(int32 ValueCount) override
    {
        SetValues(ValueCount);
    }

    FORCEINLINE virtual int32 GetElementCount() const override
    {
        return Values.Num();
    }

    FORCEINLINE virtual EDelaunatorValueType GetValueType() const
    {
        return EDelaunatorValueType::DELVT_Float;
    }

    FORCEINLINE virtual float GetValueFloat(int32 Index) const override
    {
        return Values[Index];
    }

    FORCEINLINE virtual void SetValueFloat(int32 Index, float InValue) override
    {
        Values[Index] = InValue;
    }
};

// Blueprint Bit Array Value

FORCEINLINE_DEBUGGABLE void UDelaunatorBitFlagsValueObject::K2_SetUniformValue(bool InValue)
{
    SetUniformValue(InValue);
}

FORCEINLINE_DEBUGGABLE void UDelaunatorBitFlagsValueObject::K2_SetValuesByIndices(const TArray<int32>& InIndices, bool InValue)
{
    SetValues(InIndices, InValue);
}

// Blueprint Int Value

FORCEINLINE_DEBUGGABLE void UDelaunatorIntValueObject::K2_SetUniformValue(int32 InValue)
{
    SetUniformValue(InValue);
}

FORCEINLINE_DEBUGGABLE void UDelaunatorIntValueObject::K2_SetValuesByIndices(const TArray<int32>& InIndices, int32 InValue)
{
    SetValues(InIndices, InValue);
}

// Blueprint Float Value

FORCEINLINE_DEBUGGABLE void UDelaunatorFloatValueObject::K2_SetUniformValue(float InValue)
{
    SetUniformValue(InValue);
}

FORCEINLINE_DEBUGGABLE void UDelaunatorFloatValueObject::K2_SetValuesByIndices(const TArray<int32>& InIndices, float InValue)
{
    SetValues(InIndices, InValue);
}
