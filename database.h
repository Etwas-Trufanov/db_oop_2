#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <qdatetime.h>
#include <qdir.h>
#include "json.hpp"

enum sorts {
    ById,
    BySkipps,
    BySkippsInv,
    ByLevel,
    ByLevelInv,
    ByName
};

class TDataBase {
private:
    nlohmann::json data;

    nlohmann::json cache;

    std::vector<std::function<void()>> observers;

    void callObservers() {
        for (std::function<void()> &i : observers) {
            i();
        }
    }
public:
    // Простой инициализатор
    TDataBase() {
        data = nlohmann::json();
        data["spreadsheet"] = nlohmann::json::array();
    };

    // Загрузка БД из
    // Выбрасывает исключения
    // &path - путь до файла
    void loadFromFile(const QString &path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return; // Файла нет или не открывается — просто выходим
        }

        QString content = file.readAll();
        file.close();

        try {
            nlohmann::json loaded = nlohmann::json::parse(content.toStdString());
            if (loaded.contains("spreadsheet") && loaded["spreadsheet"].is_array()) {
                data = loaded;
                callObservers();
            }
        } catch (std::runtime_error &e) {
            throw;
        } catch (nlohmann::json::parse_error &e) {
            throw(std::runtime_error(e.what()));
        }
    }

    // Сохранение бд
    // Выбрасывает исключения
    // &path - путь до файла
    void saveDateBase(const QString &path) {
        QDir dir;
        dir.mkpath(QFileInfo(path).path()); // Создаём папки, если их нет

        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return;
        }

        QTextStream out(&file);
        out << QString::fromStdString(data.dump(4));
        file.close();
    }

    // Добавления наблюдателя
    void addObserver(std::function<void()> observer) {
        observers.push_back(observer);
    };

    // Добавление ученика в базу
    // name - имя ученика
    // level - класс/курс
    void addStudent(const QString &name, int level) {
        // Ищем свободный индекс
        int nextIndex = 0;
        if (data["spreadsheet"].size() > 0) nextIndex = data["spreadsheet"][data["spreadsheet"].size()-1]["id"].get<int>()+1;

        data["spreadsheet"].push_back
            (
            {
                {"id", nextIndex},
                {"name", name.toStdString()},
                {"level", level},
                {"skippsCount", 0},
                {"skippings", nlohmann::json::array()}
            }
            );

        callObservers();
    }

    // Добавление пропуска ученика с пробросом исключения
    // ID - ID ученика
    // date - дата пропуска
    // subject - предмет
    void addSkipping(int ID, const QDate &date, const QString &subject) {

        for (auto &i : data["spreadsheet"]) {
            if (i["id"].get<int>() == ID) {
                i["skippings"].push_back
                    (
                    {
                        {"date", date.toString().toStdString()},
                        {"subject", subject.toStdString()}
                    }
                    );
                i["skippsCount"] = i["skippsCount"].get<int>() + 1;
                callObservers();
                return;
            }
        }
        callObservers();
        throw(std::runtime_error("Data does not contain student with ID " + std::to_string(ID)));
    }

    // Удаление студента с пробросом исключения
    // ID - ID студента
    void removeStudent(const int ID) {
        for (unsigned i = 0; i < data["spreadsheet"].size(); i++) {
            if (data["spreadsheet"][i]["id"].get<int>() == ID) {
                data["spreadsheet"].erase(data["spreadsheet"].begin()+i);
                callObservers();
                return;
            }
        }
        callObservers();
        throw(std::runtime_error("Data does not contain student with ID " + std::to_string(ID)));
    }

    // Удаление пропуска ученика с пробросом исключения
    // studentID - ID ученика
    // skippingID - номер пропуска
    void removeSkipping(const int studentID, const int skippingID) {
        if (studentID >= data["spreadsheet"].size() or studentID < 0)
            throw(std::runtime_error("Data does not contain student with ID " + std::to_string(studentID)));

        for (auto &i : data["spreadsheet"]) {
            if (i["id"].get<int>() == studentID) {
                if (skippingID >= i["skippings"].size() or skippingID < 0)
                    throw(std::runtime_error("Skippings does not contain skipping with ID " + std::to_string(skippingID)));

                i["skippings"].erase(i["skippings"].begin() + skippingID);
                i["skippsCount"] = i["skippsCount"].get<int>() - 1;
                callObservers();
                return;
            }
        }
        throw(std::runtime_error("Data does not contain student with ID " + std::to_string(studentID)));
    }

    // Геттер всей БД
    // sort - режим сортировки
    // filterLevel - требуется ли фильтрация по уровню
    // fromLevel, toLevel - границы
    // filterSkipps - требуется ли фильтрация по пропускам
    // fromSkipps, toSkipps - границы
    const nlohmann::json& getAllData(sorts sort, bool filterLevel, int fromLevel, int toLevel, bool filterSkipps, int fromSkipps, int toSkipps) {
        cache = nlohmann::json::object();
        cache["spreadsheet"] = nlohmann::json::array();
        for (auto i : data["spreadsheet"]) {
            bool ok = true;

            if (filterLevel) {
                ok = (i["level"] >= fromLevel && i["level"] <= toLevel);
            }

            if (ok && filterSkipps) {
                ok = (i["skippsCount"] >= fromSkipps && i["skippsCount"] <= toSkipps);
            }

            if (ok) cache["spreadsheet"].push_back(i);
        }

        // Пузырьковая сортировка
        for (size_t i = 0; i < cache["spreadsheet"].size(); ++i) {
            for (size_t j = 0; j + 1 < cache["spreadsheet"].size() - i; ++j) {
                bool needSwap = false;

                switch (sort) {
                case ById:        needSwap = cache["spreadsheet"][j]["id"]           > cache["spreadsheet"][j+1]["id"];          break;
                case BySkipps:    needSwap = cache["spreadsheet"][j]["skippsCount"]  > cache["spreadsheet"][j+1]["skippsCount"];  break;
                case BySkippsInv: needSwap = cache["spreadsheet"][j]["skippsCount"]  < cache["spreadsheet"][j+1]["skippsCount"];  break;
                case ByLevel:     needSwap = cache["spreadsheet"][j]["level"]        > cache["spreadsheet"][j+1]["level"];       break;
                case ByLevelInv:  needSwap = cache["spreadsheet"][j]["level"]        < cache["spreadsheet"][j+1]["level"];       break;
                case ByName:      needSwap = cache["spreadsheet"][j]["name"]         > cache["spreadsheet"][j+1]["name"];        break;
                }

                if (needSwap) {
                    std::swap(cache["spreadsheet"][j], cache["spreadsheet"][j+1]);
                }
            }
        }
        return cache;
    }


    // Очистка бд
    void clear() {
        data.clear();
        data["spreadsheet"] = nlohmann::json::array();
    }
};


#endif // DATABASE_H
