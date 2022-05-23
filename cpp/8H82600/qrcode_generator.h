#ifndef QRCODEGENERATOR_H
#define QRCODEGENERATOR_H

#include <string>
#include <memory>
#include <QPixmap>
#include "qrencode.h"

class QRCodeGenerator
{
public:
    QRCodeGenerator();  

    QRCodeGenerator(const QRCodeGenerator&) = delete;
    QRCodeGenerator& operator=(const QRCodeGenerator&) = delete;

    // Encode QR.
    std::shared_ptr<QRcode> Encode(const std::string& message);

    // Convert to QPixmap for QR.
    // @param <code>: QRcode object.
    // @param <footnote>: note text on the foot of QR image.
    QPixmap ToQPixmap(std::shared_ptr<QRcode> code, const QString& footnote);

private:
    const std::size_t MAX_SIZE = 255;
    const int QR_IMAGE_SIZE = 300;
    const int COLOR_WHITE = 0xFFFFFF;
    const int COLOR_BLACK = 0x0;
};

#endif // QRCODEGENERATOR_H
