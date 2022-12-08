#include "form_distributorsetting.h"
#include "ui_form_distributorsetting.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <thread>

FormDistributorSetting::FormDistributorSetting(QWidget *parent, bool admin) :
    FormCommon(parent, "distsetting", QString::fromUtf8("移液仪"), admin),
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
    typedef SamplingUIItem::Layout Layout;
    std::vector<SamplingUIItem::Layout> layout_sampling = {Layout::I, Layout::P, Layout::I,
                                  Layout::I, Layout::P, Layout::I, Layout::C,
                                  Layout::I, Layout::P, Layout::I,
                                  Layout::I, Layout::P, Layout::I, Layout::H,
                                  Layout::I, Layout::P, Layout::I,
                                  Layout::I, Layout::P, Layout::I, Layout::C,
                                  Layout::I, Layout::P, Layout::I,
                                  Layout::I, Layout::P, Layout::I};
    std::vector<Layout> layout_collection = {Layout::I, Layout::I, Layout::C,
                                  Layout::I, Layout::I, Layout::H,
                                  Layout::I, Layout::I, Layout::C,
                                  Layout::I, Layout::I};
    // 1 sampling home row, distributor X 0
    InitSamplingItem(45, 40, 15, 4, layout_sampling, 2 * (2 * 65)/* x_count * (2 * x_gap) */, 0, Layout::H);
    // 16 sampling rows, distributor X 1-16
    InitSamplingItem(45, 40, 15, 4, layout_sampling, 2 * (2 * 65)/* x_count * (2 * x_gap) */, 0, Layout::I);
    // 8 collection rows, distributor X 17-24
    InitSamplingItem(75, 65, 30, 2, layout_collection, 0/* pos_x */, 0/* pos_y */, Layout::I);
    // 2 sampling clean rows, distributor X 25-26
    InitSamplingItem(45, 40, 15, 4, layout_sampling, 2 * (2 * 65)/* x_count * (2 * x_gap) */, 0, Layout::C);
    // 2 collection clean rows, distributor X 25-26 (repeatedly redundant, required to be omitted)
    InitSamplingItem(75, 65, 30, 2, layout_collection, 0/* pos_x */, 0/* pos_y */, Layout::C);
    // 8 sampling purge rows, distributor X 27-34
    InitSamplingItem(45, 40, 15, 4, layout_sampling, 2 * (2 * 65)/* x_count * (2 * x_gap) */, 0, Layout::P);
    // 1 collection home row, distributor X 0, only for view
    InitSamplingItem(75, 65, 30, 2, layout_collection, 0/* pos_x */, 0/* pos_y */, Layout::H);

    auto scene = new QGraphicsScene(
                0, 0, /*x_count * (2 * x_gap)*/500,
                (32 + 3) * 35 + 10/*(y_count + 3) * y_gap + y_margin)*/);
    for (auto& item : sampling_ui_items_)
    {
        scene->addItem(item.get());
    }

    auto view = new QGraphicsView(scene);
    view->show();
    ui->verticalLayout->addWidget(view, 0, Qt::AlignTop | Qt::AlignLeft);
}

void FormDistributorSetting::InitSamplingItem(int x_gap, int y_gap, double radius,
                                                  int x_count, const std::vector<SamplingUIItem::Layout>& layout,
                                                  int pos_x, int pos_y, SamplingUIItem::Layout specified)
{
    int x_base = x_gap + 40;
    int y_base = y_gap;
    int num = 1;
    for (std::size_t i = 0; i < layout.size(); i++)
    {
        if (layout.at(i) == specified)
        {
            for (int j = 0; j < x_count; j++)
            {
                std::shared_ptr<SamplingUIItem> item;
                if (SamplingUIItem::Layout::I == specified)
                {
                    item = std::make_shared<SamplingUIItem>(radius, num);
                }
                else if (SamplingUIItem::Layout::P == specified)
                {
                    item = std::make_shared<SamplingUIItem>(
                                radius / 2, 0, 0, SamplingUIItem::SamplingUIItemType::Purge);
                }
                else if (SamplingUIItem::Layout::C == specified)
                {
                    item = std::make_shared<SamplingUIItem>(
                                radius / 2, 0, 0, SamplingUIItem::SamplingUIItemType::Clean,
                                SamplingUIItem::SamplingUIItemStatus::Undischarge);
                }
                else if (SamplingUIItem::Layout::H == specified)
                {
                    item =
                            std::make_shared<SamplingUIItem>(
                                radius / 2, 0, 0, SamplingUIItem::SamplingUIItemType::Home);
                }
                item->setPos(pos_x + x_base + j * x_gap, pos_y + y_base + i * y_gap);
                sampling_ui_items_.push_back(item);
                num++;
            }
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
                        SamplingUIItem::SamplingUIItemType::Home,
                        SamplingUIItem::SamplingUIItemStatus::Unsigned);
        item->setPos(pos_x + x * x_gap + 2 * x_gap, pos_y + radius);
        sampling_ui_items_.push_back(item);
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
    for (int i = 0; i <= 37; i++) // 36, 37 +/- limits, user gets the bottles.
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
    if (pos_x_ > 1 || pos_y_ > 35)
    {
        return;
    }
    for (auto& item : sampling_ui_items_)
    {
        item->SetStatus(SamplingUIItem::SamplingUIItemStatus::Unsigned, 0/*channel*/);
    }
    int LIQ_SAMPLING_SLOTS = 4;
    int LIQ_SAMPLING_GAP = 2;
    int LIQ_COLLECTION_SLOTS = 2;
    int LIQ_COLLECTION_GAP = 1;
    if (pos_y_ == 0 || pos_y_ == 35) // home rows
    {
        int selected = 0 + pos_x_ * LIQ_SAMPLING_GAP;
        sampling_ui_items_.at(selected)->SetStatus(
                    SamplingUIItem::SamplingUIItemStatus::Waiting, 0/*channel*/);
        sampling_ui_items_.at(selected + LIQ_SAMPLING_GAP)->SetStatus(
                    SamplingUIItem::SamplingUIItemStatus::Waiting, 0/*channel*/);

        selected = 17 * LIQ_SAMPLING_SLOTS + 8 * LIQ_COLLECTION_SLOTS +
                2 * LIQ_SAMPLING_SLOTS + 2 * LIQ_COLLECTION_SLOTS +
                8 * LIQ_SAMPLING_SLOTS;
        sampling_ui_items_.at(selected)->SetStatus(
                    SamplingUIItem::SamplingUIItemStatus::Waiting, 0/*channel*/);
        sampling_ui_items_.at(selected + LIQ_COLLECTION_GAP)->SetStatus(
                    SamplingUIItem::SamplingUIItemStatus::Waiting, 0/*channel*/);
    }
    else if (pos_y_ >= 1 && pos_y_ <= 16) // sampling rows
    {
        int selected = 1 * LIQ_SAMPLING_SLOTS;
        selected += (pos_y_ - 1) * LIQ_SAMPLING_SLOTS + pos_x_ * LIQ_SAMPLING_GAP;
        sampling_ui_items_.at(selected)->SetStatus(
                    SamplingUIItem::SamplingUIItemStatus::Waiting, 0/*channel*/);
        sampling_ui_items_.at(selected + LIQ_SAMPLING_GAP)->SetStatus(
                    SamplingUIItem::SamplingUIItemStatus::Waiting, 0/*channel*/);
    }
    else if (pos_y_ >= 17 && pos_y_ <= 24) // collection rows
    {
        int selected = 17 * LIQ_SAMPLING_SLOTS;
        selected += (pos_y_ - 17) * LIQ_COLLECTION_SLOTS;
        sampling_ui_items_.at(selected)->SetStatus(
                    SamplingUIItem::SamplingUIItemStatus::Waiting, 0/*channel*/);
        sampling_ui_items_.at(selected + LIQ_COLLECTION_GAP)->SetStatus(
                    SamplingUIItem::SamplingUIItemStatus::Waiting, 0/*channel*/);
    }
    else if (pos_y_ == 25 || pos_y_ == 26) // clean rows
    {
        int selected = 17 * LIQ_SAMPLING_SLOTS + 8 * LIQ_COLLECTION_SLOTS;
        selected += (pos_y_ - 25) * LIQ_SAMPLING_SLOTS + pos_x_ * LIQ_SAMPLING_GAP;
        sampling_ui_items_.at(selected)->SetStatus(
                    SamplingUIItem::SamplingUIItemStatus::Waiting, 0/*channel*/);
        sampling_ui_items_.at(selected + LIQ_SAMPLING_GAP)->SetStatus(
                    SamplingUIItem::SamplingUIItemStatus::Waiting, 0/*channel*/);

        selected = 17 * LIQ_SAMPLING_SLOTS + 8 * LIQ_COLLECTION_SLOTS +
                2 * LIQ_SAMPLING_SLOTS;
        selected += (pos_y_ - 25) * LIQ_COLLECTION_SLOTS;
        sampling_ui_items_.at(selected)->SetStatus(
                    SamplingUIItem::SamplingUIItemStatus::Waiting, 0/*channel*/);
        sampling_ui_items_.at(selected + LIQ_COLLECTION_GAP)->SetStatus(
                    SamplingUIItem::SamplingUIItemStatus::Waiting, 0/*channel*/);
    }
    else if (pos_y_ >= 27 && pos_y_ <= 34) // sampling N2 purge rows
    {
        int selected = 17 * LIQ_SAMPLING_SLOTS + 8 * LIQ_COLLECTION_SLOTS +
                2 * LIQ_SAMPLING_SLOTS + 2 * LIQ_COLLECTION_SLOTS;
        selected += (pos_y_ - 27) * LIQ_SAMPLING_SLOTS + pos_x_ * LIQ_SAMPLING_GAP;
        sampling_ui_items_.at(selected)->SetStatus(
                    SamplingUIItem::SamplingUIItemStatus::Waiting, 0/*channel*/);
        sampling_ui_items_.at(selected + LIQ_SAMPLING_GAP)->SetStatus(
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
