#pragma once
#include <QString>
#include <QStringList>
#pragma warning (disable: 4100)
#pragma warning (disable: 4101)
#pragma warning (disable: 4522)
#pragma warning (disable: 4267)

#include <opencv2/opencv.hpp>

#undef slots
#include <torch/script.h>
# define slots Q_SLOTS

class Classifier {
public:
  using InputData = cv::Mat;
  struct Item {
    QString mnemonic_code;
    float confidence = 0.0f;
  };

protected:
  QString last_error_;
  QStringList header_;
  cv::Size input_size_;
  torch::jit::script::Module module_;

public:
  bool initFromResource(const QString& filename);

  QString header(int idx) const;
  cv::Size getFrameSize() const;
  QVector<Item> getGrade(const InputData& input_data);
};

#pragma warning (default: 4267)
#pragma warning (default: 4522)
#pragma warning (default: 4101)
#pragma warning (default: 4100)
