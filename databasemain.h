#ifndef DATABASEMAIN_H
#define DATABASEMAIN_H

#include <QMainWindow>
#include <QStandardItemModel>
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
    void update();

    ~DatabaseMain() override;
private slots:
    void on_add_item_button_clicked();

    void on_add_item_button_2_clicked();

    void on_table_clicked(const QModelIndex &index);

    void on_removeSkippingButton_clicked();

    void on_removeStudentButton_clicked();

    void on_idEdit_textChanged(const QString &arg1);

    void on_comboBox_currentIndexChanged(int index);

    void on_hideFiltersButton_clicked();

    void on_skippsFromEdit_textChanged(const QString &arg1);

    void on_skippsToEdit_textChanged(const QString &arg1);

    void on_levelFromEdit_textChanged(const QString &arg1);

    void on_levelToEdit_textChanged(const QString &arg1);

    void on_hideAddDockButton_clicked();

private:
    Ui::DatabaseMain *ui;

    // База данных
    TDataBase *db;

    // Модель таблицы
    QStandardItemModel *tableModel;

    // Стандартные колонки
    const QStringList stdTableLabels = {"ID", "Имя", "Класс", "Пропусков"};

    // Режим сортировки и фильтрации
    sorts currentSortMode = sorts::ById;

    int selectedStudent = -1;
    int selectedSkip = -1;

    // Загрузка db из файла
    void tryLoadDB();

    //Сохранение db
    void trySaveDB();

    // Создание новой базы данных
    void newDB();

    // Добавление ученика
    void addStudent(const QString &name, const unsigned level);

    // Добавление пропуска ученики
    void addSkipping(const QString &name, const QDateTime date, unsigned subject);

    // Удаление записи из под выделения
    // Автоматически понимает, что хочет пользователь удалить
    void delUnderSelection();

    // Метод удаления студента
    void delStudent();

    // Метод удаления записи пропуска
    void delSkipping();

    // Очистка координат
    void clearCords();
};
#endif // DATABASEMAIN_H
