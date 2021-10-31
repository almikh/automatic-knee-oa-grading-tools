#include "classifier.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QFile>

#include <fstream>
#include <zip/qzipreader.h>

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
        // input_size_.height = input_size["height"].toInt();
        // input_size_.width = input_size["width"].toInt();
    }
    else {
        last_error_ = "ошибка парсинга метаданных: отсутствует поле `input_size`";
        return false;
    }

    if (meta_data.contains("use_axis_num")) {
        use_axis_num_ = meta_data["use_axis_num"].toInt();
    }
    else use_axis_num_ = 0;

    if (meta_data.contains("use_central_distances")) {
        use_central_distances_ = meta_data["use_central_distances"].toInt();
    }
    else use_central_distances_ = 0;

    if (meta_data.contains("use_axis_weights")) {
        use_axis_weights_ = meta_data["use_axis_weights"].toInt();
    }
    else use_axis_weights_ = 0;

    qDebug() << header_;

    // обученная модель НС
    QFile file(QString(filename).replace(".dll", "_script.pth"));
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
