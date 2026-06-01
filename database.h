#ifndef DATABASE_H
#define DATABASE_H

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

class TDataBase {
private:
    QJsonArray data;
    std::vector<std::function<void(const QJsonArray&)>> observers;
public:
    // Простой инициализатор
    TDataBase() : data(QJsonArray()) {};

    // С загрузкой из файла
    TDataBase(const QString &path);

    // Сохранение файла
    void saveDateBase(const QString &path);

    // Добавления наблюдателя
    void addObserver(std::function<void(const QJsonArray&)> observer) {
        observers.push_back(observer);
    };

    // Добавление студента в базу
    void addStudent(const QString &name, int level) {
        QJsonObject student;
        student["name"] = name;
        student["level"] = level;
        student["skippings"] = QJsonArray();
        data.append(student);
    }

    void addSkipping(int ID, const QDate &date, const QString &subject) {
        if (ID >= data.size()) throw(std::runtime_error("Data does not contain student with ID " + std::to_string(ID)));

        QJsonObject skipping;
        skipping["data"] = date.toString();
        skipping["subject"] = subject;
        (data[ID]).toObject()["skippings"].toArray().append(skipping);
    }

    void removeStudent(const int ID) {
        if (ID >= data.size()) throw(std::runtime_error("Data does not contain student with ID " + std::to_string(ID)));
        data.removeAt(ID);
    }

    void removeSkipping(const int studentID, const int skippingID) {
        if (studentID >= data.size()) throw(std::runtime_error("Data does not contain student with ID " + std::to_string(studentID)));
        if (skippingID >= data[studentID].toArray().size()) throw(std::runtime_error("Skippings does not contain skipping with ID " + std::to_string(skippingID)));

        data[studentID].toObject()["skippings"].toArray().removeAt(skippingID);
    }

    // Только для дебага, удалить
    void show() {
        qDebug() << "Base data";
        QJsonObject obj;
        obj["database"] = data;
        QJsonDocument doc(obj);
        qDebug().noquote() << doc.toJson(QJsonDocument::Indented);
    }
};


#endif // DATABASE_H
