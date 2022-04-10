#include "app_preferences.h"
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <memory>

std::unique_ptr<QSettings> instance;

void openIfNotOpened() {
  if (!instance) {
    instance.reset(new QSettings("Grading Tools", "Grading Tool"));
  }
}

void AppPrefs::clear() {
  openIfNotOpened();
  return instance->clear();
}

bool AppPrefs::contains(const QString& name) {
  openIfNotOpened();
  return instance->contains(name);
}

void AppPrefs::write(const QString& name, const QVariant& value) {
  openIfNotOpened();
  instance->setValue(name, value);
  instance->sync();
}

QVariant AppPrefs::read(const QString& name, const QVariant& default_value) {
  openIfNotOpened();
  return instance->value(name, default_value);
}

void AppPrefs::remove(const QString& name) {
  openIfNotOpened();
  return instance->remove(name);
}

QStringList AppPrefs::getAllKeys() {
  return instance->allKeys();
}

QStringList AppPrefs::getAllKeys(const QString& group_name) {
  instance->beginGroup(group_name);
  auto keys = instance->childKeys();
  instance->endGroup();

  return keys;
}
