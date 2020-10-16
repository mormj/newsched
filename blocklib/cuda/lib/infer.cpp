#include <gnuradio/blocklib/cuda/infer.hpp>

namespace gr {
namespace cuda {


infer::infer(const std::string& onnx_pathname,
             size_t itemsize,
             memory_model_t memory_model,
             uint64_t workspace_size,
             int dla_core)
    : block("infer"),
      _onnx_pathname(onnx_pathname),
      _engine(nullptr),
      _memory_model(memory_model),
      _workspace_size(workspace_size),
      _dla_core(dla_core)

{

    build();

    set_output_multiple(d_output_vlen);
}


//!
//! \brief Uses a ONNX parser to create the Onnx MNIST Network and marks the
//!        output layers
//!
//! \param network Pointer to the network that will be populated with the Onnx MNIST
//! network
//!
//! \param builder Pointer to the engine builder
//!
bool infer::constructNetwork(SampleUniquePtr<nvinfer1::IBuilder>& builder,
                             SampleUniquePtr<nvinfer1::INetworkDefinition>& network,
                             SampleUniquePtr<nvinfer1::IBuilderConfig>& config,
                             SampleUniquePtr<nvonnxparser::IParser>& parser)
{
    auto parsed =
        parser->parseFromFile(_onnx_pathname.c_str(),
                              static_cast<int>(sample::gLogger.getReportableSeverity()));
    if (!parsed) {
        return false;
    }

    // config->setMaxWorkspaceSize(2_GiB);
    config->setMaxWorkspaceSize(_workspace_size);
    if (d_fp16) {
        config->setFlag(BuilderFlag::kFP16);
    }
    if (d_int8) {
        config->setFlag(BuilderFlag::kINT8);
        samplesCommon::setAllTensorScales(network.get(), 127.0f, 127.0f);
    }

    samplesCommon::enableDLA(builder.get(), config.get(), d_dla_core);

    return true;
}

//!
//! \brief Creates the network, configures the builder and creates the network engine
//!
//! \details This function creates the Onnx MNIST network by parsing the Onnx model and
//! builds
//!          the engine that will be used to run MNIST (_engine)
//!
//! \return Returns true if the engine was created successfully and false otherwise
//!
bool infer::build()
{
    auto builder = SampleUniquePtr<nvinfer1::IBuilder>(
        nvinfer1::createInferBuilder(sample::gLogger.getTRTLogger()));
    if (!builder) {
        return false;
    }

    const auto explicitBatch =
        1U << static_cast<uint32_t>(NetworkDefinitionCreationFlag::kEXPLICIT_BATCH);
    auto network = SampleUniquePtr<nvinfer1::INetworkDefinition>(
        builder->createNetworkV2(explicitBatch));
    if (!network) {
        return false;
    }


    auto config =
        SampleUniquePtr<nvinfer1::IBuilderConfig>(builder->createBuilderConfig());
    if (!config) {
        return false;
    }

    auto parser = SampleUniquePtr<nvonnxparser::IParser>(
        nvonnxparser::createParser(*network, sample::gLogger.getTRTLogger()));
    if (!parser) {
        return false;
    }

    auto constructed = constructNetwork(builder, network, config, parser);
    if (!constructed) {
        return false;
    }

    _engine = std::shared_ptr<nvinfer1::ICudaEngine>(
        builder->buildEngineWithConfig(*network, *config), samplesCommon::InferDeleter());
    if (!_engine) {
        return false;
    }

    std::cout << "Max Batch Size: " << _engine->getMaxBatchSize() << std::endl;
    assert(network->getNbInputs() == 1);
    d_input_dims = network->getInput(0)->getDimensions();
    // assert(d_input_dims.nbDims == 4);

    assert(network->getNbOutputs() == 1);
    d_output_dims = network->getOutput(0)->getDimensions();
    // assert(d_output_dims.nbDims == 2);

    d_inputH = d_input_dims.d[0];
    d_inputW = d_input_dims.d[1];
    d_outputH = d_output_dims.d[0];
    d_outputW = d_output_dims.d[1];

    d_input_vlen = d_inputH * d_inputW;
    d_output_vlen = d_outputH * d_outputW;

    d_context =
        SampleUniquePtr<nvinfer1::IExecutionContext>(_engine->createExecutionContext());
    if (!d_context) {
        return false;
    }

    auto nb = _engine->getNbBindings();
    for (auto i = 0; i < nb; i++) {
        auto type = _engine->getBindingDataType(i);
        auto dims = d_context->getBindingDimensions(i);
        int vecDim = _engine->getBindingVectorizedDim(i);
        // size_t vol = d_context || !d_batch_size ? 1 :
        // static_cast<size_t>(d_batch_size);
        size_t vol = 1; // ONNX Parser only supports explicit batch which means it is
                        // baked into the model
        // size_t vol = d_batch_size;
        if (-1 != vecDim) // i.e., 0 != lgScalarsPerVector
        {
            int scalarsPerVec = _engine->getBindingComponentsPerElement(i);
            dims.d[vecDim] = samplesCommon::divUp(dims.d[vecDim], scalarsPerVec);
            vol *= scalarsPerVec;
        }
        vol *= samplesCommon::volume(dims);
    }


    return true;
}

work_return_code_t infer::check_work_io(std::vector<block_work_input>& work_input,
                                        std::vector<block_work_output>& work_output)
{
    // Instead of forecast, let's check what the scheduler gave us
    //  and notify appropriately

    // For this block, the number of outputs relative to the number of
    // inputs is related to the parsed model

    // Assuming that ninput_items > noutput_items
    int nb =
        work_output[0].n_items / d_output_vlen; // number of input vectors (batch size)
    int n_input_required = nb * d_input_vlen;

    if (n_input_required > work_input[0].n_items) {
        return work_return_code_t::WORK_INSUFFICIENT_INPUT_ITEMS;
    }

    return work_return_code_t::WORK_OK;
}

work_return_code_t infer::work(std::vector<block_work_input>& work_input,
                               std::vector<block_work_output>& work_output)
{
    auto ret = check_work_io(work_input, work_output);
    if (ret != work_return_code_t::WORK_OK && ret != work_return_code_t::WORK_DONE) {
        return ret;
    }

    const float* in = reinterpret_cast<const float*>(work_input[0].items);
    float* out = reinterpret_cast<float*>(work_output[0].items);



    int in_sz = d_input_vlen;   // * d_batch_size;
    int out_sz = d_output_vlen; // * d_batch_size;

    auto noutput_items = work_output[0].n_items;
    auto num_batches = noutput_items / out_sz;
    auto ni = num_batches * in_sz;

    unsigned int n_consumed = 0;
    unsigned int n_produced = 0;

    std::vector<void *> bindings(2);

    for (auto b = 0; b < num_batches; b++) {

        // in and out are assumed to be valid device bindings from pinned or unified buffers
        // requiring no additional memcpy
        bindings[0] = const_cast<void *>(static_cast<const void *>(in + b*in_sz));
        bindings[1] = static_cast<void *>(out + b*out_sz);

        bool status = d_context->executeV2(bindings.data());
        // bool status = d_context->execute(d_batch_size, d_device_bindings.data());
        if (!status) {
            return work_return_code_t::WORK_ERROR;
        }
        cudaDeviceSynchronize();

        n_consumed += in_sz;
        n_produced += out_sz;
    }


    work_input[0].n_consumed = n_consumed;
    work_output[0].n_produced = n_produced;
    return work_return_code_t::WORK_OK;
}

} // namespace cuda
} // namespace gr