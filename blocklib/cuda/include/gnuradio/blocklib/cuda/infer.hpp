#pragma once

#include <gnuradio/block.hpp>


#include <NvInfer.h>
// #include <cuda_runtime_api.h>

#include "trt_common.h"
#include "trt_logger.h"
#include "parserOnnxConfig.h"


namespace gr {
namespace cuda {

enum class memory_model_t { TRADITIONAL = 0, PINNED, UNIFIED };

class infer : public block
{
public:
    enum params : uint32_t { num_params };

    typedef std::shared_ptr<infer> sptr;
    static sptr make(const std::string& onnx_pathname,
                     size_t itemsize,
                     memory_model_t memory_model = memory_model_t::TRADITIONAL,
                     uint64_t workspace_size = (1 << 30),
                     int dla_core = -1)
    {
        auto ptr = std::make_shared<infer>(onnx_pathname, itemsize, memory_model, workspace_size, dla_core);

        ptr->add_port(port<float>::make("input",
                                    port_direction_t::INPUT,
                                    port_type_t::STREAM));
        ptr->add_port(port<float>::make("output",
                                    port_direction_t::OUTPUT,
                                    port_type_t::STREAM));

        // ptr->add_param(
        //     param<T>::make(infer<T>::params::id_k, "k", k, &ptr->d_k));

        // // TODO: vlen should be const and unchangeable as a parameter
        // ptr->add_param(param<size_t>::make(
        //     infer<T>::params::id_vlen, "vlen", vlen, &ptr->d_vlen));

        return ptr;

    }

    infer(const std::string& onnx_pathname,
                     size_t itemsize,
                     memory_model_t memory_model,
                     uint64_t workspace_size,
                     int dla_core);
    // ~infer() {};

    work_return_code_t check_work_io(std::vector<block_work_input>& work_input,
                                    std::vector<block_work_output>& work_output);
                                    
    virtual work_return_code_t work(std::vector<block_work_input>& work_input,
                                    std::vector<block_work_output>& work_output);

private:
    std::string _onnx_pathname;
    size_t _itemsize;
    memory_model_t _memory_model;
    uint64_t _workspace_size;
    int _dla_core;

    template <typename T>
    using SampleUniquePtr = std::unique_ptr<T, samplesCommon::InferDeleter>;
    nvinfer1::Dims d_input_dims;  //!< The dimensions of the input to the network.
    nvinfer1::Dims d_output_dims; //!< The dimensions of the output to the network.
    bool d_int8{ false };         //!< Allow runnning the network in Int8 mode.
    bool d_fp16{ false };         //!< Allow running the network in FP16 mode.
    int32_t d_dla_core{ -1 };     //!< Specify the DLA core to run network on.

    std::shared_ptr<nvinfer1::ICudaEngine>
        _engine; //!< The TensorRT engine used to run the network

    SampleUniquePtr<nvinfer1::IExecutionContext> d_context;

    std::vector<void*> d_device_bindings;

    int d_inputH, d_outputH, d_inputW, d_outputW;
    int d_input_vlen, d_output_vlen;

    bool build();
    bool constructNetwork(SampleUniquePtr<nvinfer1::IBuilder>& builder,
                          SampleUniquePtr<nvinfer1::INetworkDefinition>& network,
                          SampleUniquePtr<nvinfer1::IBuilderConfig>& config,
                          SampleUniquePtr<nvonnxparser::IParser>& parser);


    };

} // namespace blocks
} // namespace blocks