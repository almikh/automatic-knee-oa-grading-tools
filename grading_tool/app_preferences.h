#pragma once
#include <QVariant>
#include <QString>

struct AppPrefs {
  static void clear();
  static bool contains(const QString& name);
  static void write(const QString& name, const QVariant& value);
  static QVariant read(const QString& name, const QVariant& default_value = QVariant());
  static void remove(const QString& name);

  static QStringList getAllKeys();
  static QStringList getAllKeys(const QString& group_name);
};
