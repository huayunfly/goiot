#ifndef FORM_GASFEED_H
#define FORM_GASFEED_H

#include <QWidget>

namespace Ui {
class FormGasFeed;
}

class FormGasFeed : public QWidget
{
    Q_OBJECT

public:
    explicit FormGasFeed(QWidget *parent = nullptr);
    ~FormGasFeed();

    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

private:
    Ui::FormGasFeed *ui;
};

#endif // FORM_GASFEED_H
