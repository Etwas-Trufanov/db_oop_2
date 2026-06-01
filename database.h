#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <qdatetime.h>
#include "json.hpp"

class TDataBase {
private:
    nlohmann::json data;
    std::vector<std::function<void(const nlohmann::json&)>> observers;
public:
    // Простой инициализатор
    TDataBase() {
        data = nlohmann::json();
        data["spreadsheet"] = nlohmann::json::array();
    };

    // С загрузкой из файла
    TDataBase(const QString &path) {};

    // Сохранение файла
    void saveDateBase(const QString &path);

    // Добавления наблюдателя
    void addObserver(std::function<void(const nlohmann::json&)> observer) {
        observers.push_back(observer);
    };

    // Добавление ученика в базу
    // name - имя ученика
    // level - класс/курс
    void addStudent(const QString &name, int level) {
        data["spreadsheet"].push_back
            (
            {
                {"name", name.toStdString()},
                {"level", level},
                {"skippsCount", 0},
                {"skippings", nlohmann::json::array()}
            }
            );
    }

    // Добавление пропуска ученика с пробросом исключения
    // ID - ID ученика
    // date - дата пропуска
    // subject - предмет
    void addSkipping(int ID, const QDate &date, const QString &subject) {
        if (ID >= data["spreadsheet"].size() or ID < 0) throw(std::runtime_error("Data does not contain student with ID " + std::to_string(ID)));

        data["spreadsheet"][ID]["skippings"].push_back
            (
            {
                {"date", date.toString().toStdString()},
                {"subject", subject.toStdString()}
            }
            );
        data["spreadsheet"][ID]["skippsCount"] = data["spreadsheet"][ID]["skippsCount"].get<int>() + 1;
    }

    // Удаление студента с пробросом исключения
    // ID - ID студента
    void removeStudent(const int ID) {
        if (ID >= data["spreadsheet"].size() or ID < 0) throw(std::runtime_error("Data does not contain student with ID " + std::to_string(ID)));

        data["spreadsheet"].erase(data["spreadsheet"].begin()+ID);
    }

    // Удаление пропуска ученика с пробросом исключения
    // studentID - ID ученика
    // skippingID - номер пропуска
    void removeSkipping(const int studentID, const int skippingID) {
        if (studentID >= data["spreadsheet"].size() or studentID < 0) throw(std::runtime_error("Data does not contain student with ID " + std::to_string(studentID)));
        if (skippingID >= data["spreadsheet"][studentID]["skippings"].size() or skippingID < 0) throw(std::runtime_error("Skippings does not contain skipping with ID " + std::to_string(skippingID)));

        data["spreadsheet"][studentID]["skippings"].erase(data["spreadsheet"][studentID]["skippings"].begin()+skippingID);
        data["spreadsheet"][studentID]["skippsCount"] += -1;
    }

    // Только для дебага, удалить
    void show() {
        qDebug() << "=============== Вывод БД ===============";
        qDebug() << data.dump(2);
    }

    const nlohmann::json& getAllData() const {
        return data;
    }
};


#endif // DATABASE_H
