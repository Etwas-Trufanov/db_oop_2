#include "databasemain.h"
#include "ui_databasemain.h"
#include "database.h"

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

void DatabaseMain::update(const QJsonObject &object) {
    //ui->tableView.
}