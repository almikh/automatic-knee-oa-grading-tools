#pragma once
#include <QString>
#include <QStringList>
#pragma warning (disable: 4100)
#pragma warning (disable: 4101)
#pragma warning (disable: 4522)
#pragma warning (disable: 4267)

#undef slots
#include <torch/script.h>
# define slots Q_SLOTS

class Classifier {
protected:
    QString last_error_;
    QStringList header_;
    // cv::Size input_size_;
    int use_axis_num_ = 0;
    int use_central_distances_ = 0;
    int use_axis_weights_ = 0;
    torch::jit::script::Module module_;

public:
    bool initFromResource(const QString& filename);
};

#pragma warning (default: 4267)
#pragma warning (default: 4522)
#pragma warning (default: 4101)
#pragma warning (default: 4100)
