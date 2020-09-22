#pragma once

#include <pmt/pmt_generated.h>
#include <complex>
#include <map>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <vector>

namespace pmtf {

class pmt_functions
{
private:
    static std::map<DataType, std::type_index> pmt_type_index_map;
    static std::map<DataType, size_t> pmt_type_size_map;
    static std::map<std::type_index, DataType> pmt_index_type_map;

public:
    static size_t pmt_size_info(DataType p);
    static DataType get_pmt_type_from_typeinfo(std::type_index t);
};


class pmt_base
{
public:
    pmt_base(const DataType data_type, Container container_type = Container_Scalar)
        : _data_type(data_type),
          _container_type(container_type){

          };
    DataType data_type() const { return _data_type; };
    Container container_type() const { return _container_type; };

    bool serialize(std::streambuf& sink);
    bool deserialize(std::streambuf& source);

    void build()
    {
        PmtBuilder pb(_fbb);
        pb.add_container_type(_container_type);
        pb.add_data_type(_data_type);
        pb.add_container(_container);
        _blob = pb.Finish();
        _fbb.Finish(_blob);
    }

protected:
    DataType _data_type;
    Container _container_type;
    flatbuffers::FlatBufferBuilder _fbb;
    flatbuffers::Offset<void> _container;
    flatbuffers::Offset<Pmt> _blob;
    // PmtBuilder _builder;
};

typedef std::shared_ptr<pmt_base> pmt_sptr;


template <class T>
class pmt_scalar : public pmt_base
{
public:
    typedef std::shared_ptr<pmt_scalar> sptr;
    static sptr make(const T value)
    {
        return std::make_shared<pmt_scalar<T>>(pmt_scalar<T>(value));
    }

    pmt_scalar(const T& val)
        : pmt_base(pmt_functions::get_pmt_type_from_typeinfo(std::type_index(typeid(T))),
                   Container_Scalar)
    {
        set_value(val);
    }

    void set_value(T val);
    T value();

    void operator=(const T& other) // copy assignment
    {
        set_value(other);
    }

    bool operator==(const T& other) { return other == value(); }
    bool operator!=(const T& other) { return other != value(); }
};

template <class T>
class pmt_vector : public pmt_base
{
public:
    typedef std::shared_ptr<pmt_vector> sptr;
    static sptr make(const T value)
    {
        return std::make_shared<pmt_vector<T>>(pmt_vector<T>(value));
    }

    pmt_vector(const std::vector<T>& val)
        : pmt_base(pmt_functions::get_pmt_type_from_typeinfo(std::type_index(typeid(T))),
                   Container_Vector)
    {
        set_value(val);
    }

    void set_value(const std::vector<T>& val);
    std::vector<T>& value();
    const T* data();
    size_t size()
    {
        auto pmt = GetPmt(_fbb.GetBufferPointer());
        auto fb_vec = pmt->container_as_Vector()->int32_val();
        return fb_vec->size();
    }

    void operator=(const std::vector<T>& other) // copy assignment
    {
        set_value(other);
    }

    bool operator==(const std::vector<T>& other) { return other == value(); }
    bool operator!=(const std::vector<T>& other) { return other != value(); }

    private:
    std::vector<T> _value;
};

} // namespace pmtf