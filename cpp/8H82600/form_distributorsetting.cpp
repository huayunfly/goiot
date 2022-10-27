#include "form_distributorsetting.h"
#include "ui_form_distributorsetting.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <thread>

FormDistributorSetting::FormDistributorSetting(QWidget *parent) :
    FormCommon(parent, "distsetting", QString::fromUtf8("移液仪")),
    ui(new Ui::FormDistributorSetting), pos_x_(-1), pos_y_(-1)
{
    ui->setupUi(this);
    InitRuntimeView();
    InitControlPanel();
    EnableButtons(false);
}

FormDistributorSetting::~FormDistributorSetting()
{
    delete ui;
}

bool FormDistributorSetting::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    if (event->type() == Ui::RefreshStateEvent::myType)
    {
        Ui::RefreshStateEvent* e = static_cast<Ui::RefreshStateEvent*>(event);
        if (e->Name().compare("label_servo_on", Qt::CaseInsensitive) == 0)
        {
            if (e->State() > 0)
            {
                ui->label_servo_on->setText("SERVO ON");
                ui->label_servo_on->setStyleSheet("QLabel{background-color:rgb(144,238,144);}");
                EnableButtons(true);
            }
            else
            {
                ui->label_servo_on->setText("SERVO OFF");
                ui->label_servo_on->setStyleSheet("QLabel{background-color:rgb(165,42,42);}");
                EnableButtons(false);
            }
        }
        else if (e->Name().compare("label_servo_error", Qt::CaseInsensitive) == 0)
        {
            if (e->State() > 0)
            {
                ui->label_servo_error->setText("ERROR");
                ui->label_servo_error->setStyleSheet("QLabel{background-color:rgb(165,42,42);}");
            }
            else
            {
                ui->label_servo_error->setText("NORMAL");
                ui->label_servo_error->setStyleSheet("QLabel{background-color:rgb(190,190,190);}");
            }
        }
        else if (e->Name().compare("label_speedY", Qt::CaseInsensitive) == 0)
        {
            ui->label_speedY->setText(QString::number(e->State()) + " (当前速率mm/s)");
        }
        else if (e->Name().compare("label_posX", Qt::CaseInsensitive) == 0)
        {
            ui->label_posX->setText(QString::number(e->State()) + " (当前X位置)");
            pos_x_ = e->State();
            UpdateRuntimeView();
        }
        else if (e->Name().compare("label_posY", Qt::CaseInsensitive) == 0)
        {
            ui->label_posY->setText(QString::number(e->State()) + " (当前Y位置)");
            pos_y_ = e->State();
            UpdateRuntimeView();
        }
        else if (e->Name().compare("label_sampling_injector", Qt::CaseInsensitive) == 0)
        {
            if (e->State() > 0)
            {
                ui->label_sampling_injector->setText(QString::number(e->State()) + "(采液头下伸)");
            }
            else
            {
                ui->label_sampling_injector->setText(QString::number(e->State()) + "(采液头复位)");
            }
        }
        else if (e->Name().compare("label_collection_injector", Qt::CaseInsensitive) == 0)
        {
            if (e->State() > 0)
            {
                ui->label_collection_injector->setText(QString::number(e->State()) + "(收集头下伸)");
            }
            else
            {
                ui->label_collection_injector->setText(QString::number(e->State()) + "(收集头复位)");
            }
        }
        return true;
    }

    return QWidget::event(event);
}

void FormDistributorSetting::InitRuntimeView()
{
    InitHomePosItem(35, 35, 10, 4, 1, 2 * (2 * 65), (16 + 1) * 35 + 15);
    InitSamplingItem(35, 35, 15, 4, 32, 8, 2 * (2 * 65)/* x_count * (2 * x_gap) */, 0);
    InitSamplingItem(65, 65, 30, 2, 8, 2, 0/* pos_x */, 0/* pos_y */);

    auto scene = new QGraphicsScene(
                0, 0, /*x_count * (2 * x_gap)*/500,
                (32 + 3) * 35 + 10/*(y_count + 3) * y_gap + y_margin)*/);
    for (auto& item : sampling_ui_items)
    {
        scene->addItem(item.get());
    }

    auto view = new QGraphicsView(scene);
    view->show();
    ui->verticalLayout->addWidget(view, 0, Qt::AlignTop | Qt::AlignLeft);
}

void FormDistributorSetting::InitSamplingItem(
        int x_gap, int y_gap, double radius, int x_count,
        int y_count, int y_section, int pos_x, int pos_y)
{
    std::vector<std::pair<double, double>> positions;
    int number = 1;
    int y_margin = 10;
    double y_base = y_margin;
    for (int y = 0; y < y_count; y++)
    {
        if (y >= y_section && y < 2 * y_section)
        {
            y_base = y_gap + y_margin;
        }
        else if (y >= 2 * y_section && y < 3 * y_section)
        {
            y_base = 2 * y_gap + y_margin;
        }
        else if (y >= 3 * y_section)
        {
            y_base = 3 * y_gap + y_margin;
        }
        for (int x = 0; x < x_count; x++)
        {
            auto item =
                    std::make_shared<SamplingUIItem>(radius, number);
            item->setPos(pos_x + x * x_gap + 2 * x_gap,
                         pos_y + y_base + y * y_gap + radius);
            sampling_ui_items.push_back(item);
            number++;
        }
    }
    // for 2 sink positions
    for (int y = 0; y < 2; y++)
    {
        if (y == 0)
        {
            y_base = y_section * y_gap + y_margin;
        }
        else
        {
            y_base = (3 * y_section + 2) * y_gap + y_margin;
        }
        for (int x = 0; x < x_count; x++)
        {
            auto item = std::make_shared<SamplingUIItem>(
                            radius,
                            129,
                            0,
                            SamplingUIItem::SamplingUIItemStatus::Undischarge);
            item->setPos(pos_x + x * x_gap + 2 * x_gap,
                         pos_y + y_base + radius);
            sampling_ui_items.push_back(item);
            number++;
        }
    }
}

void FormDistributorSetting::InitHomePosItem(
        int x_gap, int y_gap, double radius, int x_count, int y_count,
        int pos_x, int pos_y)
{
    Q_UNUSED(y_gap);
    Q_UNUSED(y_count);
    for (int x = 0; x < x_count; x++)
    {
        auto item = std::make_shared<SamplingUIItem>(
                        radius,
                        0,
                        0,
                        SamplingUIItem::SamplingUIItemStatus::Unsigned);
        item->setPos(pos_x + x * x_gap + 2 * x_gap, pos_y + radius);
        sampling_ui_items.push_back(item);
    }
}

void FormDistributorSetting::InitControlPanel()
{
    // Servo power
    ui->label_servo_on->setText("OFF");
    ui->label_servo_on->setStyleSheet("QLabel{background-color:rgb(165,42,42);}");
    // List
    QStringList speed_y;
    for (int i = 0; i <= 50; i++)
    {
        speed_y.push_back(QString::number(i));
    }
    QStringList pos_x = {"0", "1"};
    QStringList pos_y;
    for (int i = 0; i < 45; i++)
    {
        pos_y.push_back(QString::number(i));
    }
    QStringList pos_injector = {"0", "1"};
    ui->comboBox_speedY->addItems(speed_y);
    ui->comboBox_moveX->addItems(pos_x);
    ui->comboBox_moveY->addItems(pos_y);
    ui->comboBox_sampling_injector->addItems(pos_injector);
    ui->comboBox_collection_injector->addItems(pos_injector);
}

void FormDistributorSetting::UpdateRuntimeView()
{
    if (pos_x_ < 0 || pos_y_ < 0)
    {
        return;
    }
    if (pos_x_ > 1 || pos_y_ > 45)
    {
        return;
    }
    for (auto& item : sampling_ui_items)
    {
        item->SetStatus(SamplingUIItem::SamplingUIItemStatus::Unsigned, 0/*channel*/);
    }
    int LIQ_SAMPLING_SLOTS = 4;
    int LIQ_SAMPLING_GAP = 2;
    int LIQ_COLLECTION_SLOTS = 2;
    int LIQ_COLLECTION_GAP = 1;
    if (pos_y_ <= 34)
    {
        int selected = pos_y_ * LIQ_SAMPLING_SLOTS + pos_x_ * LIQ_SAMPLING_GAP;
        sampling_ui_items.at(selected)->SetStatus(
                    SamplingUIItem::SamplingUIItemStatus::Waiting, 0/*channel*/);
        sampling_ui_items.at(selected + LIQ_SAMPLING_GAP)->SetStatus(
                    SamplingUIItem::SamplingUIItemStatus::Waiting, 0/*channel*/);
    }
    else if (pos_y_ > 34 && pos_y_ < 45)
    {
        int selected = 35 * LIQ_SAMPLING_SLOTS + (pos_y_ - 35) * LIQ_COLLECTION_SLOTS;
        sampling_ui_items.at(selected)->SetStatus(
                    SamplingUIItem::SamplingUIItemStatus::Waiting, 0/*channel*/);
        sampling_ui_items.at(selected + LIQ_COLLECTION_GAP)->SetStatus(
                    SamplingUIItem::SamplingUIItemStatus::Waiting, 0/*channel*/);
    }
}

void FormDistributorSetting::EnableButtons(bool enable)
{
    ui->button_speedY->setEnabled(enable);
    ui->button_moveX->setEnabled(enable);
    ui->button_moveY->setEnabled(enable);
    ui->button_sampling_injector->setEnabled(enable);
    ui->button_collection_injector->setEnabled(enable);
}

void FormDistributorSetting::on_button_servo_on_clicked()
{
    QString button_id = "button_servo_on";
    bool ok = write_data_func_(this->objectName(), button_id, QString::number(1));
    assert(ok);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void FormDistributorSetting::on_button_servo_off_clicked()
{
    QString button_id = "button_servo_off";
    bool ok = write_data_func_(this->objectName(), button_id, QString::number(0));
    assert(ok);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void FormDistributorSetting::on_button_ack_error_clicked()
{
    QString button_id = "button_ack_error";
    bool ok = write_data_func_(this->objectName(), button_id, QString::number(1));
    assert(ok);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void FormDistributorSetting::on_button_speedY_clicked()
{
    QString button_id = "button_speedY";
    int speed_y = ui->comboBox_speedY->currentText().toInt();
    bool ok = write_data_func_(this->objectName(), button_id, QString::number(speed_y));
    assert(ok);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void FormDistributorSetting::on_button_moveX_clicked()
{
    QString button_id = "button_moveX";
    int pos_x = ui->comboBox_moveX->currentText().toInt();
    assert(pos_x == 0 || pos_x == 1);
    bool ok = write_data_func_(this->objectName(), button_id, QString::number(pos_x));
    assert(ok);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void FormDistributorSetting::on_button_moveY_clicked()
{
    QString button_id = "button_moveY";
    int pos_y = ui->comboBox_moveY->currentText().toInt();
    assert(pos_y >= 0 && pos_y <= 45);
    bool ok = write_data_func_(this->objectName(), button_id, QString::number(pos_y));
    assert(ok);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void FormDistributorSetting::on_button_sampling_injector_clicked()
{
    QString button_id = "button_sampling_injector";
    int pos_injector = ui->comboBox_sampling_injector->currentText().toInt();
    assert(pos_injector == 0 || pos_injector == 1);
    bool ok = write_data_func_(this->objectName(), button_id, QString::number(pos_injector));
    assert(ok);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void FormDistributorSetting::on_button_collection_injector_clicked()
{
    QString button_id = "button_collection_injector";
    int pos_injector = ui->comboBox_collection_injector->currentText().toInt();
    assert(pos_injector == 0 || pos_injector == 1);
    bool ok = write_data_func_(this->objectName(), button_id, QString::number(pos_injector));
    assert(ok);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
