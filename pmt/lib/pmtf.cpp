#include <pmt/pmtf.hpp>
#include <pmt/pmtf_scalar.hpp>
#include <pmt/pmtf_string.hpp>
#include <pmt/pmtf_vector.hpp>
#include <map>

namespace flatbuffers {
pmtf::Complex64 Pack(const std::complex<float>& obj)
{
    return pmtf::Complex64(obj.real(), obj.imag());
}

const std::complex<float> UnPack(const pmtf::Complex64& obj)
{
    return std::complex<float>(obj.re(), obj.im());
}
} // namespace flatbuffers

namespace pmtf {


///////////////////////////////////////////////////////////////////////////////////
// Static PMT Base functions
///////////////////////////////////////////////////////////////////////////////////

pmt_base::sptr pmt_base::from_pmt(const pmtf::Pmt* fb_pmt)
{
    switch (fb_pmt->data_type()) {
    case Data::PmtString:
        return std::static_pointer_cast<pmt_base>(pmt_string::from_pmt(fb_pmt));
    case Data::ScalarComplex64:
        return std::static_pointer_cast<pmt_base>(
            pmt_scalar<std::complex<float>>::from_pmt(fb_pmt));
    case Data::ScalarInt32:
        return std::static_pointer_cast<pmt_base>(pmt_scalar<int32_t>::from_pmt(fb_pmt));
    case Data::VectorInt32:
        return std::static_pointer_cast<pmt_base>(pmt_vector<int32_t>::from_pmt(fb_pmt));
        // case Data::VectorComplex64:
        // return
        // std::static_pointer_cast<pmt_base>(from_pmt<std::complex<float>>(buf));

    default:
        throw std::runtime_error("Unsupported PMT Type");
    }
}

pmt_base::sptr pmt_base::from_buffer(const uint8_t* buf, size_t size)
{
    auto PMT = GetPmt(buf);
    return from_pmt(PMT);
}

template class pmt_scalar<std::complex<float>>;
template class pmt_vector<std::int32_t>;
// template class pmt_vector<std::complex<float>>;

} // namespace pmtf
