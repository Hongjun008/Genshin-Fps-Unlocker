#include "wabout.h"

#include "framework.h"
#include "ui_wabout.h"

WAbout::WAbout(QWidget *parent) : QDialog(parent, Qt::WindowTitleHint | Qt::CustomizeWindowHint),
                                  ui(new Ui::WAbout)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());
    connect(ui->btn_OK, &QPushButton::clicked, this, &QWidget::close);
}

WAbout::~WAbout()
{
    delete ui;
}
