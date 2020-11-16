#pragma once

#include <cstdio>
#include <string>
#include <vector>

#include <volk/volk_alloc.hh>

namespace gr {

class data_logger {
public:
  data_logger(const std::string &output_dir, const std::string &prefix, const std::string &suffix, bool enabled = false) {
    _output_dir = output_dir;
    _prefix = prefix;
    _suffix = suffix;
    _enabled = enabled;
  }
  ~data_logger(){};

  template <typename T>
  void vector_to_bin(const std::string &filename, const std::vector<T>& vec_to_print) {
    // return;
    FILE *fp = NULL;

    if (!_enabled) return;

    fp = fopen((_output_dir + "/" + _prefix + "_" + filename + _suffix).c_str(), "w");

    if (fp != NULL) {
      fwrite(&vec_to_print[0], sizeof(T), vec_to_print.size(), fp);
    } else {
      throw std::invalid_argument("Unable to open file for writing");
    }

    fclose(fp);
  }

  template <typename T>
  void vector_to_bin(const std::string &filename, const volk::vector<T>& vec_to_print) {
    // return;
    FILE *fp = NULL;

    if (!_enabled) return;

    fp = fopen((_output_dir + "/" + _prefix + "_" + filename + _suffix).c_str(), "w");

    if (fp != NULL) {
      fwrite(&vec_to_print[0], sizeof(T), vec_to_print.size(), fp);
    } else {
      throw std::invalid_argument("Unable to open file for writing");
    }

    fclose(fp);
  }

  template <typename T>
  void buffer_to_bin(const std::string &filename, T* vec, int length) {
    if (!_enabled) return;

    FILE *fp = NULL;

    fp = fopen((_output_dir + "/" + _prefix + "_" + filename + _suffix).c_str(), "w");

    if (fp != NULL) {
      fwrite(vec, sizeof(T), length, fp);
    } else {
      throw std::invalid_argument("Unable to open file for writing");
    }

    fclose(fp);
  }

  template <typename T>
  void vector_to_bin_append(const std::string &filename, std::vector<T>& vec_to_print) {
    if (!_enabled) return;
    
    FILE *fp = NULL;

    fp = fopen((_output_dir + "/" + _prefix + "_" + filename + _suffix).c_str(), "a");

    if (fp != NULL) {
      fwrite(&vec_to_print[0], sizeof(T), vec_to_print.size(), fp);
    } else {
      throw std::invalid_argument("Unable to open file for writing");
    }

    fclose(fp);
  }

private:
  std::string _output_dir, _prefix, _suffix;
  bool _enabled;
};

} // namespace gr
