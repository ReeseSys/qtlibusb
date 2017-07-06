

#ifndef QLIBUSBGLOBAL_H
#define QLIBUSBGLOBAL_H

#include <QtCore/qstring.h>
#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#if defined(QUSB_LIBRARY)
#  define QUSB_SHARED_EXPORT Q_DECL_EXPORT
#else
#  define QUSB_SHARED_EXPORT Q_DECL_IMPORT
#endif

#if defined(QT_BUILD_LIBUSB_LIB)
#  define QT_LIBUSB_EXPORT Q_DECL_EXPORT
#else
#  define QT_LIBUSB_EXPORT Q_DECL_IMPORT
#endif

#ifndef Q_DECL_OVERRIDE
#define Q_DECL_OVERRIDE
#endif

#ifndef QStringLiteral
#define QStringLiteral(str) QString::fromUtf8(str)
#endif

#ifndef Q_NULLPTR
#define Q_NULLPTR NULL
#endif

QT_END_NAMESPACE

#endif // QLIBUSBGLOBAL_H