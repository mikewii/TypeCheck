#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <thread>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    virtual void Abort();
    virtual void UpdateAvg();

    std::atomic_bool run;
    std::atomic_bool terminate;
    std::atomic_bool stop;
    std::atomic_bool updateAvg;
    std::atomic_bool buildDict;
    std::atomic_bool spellcheck;
    std::chrono::duration<double> limit;

private slots:
    void on_btnRun_clicked();
    void on_btnAbort_clicked();
    void on_btnReset_clicked();

    void on_textPreview_textChanged();

private:
    Ui::MainWindow *ui;
    friend struct Timer;
    friend struct Record;
    friend struct Spelling;
};

struct Timer{ // took me some time to figure this out
    Timer(MainWindow* handle){h = handle;}
    void operator()(); // this was the thing missing to run new timer for each new class
    MainWindow* h; // and accessing that class members
};

struct Record{
    Record(MainWindow* handle){h = handle;}
    void operator()();
    double findAvg(double a, double b, double f);
    MainWindow* h;
};

struct Spelling{
    Spelling(MainWindow* handle){h = handle;}
    void operator()();
    MainWindow* h;
};

#endif // MAINWINDOW_H
