#include <QStandardItemModel>
#include "databasemain.h"
#include "ui_databasemain.h"
#include "database.h"
#include "json.hpp"

DatabaseMain::DatabaseMain(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DatabaseMain)
{

    ui->setupUi(this);
    db.addObserver([this](const nlohmann::json &data){
        this->update(data);
    });
    tableModel = new QStandardItemModel(this);
    ui->table->setModel(tableModel);
    ui->dateEdit->setDate(QDate::currentDate());
    // Заголовки столбцов
    tableModel->setHorizontalHeaderLabels({"ID", "Имя", "Класс", "Пропусков"});

}


void DatabaseMain::update(const nlohmann::json &data) {
    // Очищаем старое
    tableModel->clear();
    // Заголовки столбцов
    tableModel->setHorizontalHeaderLabels({"ID", "Имя", "Класс", "Пропусков"});


    // Если в JSON нет нужного ключа — выходим
    if (!data.contains("spreadsheet")) return;

    // Перебираем учеников
    for (const auto &student : data["spreadsheet"]) {
        // Достаём значения (если ключа нет — подставляем 0 или "нет")
        QString name  = QString::fromStdString(student.value("name", "нет"));
        //int     id    = student.value("id", 0);
        int     level = student.value("level", 0);
        int     skips = student.value("skippsCount", 0);

        // Создаём ячейки строки
        //QStandardItem *itemID  = new QStandardItem(id);
        QStandardItem *itemName  = new QStandardItem(name);
        QStandardItem *itemLevel = new QStandardItem(QString::number(level));
        QStandardItem *itemSkips = new QStandardItem(QString::number(skips));

        // Добавляем строку в таблицу
        tableModel->appendRow({itemName, itemLevel, itemSkips});
    }

    ui->table->resizeColumnsToContents();
}

DatabaseMain::~DatabaseMain()
{
    delete ui;
}

void DatabaseMain::on_add_item_button_clicked()
{
    unsigned level;
    QRegularExpression lvlexp("([0-9]{1, 2})");
    if (lvlexp.match(ui->levelEdit->text()).hasMatch()) {
        level = ui->levelEdit->text().toInt();
    } else return;

    QString name;
    QRegularExpression nameexp("^((?i)[a-zа-я][^0-9]+[ ]?){2, 3}$");
    if (nameexp.match(ui->nameEdit->text()).hasMatch()) {
        name = ui->nameEdit->text();
    } else return;
    db.addStudent(name, level);
}

void DatabaseMain::on_add_item_button_2_clicked()
{
    try {
        db.addSkipping(0, QDate::currentDate(), "Math");
        QDate date;
        QRegularExpression dateexp("");
        qDebug() << ui->dateEdit->text();
    } catch (const std::runtime_error &e) {
        qDebug() << e.what();
    }
}

