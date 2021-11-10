#include "classifier.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QFile>

#include "zip/qzipreader.h"

bool Classifier::initFromResource(const QString& filename) {
  // читаем архив
  QJsonParseError parse_error;
  QZipReader zip_reader(filename);
  auto raw_meta_data = zip_reader.fileData("metadata.json");
  auto meta_data = QJsonDocument::fromJson(raw_meta_data, &parse_error).object();
  auto cnn_weights_data = zip_reader.fileData("scripted_model.pth");
  zip_reader.close();

  if (parse_error.error != QJsonParseError::NoError) {
    last_error_ = "ошибка парсинга метаданных: " + parse_error.errorString();
    return false;
  }

  // парсим метаданные
  if (meta_data.contains("classes_header")) {
    header_ = meta_data["classes_header"].toString().split(',');
  }
  else {
    last_error_ = "ошибка парсинга метаданных: отсутствует поле `classes_header`";
    return false;
  }

  if (meta_data.contains("input_size")) {
    auto input_size = meta_data["input_size"].toObject();
    input_size_.height = input_size["height"].toInt();
    input_size_.width = input_size["width"].toInt();
  }
  else {
    last_error_ = "ошибка парсинга метаданных: отсутствует поле `input_size`";
    return false;
  }

  // обученная модель НС
  QFile file("scripted.pth");
  file.remove();

  file.open(QIODevice::WriteOnly);
  file.write(cnn_weights_data);
  file.close();
  file.setPermissions(QFileDevice::WriteUser | QFileDevice::ReadUser | QFileDevice::ExeUser);

  try {
    module_ = torch::jit::load(file.fileName().toStdString());
  }
  catch (c10::Error& e) {
    // qWarning() << c10::GetExceptionString(e).c_str();
    last_error_ = e.what_without_backtrace();
    return false;
  }
  catch (std::exception& e) {
    last_error_ = QString::fromStdString(e.what());
    return false;
  }

  file.remove();

  return true;
}

QString Classifier::header(int idx) const {
  if (idx < header_.size()) {
    return header_[idx];
  }

  return "unknown";
}

cv::Size Classifier::getFrameSize() const {
  return input_size_;
}

QVector<Classifier::Item> Classifier::getGrade(const Classifier::InputData& img) {
  // формируем входные данные модели
  torch::Tensor tensor_img = torch::from_blob(img.data, { img.rows, img.cols, 3 }, torch::kByte);

  // запускаем
  torch::NoGradGuard no_grad;

  auto output = module_.forward({ tensor_img });
  auto tensor = output.toTensor();
  auto softmax = tensor.softmax(0);

  auto count = softmax.size(0);
  float* data = static_cast<float*>(softmax.data_ptr());

  QVector<Item> answer(count);
  for (auto i = 0; i < count; ++i) {
    answer[i] = { header(i), data[i] };
  }

  return answer;
}
