#include <QTest>

// add necessary includes here

class db_test : public QObject
{
    Q_OBJECT

public:
    db_test();
    ~db_test() override;

private slots:
    void test_case1();
};

db_test::db_test() {}

db_test::~db_test() = default;

void db_test::test_case1() {}

QTEST_APPLESS_MAIN(db_test)
