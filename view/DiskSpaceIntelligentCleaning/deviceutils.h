#ifndef DEVICEUTILS_H
#define DEVICEUTILS_H

#include <QString>

namespace DeviceUtils {
/**
 * @brief 格式化人类友好的数据
 * @param size
 * @return
 */
QString nameOfSize(const quint64 &size);
}

#endif   // DEVICEUTILS_H
