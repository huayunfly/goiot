#include "qrcode_generator.h"
#include <QImage>
#include <QFont>
#include <QPainter>

QRCodeGenerator::QRCodeGenerator()
{

}

std::shared_ptr<QRcode> QRCodeGenerator::Encode(const std::string& message)
{
    if (message.empty() || message.size() > MAX_SIZE)
    {
        throw std::invalid_argument("Empty string or string is too long.");
    }
    int version = 0;
    QRecLevel level = QRecLevel::QR_ECLEVEL_L;
    QRencodeMode mode = QRencodeMode::QR_MODE_8;
    int casesensitive = 1;
    std::shared_ptr<QRcode> code(
                QRcode_encodeString(message.c_str(), version, level, mode, casesensitive),
                                 [] (QRcode* p) { QRcode_free(p); });
    if (code == nullptr)
    {
        throw std::runtime_error("QR encode failed.");
    }
    return code;
}

QPixmap QRCodeGenerator::ToQPixmap(std::shared_ptr<QRcode> code, const QString& footnote)
{
    QImage image = QImage(code->width + 8/*w margin*/,
                          code->width + 8/*h margin*/, QImage::Format_RGB32);
    // pre-fill
    image.fill(COLOR_WHITE); // white
    // fill
    unsigned char* data = code->data;
    for (int y = 0; y < code->width; y++)
    {
        for (int x = 0; x < code->width; x++)
        {
            image.setPixel(x + 4, y + 4, ((*data & 0x1) ? COLOR_BLACK : COLOR_WHITE));
            data++;
        }
    }
    QImage address_image =
            QImage(QR_IMAGE_SIZE, QR_IMAGE_SIZE + 20/*foot*/, QImage::Format_RGB32);
    address_image.fill(COLOR_WHITE);
    QPainter painter(&address_image);
    painter.drawImage(0, 0, image.scaled(QR_IMAGE_SIZE, QR_IMAGE_SIZE));
    QFont font("宋体", 10, QFont::Bold, true);
    font.setPixelSize(12);
    painter.setFont(font);
    QRect padded_rect = address_image.rect();
    padded_rect.setHeight(QR_IMAGE_SIZE + 12); // Draw text on foot.
    painter.drawText(padded_rect, Qt::AlignBottom | Qt::AlignCenter, footnote);
    painter.end();
    return QPixmap::fromImage(address_image);
}


