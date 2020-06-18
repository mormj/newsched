#include <gnuradio/flat_graph.hpp>
namespace gr {
class scheduler
{

public:
    scheduler(flat_graph_sptr fg) {};
    virtual ~scheduler();
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void wait() = 0;
};
} // namespace gr