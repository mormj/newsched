#pragma once

#include <algorithm>
#include <vector>

namespace gr {

/**
 * @brief Singleton object to produce unique block ids
 *
 * The logic of how nodeids are generated is very simple, just an incrementing integer so
 * that all blocks across the flowgraph are given a unique ID that can be used for map
 * indices and other references
 *
 */

class nodeid_generator
{
public:
    static nodeid_generator& get_instance()
    {
        static nodeid_generator instance;
        return instance;
    }

    /**
     * @brief Get the next global id
     *
     * Static method to return a fresh global id when a node is instantiated
     *
     * @return uint32_t the global id
     */
    static uint32_t get_id() { return get_instance().get_id_(); }

private:
    std::vector<uint32_t> _used_ids;
    uint32_t _last_id = 0;
    nodeid_generator() {}

    uint32_t get_id_()
    {
        auto next_id = _last_id + 1;

        // verify that it is not in the used_ids;
        while (std::find(_used_ids.begin(), _used_ids.end(), next_id) !=
               _used_ids.end()) {
            next_id++;
        }

        _last_id = next_id;
        _used_ids.push_back(next_id);

        return next_id;
    }

public:
    nodeid_generator(nodeid_generator const&) = delete;
    void operator=(nodeid_generator const&) = delete;
};

} // namespace gr