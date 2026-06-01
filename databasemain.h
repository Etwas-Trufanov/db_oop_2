#ifndef DATABASEMAIN_H
#define DATABASEMAIN_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "json.hpp"
#include "database.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class DatabaseMain;
}
QT_END_NAMESPACE

class DatabaseMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit DatabaseMain(QWidget *parent = nullptr);

    // Фунция обновления
    void update(const nlohmann::json &a);

    ~DatabaseMain() override;
private slots:
    void on_add_item_button_clicked();

    void on_add_item_button_2_clicked();

private:
    Ui::DatabaseMain *ui;

    TDataBase db;

    QStandardItemModel *tableModel;

    const QString stdSavePath = R"(~/.local/share/OOP2-BD/db.json)";

    // Загрузка db из файла
    void tryLoadDB();
    void tryLoadDB(const QString &path);

    //Сохранение db
    void trySaveDB();
    void trySaveDB(const QString &path);

    // Добавление ученика
    void addStudent(const QString &name, const unsigned level);

    // Добавление пропуска ученики
    void addSkipping(const QString &name, const QDateTime date, unsigned subject);

};
#endif // DATABASEMAIN_H
