// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "inc/Core/VectorSet.h"
#include <fstream>

using namespace SPTAG;

#pragma warning(disable:4996)  // 'fopen': This function or variable may be unsafe. Consider using fopen_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.

VectorSet::VectorSet()
{
}


VectorSet::~VectorSet()
{
}


BasicVectorSet::BasicVectorSet(const ByteArray& p_bytesArray,
                               VectorValueType p_valueType,
                               DimensionType p_dimension,
                               SizeType p_vectorCount)
    : m_data(p_bytesArray),
      m_valueType(p_valueType),
      m_dimension(p_dimension),
      m_vectorCount(p_vectorCount),
      m_perVectorDataSize(static_cast<SizeType>(p_dimension * GetValueTypeSize(p_valueType)))
{
}

// copied from src/IndexBuilder/main.cpp
BasicVectorSet::BasicVectorSet(const char* p_filePath, VectorValueType p_valueType)
    : m_valueType(p_valueType) 
{
    std::ifstream inputStream(p_filePath, std::ifstream::binary);
    if (!inputStream.is_open()) {
        fprintf(stderr, "Failed to read input file.\n");
        exit(1);
    }
    SizeType row;
    DimensionType col;
    inputStream.read((char*)&row, sizeof(SizeType));
    inputStream.read((char*)&col, sizeof(DimensionType));
    std::uint64_t totalRecordVectorBytes = ((std::uint64_t)GetValueTypeSize(p_valueType)) * row * col;

    m_data = ByteArray::Alloc(totalRecordVectorBytes);
    char* vecBuf = reinterpret_cast<char*>(m_data.Data());
    inputStream.read(vecBuf, totalRecordVectorBytes);
    inputStream.close();

    m_valueType = p_valueType;
    m_dimension = col;
    m_vectorCount = row;
    m_perVectorDataSize = static_cast<SizeType>(col * GetValueTypeSize(p_valueType));
}

BasicVectorSet::~BasicVectorSet()
{
}


VectorValueType
BasicVectorSet::GetValueType() const
{
    return m_valueType;
}


void*
BasicVectorSet::GetVector(SizeType p_vectorID) const
{
    if (p_vectorID < 0 || p_vectorID >= m_vectorCount)
    {
        return nullptr;
    }

    return reinterpret_cast<void*>(m_data.Data() + ((size_t)p_vectorID) * m_perVectorDataSize);
}


void*
BasicVectorSet::GetData() const
{
    return reinterpret_cast<void*>(m_data.Data());
}

DimensionType
BasicVectorSet::Dimension() const
{
    return m_dimension;
}


SizeType
BasicVectorSet::Count() const
{
    return m_vectorCount;
}


bool
BasicVectorSet::Available() const
{
    return m_data.Data() != nullptr;
}


ErrorCode 
BasicVectorSet::Save(const std::string& p_vectorFile) const
{
    FILE * fp = fopen(p_vectorFile.c_str(), "wb");
    if (fp == NULL) return ErrorCode::FailedOpenFile;

    fwrite(&m_vectorCount, sizeof(SizeType), 1, fp);
    fwrite(&m_dimension, sizeof(DimensionType), 1, fp);

    fwrite((const void*)(m_data.Data()), m_data.Length(), 1, fp);
    fclose(fp);
    return ErrorCode::Success;
}

SizeType BasicVectorSet::PerVectorDataSize() const {
    return m_perVectorDataSize;
}