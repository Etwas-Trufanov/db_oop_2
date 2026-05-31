#include "databasemain.h"
#include "./ui_databasemain.h"

DatabaseMain::DatabaseMain(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DatabaseMain)
{
    ui->setupUi(this);
}

DatabaseMain::~DatabaseMain()
{
    delete ui;
}

