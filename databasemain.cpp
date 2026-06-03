#include <QStandardItemModel>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <climits>
#include "databasemain.h"
#include "ui_databasemain.h"
#include "database.h"

DatabaseMain::DatabaseMain(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DatabaseMain)
{
    // Иницмализация UI
    ui->setupUi(this);
    // Инициализация БД
    db = new TDataBase();
    // Подключаем метод наблюдатель
    db->addObserver([this](){
        this->update();
    });
    // Инициазируем модель таблицы
    tableModel = new QStandardItemModel(this);
    ui->table->setModel(tableModel);
    // Устанавливаем в календарь текущую дату
    ui->dateEdit->setDate(QDate::currentDate());
    // Заголовки столбцов
    tableModel->setHorizontalHeaderLabels(stdTableLabels);

    // Подключаем всякое, что через формочки подкдючить нельзя
    connect(ui->newAction, &QAction::triggered, this, &DatabaseMain::newDB);
    connect(ui->loadAction, &QAction::triggered, this, &DatabaseMain::tryLoadDB);
    connect(ui->saveAction, &QAction::triggered, this, &DatabaseMain::trySaveDB);
    connect(ui->closeAction, &QAction::triggered, this, &DatabaseMain::close);
    connect(ui->actionDelete_under_selection, &QAction::triggered, this, &DatabaseMain::delUnderSelection);

    connect(ui->actionShow_sort_bar, &QAction::triggered, this, [&](){
        ui->sortBarWidget->show();
    });
    connect(ui->actionShow_add_dock, &QAction::triggered, this, [&](){
        ui->addDockWidget->show();
    });

    connect(ui->actionHide_add_dock, &QAction::triggered, this, [&](){
        ui->addDockWidget->hide();
    });
    connect(ui->actionHide_sort_bar, &QAction::triggered, this, [&](){
        ui->sortBarWidget->hide();
    });

    connect(ui->actionHide_all, &QAction::triggered, this, [&](){
        ui->sortBarWidget->hide();
        ui->addDockWidget->hide();
    });
    connect(ui->actionShow_all, &QAction::triggered, this, [&](){
        ui->sortBarWidget->show();
        ui->addDockWidget->show();
    });
}


void DatabaseMain::update() {
    // Очищаем старое
    tableModel->clear();

    // Получаем значения фильтров
    int filterValueLevelFrom = INT_MIN; bool fLF = false;
    int filterValueLevelTo = INT_MAX; bool fLT = false;

    filterValueLevelFrom = ui->levelFromEdit->text().toInt(&fLF);
    filterValueLevelTo = ui->levelToEdit->text().toInt(&fLT);


    int filterValueSkippsFrom = INT_MIN; bool fSF = false;
    int filterValueSkippsTo = INT_MAX; bool fST = false;

    filterValueSkippsFrom = ui->skippsFromEdit->text().toInt(&fSF);
    filterValueSkippsTo = ui->skippsToEdit->text().toInt(&fST);

    // Получаем базу данных
    const nlohmann::json &data = db->getAllData(currentSortMode,
                        fLF or fLT, fLF ? filterValueLevelFrom : INT_MIN, fLT ? filterValueLevelTo : INT_MAX,
                        fSF or fST, fSF ? filterValueSkippsFrom : INT_MIN, fST ? filterValueSkippsTo : INT_MAX);

    // Если в JSON нет нужного ключа — выходим
    if (!data.contains("spreadsheet")) return;

    // Начинаем генерировать лейблы колонок
    auto tableLabels = stdTableLabels;

    // Считаем максимальное количество пропусков у одного ученика
    unsigned collumnCount = 0;
    for (auto &s : data["spreadsheet"]) {
        collumnCount = s["skippsCount"].get<int>() > int(collumnCount)
        ? s["skippsCount"].get<int>()
        : collumnCount;
    }

    // Добавляем заголовки для колонок пропусков (по 2 на каждый пропуск)
    for (unsigned i = 0; i < collumnCount; i++) {
        tableLabels.append(QString::number(i));
        tableLabels.append(QString(""));
    }

    // Заголовки столбцов
    tableModel->setHorizontalHeaderLabels(tableLabels);

    // Перебираем учеников
    for (const auto &student : data["spreadsheet"]) {
        QString name  = QString::fromStdString(student.value("name", "нет"));
        int     id    = student.value("id", 0);
        int     level = student.value("level", 0);
        int     skips = student.value("skippsCount", 0);

        // Создаём ячейки базовых колонок
        QList<QStandardItem*> rowItems;
        rowItems << new QStandardItem(QString::number(id))
                 << new QStandardItem(name)
                 << new QStandardItem(QString::number(level))
                 << new QStandardItem(QString::number(skips));

        // Заполняем пропуски (дата + предмет)
        if (skips > 0 && student.contains("skippings")) {
            for (const auto &skip : student["skippings"]) {
                QString dateStr = QString::fromStdString(skip.value("date", ""));
                QString subjStr = QString::fromStdString(skip.value("subject", ""));
                rowItems << new QStandardItem(dateStr)
                         << new QStandardItem(subjStr);
            }
        }

        // Если у этого ученика пропусков меньше максимума -> забиваем пустыми ячейками
        int missing = (collumnCount - skips) * 2;
        for (int i = 0; i < missing; ++i) {
            rowItems << new QStandardItem("");
        }

        // Добавляем готовую строку в модель
        tableModel->appendRow(rowItems);
    }

    // Чтоб всё влезло
    ui->table->resizeColumnsToContents();
}

// Деструктор
DatabaseMain::~DatabaseMain()
{
    delete tableModel;
    delete db;
    delete ui;
}

// На нажатие кнопки добавление ученика
void DatabaseMain::on_add_item_button_clicked()
{
    // Проверяем корректность класса
    unsigned level;
    QRegularExpression lvlexp("(^[0-9]{1, 2}$)");
    if (lvlexp.match(ui->levelEdit->text()).hasMatch()) {
        level = ui->levelEdit->text().toInt();
    } else {  return; }
    // Проверям корректность имени
    QString name;
    QRegularExpression nameexp("^((?i)[a-zа-я][^0-9]+[ ]?){2, 3}$");
    if (nameexp.match(ui->nameEdit->text()).hasMatch()) {
        name = ui->nameEdit->text();
    } else { return; }
    db->addStudent(name, level);
}

// Добавление пропуска ученику
void DatabaseMain::on_add_item_button_2_clicked()
{
    if (selectedStudent < 0) return;
    try {
        auto tmpID = ui->idEdit->text().toInt();
        db->addSkipping(tmpID, ui->dateEdit->date(), ui->subjectComboBox->currentText());
        qDebug() << ui->dateEdit->text();
    } catch (const std::runtime_error &e) {
        qDebug() << e.what();
        ui->errorLabel->setText(e.what());
        QTimer::singleShot(4000, this, [&]() {
            ui->errorLabel->clear();
        });
    }
}

// Выделение записей в таблице
void DatabaseMain::on_table_clicked(const QModelIndex &index)
{
    // Вычисление и получние ID ученика и позиции пропуска
    selectedStudent = tableModel->index(index.row(), 0).data().toInt();
    ui->idEdit->setText(QString::number(selectedStudent));
    selectedSkip = index.column()-4;
    selectedSkip = selectedSkip>-1 ? selectedSkip/2 : -1;
    qDebug() << "sstud: "<< selectedStudent << " sskip: " << selectedSkip;
    ui->removeSkippingButton->setEnabled( selectedSkip>-1 ? true : false );
    ui->removeStudentButton->setEnabled( selectedStudent>-1 ? true : false );

    // Красиво выделяем
    if (index.column() < 4) {
        QModelIndex targetIndex = ui->table->model()->index(index.row(), 0);
        ui->table->selectionModel()->select(targetIndex, QItemSelectionModel::Select);
        targetIndex = ui->table->model()->index(index.row(), 1);
        ui->table->selectionModel()->select(targetIndex, QItemSelectionModel::Select);
        targetIndex = ui->table->model()->index(index.row(), 2);
        ui->table->selectionModel()->select(targetIndex, QItemSelectionModel::Select);
        targetIndex = ui->table->model()->index(index.row(), 3);
        ui->table->selectionModel()->select(targetIndex, QItemSelectionModel::Select);
    } else if (index.column() > 3) {
        QModelIndex targetIndex = ui->table->model()->index(index.row(), selectedSkip*2+4);
        ui->table->selectionModel()->select(targetIndex, QItemSelectionModel::Select);
        targetIndex = ui->table->model()->index(index.row(), selectedSkip*2+5);
        ui->table->selectionModel()->select(targetIndex, QItemSelectionModel::Select);
    }
    // Выводим в статусбар
    QString tmp = "Student ID: " + (selectedStudent < 0 ? "no" : QString::number(selectedStudent)) + "; Skipping: " + (selectedSkip < 0 ? "no" : QString::number(selectedSkip));
    ui->cordinatesLabel->setText(tmp);
}

// Удаление пропуска
void DatabaseMain::on_removeSkippingButton_clicked()
{
    delSkipping();
}

// Удаление ученика
void DatabaseMain::on_removeStudentButton_clicked()
{
    delStudent();
}

// На редактировние поля ввода ID ученика в добавлении пропуска
void DatabaseMain::on_idEdit_textChanged(const QString &arg1)
{
    bool ok;
    auto tmp = arg1.toInt(&ok);
    ui->add_item_button_2->setEnabled((ok and tmp>-1));
    selectedStudent = tmp;
}

// Выбор режима сортировки
void DatabaseMain::on_comboBox_currentIndexChanged(int index)
{
    currentSortMode = sorts(index);
    clearCords();
    update();
}

// Создание новой бд
void DatabaseMain::newDB() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Сохранить файл"),
        "",
        tr("Json файлы (*.json);;Все файлы (*.*)")
        );

    if (!fileName.isEmpty()) {
        try {
            db->saveDateBase(fileName);
        } catch (std::runtime_error &e){
            qDebug() << e.what();
            ui->errorLabel->setText(e.what());
            QTimer::singleShot(4000, this, [&]() {
                ui->errorLabel->clear();
            });
        }
    }
    db->clear();
    clearCords();
}

// Загрузка db из файла
void DatabaseMain::tryLoadDB() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Открыть файл"),
        "",
        tr("Json файлы (*.json);;Все файлы (*.*)")
        );

    if (!fileName.isEmpty()) {
        try {
            db->loadFromFile(fileName);
        } catch (std::runtime_error &e){
            qDebug() << e.what();
            ui->errorLabel->setText(e.what());
            QTimer::singleShot(4000, this, [&]() {
                ui->errorLabel->clear();
            });
        }
    }
}

//Сохранение db
void DatabaseMain::trySaveDB() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Сохранить файл"),
        "",
        tr("Json файлы (*.json);;Все файлы (*.*)")
        );

    if (!fileName.isEmpty()) {
        try {
            db->saveDateBase(fileName);
        } catch (std::runtime_error &e){
            qDebug() << e.what();
            ui->errorLabel->setText(e.what());
            QTimer::singleShot(4000, this, [&]() {
                ui->errorLabel->clear();
            });
        }
    }
}

// Удаление записи под выделением
void DatabaseMain::delUnderSelection() {
    try {
        if (selectedStudent >= 0) {
            if (selectedSkip < 0) {
                delStudent();
            } else {
                delSkipping();
            }
        }
    } catch (std::runtime_error &e){
        qDebug() << e.what();
        ui->errorLabel->setText(e.what());
        QTimer::singleShot(4000, this, [&]() {
            ui->errorLabel->clear();
        });
    }
}


// Удаление студента
void DatabaseMain::delStudent() {
    if (selectedStudent < 0) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение", "Вы уверены, что хотите удалить запись о ученике?",
                                  QMessageBox::Yes | QMessageBox::No);

    try {
        if (reply == QMessageBox::Yes) {
            db->removeStudent(selectedStudent);
        }
    } catch (std::runtime_error &e){
        qDebug() << e.what();
        ui->errorLabel->setText(e.what());
        QTimer::singleShot(4000, this, [&]() {
            ui->errorLabel->clear();
        });
    }
    clearCords();
}

// Удаление пропуска
void DatabaseMain::delSkipping() {
    // Проверяем индексы
    if (selectedSkip > -1 and selectedStudent > -1) {
        try {
            db->removeSkipping(selectedStudent, selectedSkip);
        } catch (std::runtime_error &e){
            qDebug() << e.what();
            ui->errorLabel->setText(e.what());
            QTimer::singleShot(4000, this, [&]() {
                ui->errorLabel->clear();
            });
        }
    }
    // Очищаем координаты
    clearCords();
}


// Очистка координат
void DatabaseMain::clearCords() {
    selectedSkip = -1;
    selectedStudent = -1;
    QString tmp = "Student ID: " + (selectedStudent < 0 ? "no" : QString::number(selectedStudent)) + "; Skipping: " + (selectedSkip < 0 ? "no" : QString::number(selectedSkip));
    ui->cordinatesLabel->setText(tmp);
    ui->add_item_button_2->setEnabled(false);
}

// Вскрывашке
void DatabaseMain::on_hideFiltersButton_clicked()
{
    ui->sortBarWidget->hide();
}

// Изменение параметра фильтрации
void DatabaseMain::on_skippsFromEdit_textChanged(const QString &arg1)
{
    update();
}

// Изменение параметра фильтрации
void DatabaseMain::on_skippsToEdit_textChanged(const QString &arg1)
{
    update();
}

// Изменение параметра фильтрации
void DatabaseMain::on_levelFromEdit_textChanged(const QString &arg1)
{
    update();
}

// Изменение параметра фильтрации
void DatabaseMain::on_levelToEdit_textChanged(const QString &arg1)
{
    update();
}

// Скрытие интерфейса
void DatabaseMain::on_hideAddDockButton_clicked()
{
    ui->addDockWidget->hide();
}

