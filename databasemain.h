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
    ~DatabaseMain() override;

private:
    Ui::DatabaseMain *ui;
};
#endif // DATABASEMAIN_H
