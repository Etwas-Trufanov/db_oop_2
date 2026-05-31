#ifndef DATABASEMAIN_H
#define DATABASEMAIN_H

#include <QMainWindow>

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

    void updateTable();

    ~DatabaseMain() override;
private:
    Ui::DatabaseMain *ui;

    QJsonDocument db();

    const QString stdSavePath = R"(~/.local/share/OOP2-BD/db.json)";

    // Загрузка db из файла
    void tryLoadDB();
    void tryLoadDB(const QString &path);

    //Сохранение db
    void trySaveDB();
    void trySaveDB(const QString &path);

    // Добавление ученика
    bool addStudent(const QString &name, const unsigned level);

    // Добавление пропуска ученики
    bool addSkipping(const QString &name, const QDateTime date, unsigned subject);

};
#endif // DATABASEMAIN_H
