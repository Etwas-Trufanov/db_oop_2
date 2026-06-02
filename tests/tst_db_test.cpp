#include <QTest>
#include <QDate>
#include <QTemporaryFile>
#include <QFile>
#include <QTextStream>
#include <stdexcept>
#include <string>
#include <vector>
#include "database.h"

class TestDataBase : public QObject
{
    Q_OBJECT

public:
    TestDataBase() = default;

private slots:
    // ─── Базовая структура ───
    void test_emptyDatabase();
    void test_dataStructure();

    // ─── Студенты ───
    void test_addStudent();
    void test_addMultipleStudents();
    void test_removeStudent();
    void test_removeStudentInvalidId();
    void test_removeStudentNegativeId();

    // ─── Пропуски ───
    void test_addSkipping();
    void test_addMultipleSkippings();
    void test_removeSkipping();
    void test_removeSkippingInvalidStudentId();
    void test_removeSkippingInvalidSkippingId();

    // ─── Валидация ID ───
    void test_invalidStudentIdForSkipping();
    void test_negativeIds();

    // ─── Observer ───
    void test_observerCalledOnAddStudent();
    void test_observerCalledOnAddSkipping();
    void test_observerCalledOnRemoveStudent();
    void test_observerCalledOnRemoveSkipping();
    void test_observerMultipleObservers();
    void test_observerReceivesActualData();

    // ─── Сортировка ───
    void test_sortById();
    void test_sortByName();
    void test_sortByLevel();
    void test_sortByLevelInv();
    void test_sortBySkipps();
    void test_sortBySkippsInv();

    // ─── Фильтрация ───
    void test_filterByLevel();
    void test_filterBySkipps();
    void test_filterCombined();
    void test_filterNoMatch();

    // ─── Файловые операции ───
    void test_saveAndLoad();
    void test_loadFromNonExistentFile();
    void test_loadInvalidJson();
    void test_loadMissingSpreadsheet();

    // ─── Очистка ───
    void test_clear();

    // ─── Граничные случаи / баги ───
    void test_idReuseAfterRemove();   // документирует текущее поведение
    void test_addSkippingAfterRemoveStudent(); // ID проверяется по size, а не по фактическому ID
};

// ═══════════════════════════════════════════════════════════════
// Реализация тестов
// ═══════════════════════════════════════════════════════════════

void TestDataBase::test_emptyDatabase()
{
    TDataBase db;
    auto data = db.getAllData(sorts::ById, false, 0, 0, false, 0, 0);
    QVERIFY(data.is_object());
    QVERIFY(data.contains("spreadsheet"));
    QVERIFY(data["spreadsheet"].is_array());
    QCOMPARE(data["spreadsheet"].size(), static_cast<size_t>(0));
}

void TestDataBase::test_dataStructure()
{
    TDataBase db;
    db.addStudent("Ivanov", 3);

    auto data = db.getAllData(sorts::ById, false, 0, 0, false, 0, 0);
    auto& student = data["spreadsheet"][0];

    QVERIFY(student.is_object());
    QVERIFY(student.contains("id"));
    QVERIFY(student.contains("name"));
    QVERIFY(student.contains("level"));
    QVERIFY(student.contains("skippsCount"));
    QVERIFY(student.contains("skippings"));
    QVERIFY(student["skippings"].is_array());
}

// ─── Студенты ───

void TestDataBase::test_addStudent()
{
    TDataBase db;
    db.addStudent("Ivanov", 3);

    auto data = db.getAllData(sorts::ById, false, 0, 0, false, 0, 0);
    auto& s = data["spreadsheet"];

    QCOMPARE(s.size(), static_cast<size_t>(1));
    QCOMPARE(s[0]["id"].get<int>(), 0);
    QCOMPARE(s[0]["name"].get<std::string>(), std::string("Ivanov"));
    QCOMPARE(s[0]["level"].get<int>(), 3);
    QCOMPARE(s[0]["skippsCount"].get<int>(), 0);
    QVERIFY(s[0]["skippings"].empty());
}

void TestDataBase::test_addMultipleStudents()
{
    TDataBase db;
    db.addStudent("Ivanov", 3);
    db.addStudent("Petrov", 2);
    db.addStudent("Sidorov", 4);

    auto data = db.getAllData(sorts::ById, false, 0, 0, false, 0, 0);
    auto& s = data["spreadsheet"];

    qDebug().noquote() << s.dump(2);
    QCOMPARE(s.size(), static_cast<size_t>(3));
    QCOMPARE(s[0]["id"].get<int>(), 0);
    QCOMPARE(s[1]["id"].get<int>(), 1);
    QCOMPARE(s[2]["id"].get<int>(), 2);
}

void TestDataBase::test_removeStudent()
{
    TDataBase db;
    db.addStudent("Ivanov", 3);
    db.addStudent("Petrov", 2);
    db.addStudent("Sidorov", 4);

    db.removeStudent(1);

    auto data = db.getAllData(sorts::ById, false, 0, 0, false, 0, 0);
    auto& s = data["spreadsheet"];

    QCOMPARE(s.size(), static_cast<size_t>(2));
    QCOMPARE(s[0]["name"].get<std::string>(), std::string("Ivanov"));
    QCOMPARE(s[1]["name"].get<std::string>(), std::string("Sidorov"));
}

void TestDataBase::test_removeStudentInvalidId()
{
    TDataBase db;
    db.addStudent("Ivanov", 3);
    QVERIFY_EXCEPTION_THROWN(db.removeStudent(5), std::runtime_error);
}

void TestDataBase::test_removeStudentNegativeId()
{
    TDataBase db;
    db.addStudent("Ivanov", 3);
    QVERIFY_EXCEPTION_THROWN(db.removeStudent(-1), std::runtime_error);
}

// ─── Пропуски ───

void TestDataBase::test_addSkipping()
{
    TDataBase db;
    db.addStudent("Ivanov", 3);
    db.addSkipping(0, QDate(2024, 1, 15), "Math");

    auto data = db.getAllData(sorts::ById, false, 0, 0, false, 0, 0);
    auto& student = data["spreadsheet"][0];

    QCOMPARE(student["skippsCount"].get<int>(), 1);
    QCOMPARE(student["skippings"].size(), static_cast<size_t>(1));
    QCOMPARE(student["skippings"][0]["date"].get<std::string>(), std::string("Mon Jan 15 2024"));
    QCOMPARE(student["skippings"][0]["subject"].get<std::string>(), std::string("Math"));
}

void TestDataBase::test_addMultipleSkippings()
{
    TDataBase db;
    db.addStudent("Ivanov", 3);
    db.addSkipping(0, QDate(2024, 1, 15), "Math");
    db.addSkipping(0, QDate(2024, 1, 16), "Physics");
    db.addSkipping(0, QDate(2024, 1, 17), "Chemistry");

    auto data = db.getAllData(sorts::ById, false, 0, 0, false, 0, 0);
    auto& student = data["spreadsheet"][0];

    QCOMPARE(student["skippsCount"].get<int>(), 3);
    QCOMPARE(student["skippings"].size(), static_cast<size_t>(3));
}

void TestDataBase::test_removeSkipping()
{
    TDataBase db;
    db.addStudent("Ivanov", 3);
    db.addSkipping(0, QDate(2024, 1, 15), "Math");
    db.addSkipping(0, QDate(2024, 1, 16), "Physics");

    db.removeSkipping(0, 0);

    auto data = db.getAllData(sorts::ById, false, 0, 0, false, 0, 0);
    auto& student = data["spreadsheet"][0];

    QCOMPARE(student["skippings"].size(), static_cast<size_t>(1));
    QCOMPARE(student["skippings"][0]["subject"].get<std::string>(), std::string("Physics"));
    QCOMPARE(student["skippsCount"].get<int>(), 1);
}

void TestDataBase::test_removeSkippingInvalidStudentId()
{
    TDataBase db;
    db.addStudent("Ivanov", 3);
    QVERIFY_EXCEPTION_THROWN(db.removeSkipping(5, 0), std::runtime_error);
}

void TestDataBase::test_removeSkippingInvalidSkippingId()
{
    TDataBase db;
    db.addStudent("Ivanov", 3);
    db.addSkipping(0, QDate::currentDate(), "Math");
    QVERIFY_EXCEPTION_THROWN(db.removeSkipping(0, 5), std::runtime_error);
}

// ─── Валидация ID ───

void TestDataBase::test_invalidStudentIdForSkipping()
{
    TDataBase db;
    db.addStudent("Ivanov", 3);
    QVERIFY_EXCEPTION_THROWN(db.addSkipping(5, QDate::currentDate(), "Math"), std::runtime_error);
}

void TestDataBase::test_negativeIds()
{
    TDataBase db;
    db.addStudent("Ivanov", 3);

    QVERIFY_EXCEPTION_THROWN(db.addSkipping(-1, QDate::currentDate(), "Math"), std::runtime_error);
    QVERIFY_EXCEPTION_THROWN(db.removeStudent(-1), std::runtime_error);
    QVERIFY_EXCEPTION_THROWN(db.removeSkipping(0, -1), std::runtime_error);
    QVERIFY_EXCEPTION_THROWN(db.removeSkipping(-1, 0), std::runtime_error);
}

// ─── Observer ───

void TestDataBase::test_observerCalledOnAddStudent()
{
    TDataBase db;
    bool called = false;
    db.addObserver([&called]() { called = true; });

    db.addStudent("Ivanov", 3);
    QVERIFY(called);
}

void TestDataBase::test_observerCalledOnAddSkipping()
{
    TDataBase db;
    db.addStudent("Ivanov", 3);
    bool called = false;
    db.addObserver([&called]() { called = true; });

    db.addSkipping(0, QDate::currentDate(), "Math");
    QVERIFY(called);
}

void TestDataBase::test_observerCalledOnRemoveStudent()
{
    TDataBase db;
    db.addStudent("Ivanov", 3);
    bool called = false;
    db.addObserver([&called]() { called = true; });

    db.removeStudent(0);
    QVERIFY(called);
}

void TestDataBase::test_observerCalledOnRemoveSkipping()
{
    TDataBase db;
    db.addStudent("Ivanov", 3);
    db.addSkipping(0, QDate::currentDate(), "Math");
    bool called = false;
    db.addObserver([&called]() { called = true; });

    db.removeSkipping(0, 0);
    QVERIFY(called);
}

void TestDataBase::test_observerMultipleObservers()
{
    TDataBase db;
    int count = 0;
    db.addObserver([&count]() { count++; });
    db.addObserver([&count]() { count++; });
    db.addObserver([&count]() { count++; });

    db.addStudent("Ivanov", 3);
    QCOMPARE(count, 3);
}

void TestDataBase::test_observerReceivesActualData()
{
    TDataBase db;
    nlohmann::json observed;
    db.addObserver([&]() {
        observed = db.getAllData(sorts::ById, false, 0, 0, false, 0, 0);
    });

    db.addStudent("Ivanov", 3);
    QCOMPARE(observed["spreadsheet"].size(), static_cast<size_t>(1));
    QCOMPARE(observed["spreadsheet"][0]["name"].get<std::string>(), std::string("Ivanov"));
}

// ─── Сортировка ───

void TestDataBase::test_sortById()
{
    TDataBase db;
    db.addStudent("B", 2);
    db.addStudent("A", 1);

    auto data = db.getAllData(sorts::ById, false, 0, 0, false, 0, 0);
    auto& s = data["spreadsheet"];
    QCOMPARE(s[0]["id"].get<int>(), 0);
    QCOMPARE(s[1]["id"].get<int>(), 1);
}

void TestDataBase::test_sortByName()
{
    TDataBase db;
    db.addStudent("Zebra", 1);
    db.addStudent("Alpha", 2);
    db.addStudent("Mike", 3);

    auto data = db.getAllData(sorts::ByName, false, 0, 0, false, 0, 0);
    auto& s = data["spreadsheet"];

    QCOMPARE(s[0]["name"].get<std::string>(), std::string("Alpha"));
    QCOMPARE(s[1]["name"].get<std::string>(), std::string("Mike"));
    QCOMPARE(s[2]["name"].get<std::string>(), std::string("Zebra"));
}

void TestDataBase::test_sortByLevel()
{
    TDataBase db;
    db.addStudent("C", 5);
    db.addStudent("A", 1);
    db.addStudent("B", 3);

    auto data = db.getAllData(sorts::ByLevel, false, 0, 0, false, 0, 0);
    auto& s = data["spreadsheet"];

    QCOMPARE(s[0]["level"].get<int>(), 1);
    QCOMPARE(s[1]["level"].get<int>(), 3);
    QCOMPARE(s[2]["level"].get<int>(), 5);
}

void TestDataBase::test_sortByLevelInv()
{
    TDataBase db;
    db.addStudent("A", 1);
    db.addStudent("B", 3);
    db.addStudent("C", 5);

    auto data = db.getAllData(sorts::ByLevelInv, false, 0, 0, false, 0, 0);
    auto& s = data["spreadsheet"];

    QCOMPARE(s[0]["level"].get<int>(), 5);
    QCOMPARE(s[1]["level"].get<int>(), 3);
    QCOMPARE(s[2]["level"].get<int>(), 1);
}

void TestDataBase::test_sortBySkipps()
{
    TDataBase db;
    db.addStudent("A", 1);
    db.addStudent("B", 1);
    db.addStudent("C", 1);

    db.addSkipping(0, QDate::currentDate(), "Math");
    db.addSkipping(0, QDate::currentDate(), "Math");
    db.addSkipping(2, QDate::currentDate(), "Math");

    auto data = db.getAllData(sorts::BySkipps, false, 0, 0, false, 0, 0);
    auto& s = data["spreadsheet"];

    QCOMPARE(s[0]["skippsCount"].get<int>(), 0); // B
    QCOMPARE(s[1]["skippsCount"].get<int>(), 1); // C
    QCOMPARE(s[2]["skippsCount"].get<int>(), 2); // A
}

void TestDataBase::test_sortBySkippsInv()
{
    TDataBase db;
    db.addStudent("A", 1);
    db.addStudent("B", 1);
    db.addStudent("C", 1);

    db.addSkipping(0, QDate::currentDate(), "Math");
    db.addSkipping(0, QDate::currentDate(), "Math");
    db.addSkipping(2, QDate::currentDate(), "Math");

    auto data = db.getAllData(sorts::BySkippsInv, false, 0, 0, false, 0, 0);
    auto& s = data["spreadsheet"];

    QCOMPARE(s[0]["skippsCount"].get<int>(), 2); // A
    QCOMPARE(s[1]["skippsCount"].get<int>(), 1); // C
    QCOMPARE(s[2]["skippsCount"].get<int>(), 0); // B
}

// ─── Фильтрация ───

void TestDataBase::test_filterByLevel()
{
    TDataBase db;
    db.addStudent("A", 1);
    db.addStudent("B", 3);
    db.addStudent("C", 5);
    db.addStudent("D", 7);

    auto data = db.getAllData(sorts::ById, true, 3, 5, false, 0, 0);
    auto& s = data["spreadsheet"];

    QCOMPARE(s.size(), static_cast<size_t>(2));
    QCOMPARE(s[0]["name"].get<std::string>(), std::string("B"));
    QCOMPARE(s[1]["name"].get<std::string>(), std::string("C"));
}

void TestDataBase::test_filterBySkipps()
{
    TDataBase db;
    db.addStudent("A", 1);
    db.addStudent("B", 1);
    db.addStudent("C", 1);

    db.addSkipping(0, QDate::currentDate(), "Math");
    db.addSkipping(0, QDate::currentDate(), "Math");
    db.addSkipping(2, QDate::currentDate(), "Math");

    auto data = db.getAllData(sorts::ById, false, 0, 0, true, 1, 2);
    auto& s = data["spreadsheet"];

    QCOMPARE(s.size(), static_cast<size_t>(2));
    QCOMPARE(s[0]["name"].get<std::string>(), std::string("A"));
    QCOMPARE(s[1]["name"].get<std::string>(), std::string("C"));
}

void TestDataBase::test_filterCombined()
{
    TDataBase db;
    db.addStudent("A", 1);
    db.addStudent("B", 3);
    db.addStudent("C", 3);

    db.addSkipping(0, QDate::currentDate(), "Math");
    db.addSkipping(0, QDate::currentDate(), "Math");
    db.addSkipping(1, QDate::currentDate(), "Math");

    auto data = db.getAllData(sorts::ById, true, 3, 3, true, 1, 1);
    auto& s = data["spreadsheet"];

    QCOMPARE(s.size(), static_cast<size_t>(1));
    QCOMPARE(s[0]["name"].get<std::string>(), std::string("B"));
}

void TestDataBase::test_filterNoMatch()
{
    TDataBase db;
    db.addStudent("A", 1);
    db.addStudent("B", 2);

    auto data = db.getAllData(sorts::ById, true, 10, 20, false, 0, 0);
    auto& s = data["spreadsheet"];

    QCOMPARE(s.size(), static_cast<size_t>(0));
}

// ─── Файловые операции ───

void TestDataBase::test_saveAndLoad()
{
    QString path = QDir::tempPath() + "/test_db_save_load.json";

    {
        TDataBase db;
        db.addStudent("Ivanov", 3);
        db.addStudent("Petrov", 2);
        db.addSkipping(0, QDate(2024, 1, 15), "Math");
        db.saveDateBase(path);
    }

    {
        TDataBase db;
        db.loadFromFile(path);
        auto data = db.getAllData(sorts::ById, false, 0, 0, false, 0, 0);
        auto& s = data["spreadsheet"];

        QCOMPARE(s.size(), static_cast<size_t>(2));
        QCOMPARE(s[0]["name"].get<std::string>(), std::string("Ivanov"));
        QCOMPARE(s[0]["level"].get<int>(), 3);
        QCOMPARE(s[0]["skippsCount"].get<int>(), 1);
        QCOMPARE(s[0]["skippings"][0]["subject"].get<std::string>(), std::string("Math"));
        QCOMPARE(s[1]["name"].get<std::string>(), std::string("Petrov"));
    }

    QFile::remove(path);
}

void TestDataBase::test_loadFromNonExistentFile()
{
    TDataBase db;
    db.loadFromFile("/nonexistent/path/file.json");
    // не должно быть исключений, просто пустая БД
    auto data = db.getAllData(sorts::ById, false, 0, 0, false, 0, 0);
    QCOMPARE(data["spreadsheet"].size(), static_cast<size_t>(0));
}

void TestDataBase::test_loadInvalidJson()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    file.write("{ invalid json }");
    file.flush();

    TDataBase db;
    // выбрасывает std::runtime_error
    QVERIFY_EXCEPTION_THROWN(db.loadFromFile(file.fileName()), std::runtime_error);
}

void TestDataBase::test_loadMissingSpreadsheet()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    file.write(R"({"other_key": []})");
    file.flush();

    TDataBase db;
    db.loadFromFile(file.fileName());
    // если нет spreadsheet, data не меняется — остаётся пустой
    auto data = db.getAllData(sorts::ById, false, 0, 0, false, 0, 0);
    QCOMPARE(data["spreadsheet"].size(), static_cast<size_t>(0));
}

// ─── Очистка ───

void TestDataBase::test_clear()
{
    TDataBase db;
    db.addStudent("Ivanov", 3);
    db.addSkipping(0, QDate::currentDate(), "Math");
    db.clear();

    auto data = db.getAllData(sorts::ById, false, 0, 0, false, 0, 0);
    QCOMPARE(data["spreadsheet"].size(), static_cast<size_t>(0));
}

// ─── Граничные случаи / баги ───

void TestDataBase::test_idReuseAfterRemove()
{
    // Документирует текущее поведение:
    // после removeStudent ID не пересчитываются,
    // а новый студент получает ID = size(), что может привести к дублированию ID.
    TDataBase db;
    db.addStudent("A", 1); // id=0
    db.addStudent("B", 2); // id=1
    db.addStudent("C", 3); // id=2

    db.removeStudent(1); // остались id=0, id=2

    db.addStudent("D", 4); // id=3

    auto data = db.getAllData(sorts::ById, false, 0, 0, false, 0, 0);
    auto& s = data["spreadsheet"];

    QCOMPARE(s.size(), static_cast<size_t>(3));
    int id0 = s[0]["id"].get<int>();
    int id1 = s[1]["id"].get<int>();
    int id2 = s[2]["id"].get<int>();

    QVERIFY(id0 == 0);
    QVERIFY(id1 == 2);
    QVERIFY(id2 == 3);
}

void TestDataBase::test_addSkippingAfterRemoveStudent()
{

    TDataBase db;
    db.addStudent("A", 1); // id=0
    db.addStudent("B", 2); // id=1
    db.addStudent("C", 3); // id=2

    db.removeStudent(1); // остались id=0, id=2. size=2

    db.addSkipping(2, QDate::currentDate(), "Math");
    auto d = db.getAllData(sorts::ById, false, 0, 0, false, 0, 0);
    nlohmann::json &a = d["spreadsheet"][1];
    QVERIFY(a["skippings"][0]["subject"].get<std::string>() == "Math");
}

QTEST_MAIN(TestDataBase)
#include "tst_db_test.moc"
