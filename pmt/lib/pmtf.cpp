#include <pmt/pmtf.hpp>

namespace pmtf {

std::map<DataType, std::type_index> pmt_functions::pmt_type_index_map = {
    { DataType_FLOAT, typeid(float) },
    { DataType_DOUBLE, typeid(double) },
    { DataType_CFLOAT, typeid(std::complex<float>) },
    { DataType_CDOUBLE, typeid(std::complex<double>) },
    { DataType_INT8, typeid(int8_t) },
    { DataType_INT16, typeid(int16_t) },
    { DataType_INT32, typeid(int32_t) },
    { DataType_INT64, typeid(int64_t) },
    { DataType_UINT8, typeid(uint8_t) },
    { DataType_UINT16, typeid(uint16_t) },
    { DataType_UINT32, typeid(uint32_t) },
    { DataType_UINT64, typeid(uint64_t) },
    { DataType_BOOL, typeid(bool) },
    { DataType_ENUM, typeid(int) }, //??
    { DataType_STRING, typeid(std::string) },
    { DataType_VOID, typeid(void) }
};

std::map<std::type_index, DataType> pmt_functions::pmt_index_type_map = {
    { std::type_index(typeid(float)), DataType_FLOAT },
    { std::type_index(typeid(double)), DataType_DOUBLE },
    { std::type_index(typeid(std::complex<float>)), DataType_CFLOAT },
    { std::type_index(typeid(std::complex<double>)), DataType_CDOUBLE },
    { std::type_index(typeid(int8_t)), DataType_INT8 },
    { std::type_index(typeid(int16_t)), DataType_INT16 },
    { std::type_index(typeid(int32_t)), DataType_INT32 },
    { std::type_index(typeid(int64_t)), DataType_INT64 },
    { std::type_index(typeid(uint8_t)), DataType_UINT8 },
    { std::type_index(typeid(uint16_t)), DataType_UINT16 },
    { std::type_index(typeid(uint32_t)), DataType_UINT32 },
    { std::type_index(typeid(uint64_t)), DataType_UINT64 },
    { std::type_index(typeid(bool)), DataType_BOOL },
    { std::type_index(typeid(int)), DataType_ENUM }, //??
    { std::type_index(typeid(std::string)), DataType_STRING },
    { std::type_index(typeid(void)), DataType_VOID }
};

std::map<DataType, size_t> pmt_functions::pmt_type_size_map = {
    { DataType_FLOAT, sizeof(float) },
    { DataType_DOUBLE, sizeof(double) },
    { DataType_CFLOAT, sizeof(std::complex<float>) },
    { DataType_CDOUBLE, sizeof(std::complex<double>) },
    { DataType_INT8, sizeof(int8_t) },
    { DataType_INT16, sizeof(int16_t) },
    { DataType_INT32, sizeof(int32_t) },
    { DataType_INT64, sizeof(int64_t) },
    { DataType_UINT8, sizeof(uint8_t) },
    { DataType_UINT16, sizeof(uint16_t) },
    { DataType_UINT32, sizeof(uint32_t) },
    { DataType_UINT64, sizeof(uint64_t) },
    { DataType_BOOL, sizeof(bool) },
    { DataType_ENUM, sizeof(int) }, //??
    { DataType_STRING, sizeof(std::string) },
    { DataType_VOID, sizeof(void*) }
};

size_t pmt_functions::pmt_size_info(DataType p) { return pmt_type_size_map[p]; }
DataType pmt_functions::get_pmt_type_from_typeinfo(std::type_index t)
{
    return pmt_index_type_map[t];
}

///////////////////////////
// Scalar
///////////////////////////

template <>
void pmt_scalar<int32_t>::set_value(int32_t val)
{
    ScalarInt32Builder sb(_fbb);
    sb.add_value(val);
    _data = sb.Finish().Union();
    build();
}

template <>
pmt_scalar<int32_t>::pmt_scalar(const int32_t& val) : pmt_base(Data_ScalarInt32)
{
    set_value(val);
}

template <>
int32_t pmt_scalar<int32_t>::value()
{
    auto pmt = GetPmt(_fbb.GetBufferPointer());
    return pmt->data_as_ScalarInt32()->value();
}


///////////////////////////
// Vector
///////////////////////////


template <>
void pmt_vector<int32_t>::set_value(const std::vector<int32_t>& val)
{
    auto vec = _fbb.CreateVector(val.data(), val.size());
    VectorInt32Builder vb(_fbb);
    vb.add_value(vec);
    _data = vb.Finish().Union();
    build();
}

template <>
pmt_vector<int32_t>::pmt_vector(const std::vector<int32_t>& val)
    : pmt_base(Data_VectorInt32)
{
    set_value(val);
}

template <>
std::vector<int32_t>& pmt_vector<int32_t>::value()
{
    auto pmt = GetPmt(_fbb.GetBufferPointer());
    auto fb_vec = pmt->data_as_VectorInt32()->value();
    _value.assign(fb_vec->begin(), fb_vec->end());
    return _value;
}

template <>
const int32_t* pmt_vector<int32_t>::data()
{
    auto pmt = GetPmt(_fbb.GetBufferPointer());
    auto fb_vec = pmt->data_as_VectorInt32()->value();
    return fb_vec->data();
}

template <>
size_t pmt_vector<int32_t>::size()
{
    auto pmt = GetPmt(_fbb.GetBufferPointer());
    auto fb_vec = pmt->data_as_VectorInt32()->value();
    return fb_vec->size();
}

} // namespace pmtf