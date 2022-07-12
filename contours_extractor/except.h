#pragma once
#include <stdexcept>

namespace xr
{
  struct OutOfRangeException : public std::exception {
    const char* what() const override {
      return "incorrect pixel in image!";
    }
  };

  struct InvalidParameterException : public std::exception {
    InvalidParameterException(const std::string& param_name) : 
      param_name_("invalid parameter: " + param_name)
    {

    }

    const char* what() {
      return param_name_.c_str();
    }

  private:
    std::string param_name_;
  };
}