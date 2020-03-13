#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dict.h"

const std::string fname = "tc.mw";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    run = false;
    terminate = false;
    stop = false;
    updateAvg = false;
    buildDict = false;
    spellcheck = false;
    ui->setupUi(this);
    ui->textInput->setReadOnly(true);

    UpdateAvg();

    std::thread t1(Timer(this));
    std::thread t2(Record(this));
    std::thread t3(Spelling(this));
    t1.detach();
    t2.detach();
    t3.detach();
}

MainWindow::~MainWindow()
{
    terminate = true; // to kill threads // were used in multi deployment env
    delete ui;
}

void MainWindow::on_btnRun_clicked() {
    QPalette s;
    s.setColor(QPalette::Active, QPalette::Base, QColor(255,255,255));
    s.setColor(QPalette::Inactive, QPalette::Base, QColor(255,255,255));
    ui->textInput->clear();
    ui->btnRun->setEnabled(false);
    ui->btnAbort->setEnabled(true);
    ui->textInput->setPalette(s);
    ui->textInput->setReadOnly(false);
    ui->textInput->setFocus(Qt::FocusReason::MouseFocusReason);
    std::chrono::duration<double> t(ui->doubleLimit->value());
    ui->doubleLimit->setEnabled(false);

    limit = t;
    stop = false;
    run = true;

}

void MainWindow::Abort(){
    QPalette s;
    s.setColor(QPalette::Active, QPalette::Base, QColor(255,255,220));
    s.setColor(QPalette::Inactive, QPalette::Base, QColor(255,255,220));
    run = false;
    ui->textInput->setReadOnly(true);
    ui->textInput->setPalette(s);
    ui->btnRun->setEnabled(true);
    ui->btnAbort->setEnabled(false);
    ui->doubleLimit->setEnabled(true);
    ui->lcdSymbols->display(ui->textInput->toPlainText().size());
    updateAvg = true;
    spellcheck = true;
}

void MainWindow::on_btnAbort_clicked() {
    stop = true;
    Abort();
}

void MainWindow::UpdateAvg(){
    double val;
    QFile file(fname.c_str());
    file.open(QIODevice::ReadOnly);
    file.read(reinterpret_cast<char*>(&val), sizeof(double));

    ui->lcdAvg->display(val);
}

void MainWindow::on_btnReset_clicked()
{
    ui->lcdAvg->display(0.0);
    QFile file(fname.c_str());
    file.open(QIODevice::WriteOnly);
    file.close();
}

void Timer::operator()(){
    std::chrono::seconds s(1);
    while(!h->terminate){
        if (h->run){
            std::chrono::duration<double> current(static_cast<int>(h->ui->doubleLimit->value()));
            while(!h->terminate & !h->stop){
                current -= s;
                h->ui->lcdTimer->display(current.count());

                if(current <= std::chrono::seconds(0)) break;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            if (!h->stop) h->Abort();
        }
    }
}

double Record::findAvg(double a, double b, double f){
    double t;
    if(a == 0.0 || b == 0.0){
        a < b ? t = b : t = a;
        return t;
    }

    a /= f / 60.0;
    a < b ? t = b:t = a;
    a < b ? t -= a: t -= b;
    t /= 2;
    a < b ? t += a: t += b;
    return t;
}

void Record::operator()(){
    while(!h->terminate){
        if(h->updateAvg){
            double a = h->ui->lcdSymbols->value();
            double b = h->ui->lcdAvg->value();
            double c = h->ui->doubleLimit->value();
            h->ui->lcdAvg->display(findAvg(a, b, c));

            double res = h->ui->lcdAvg->value();
            QFile file(fname.c_str());
            file.open(QIODevice::WriteOnly);
            file.write(reinterpret_cast<char*>(&res), sizeof(double));
            file.close();
            h->updateAvg = false;
        }
    }
}


#include <array>
// to sort alphabet
//QStringList s;
//for(const auto& i:RusAlphabet){
//    s.push_back(i);
//}
//s.sort();

void MainWindow::on_textPreview_textChanged() { // make ascii to utf16 convertor
    buildDict = true;
}





void Spelling::operator()(){
    while(!h->terminate){
        if(h->buildDict){
            QString text = h->ui->textPreview->toPlainText();
            double time = dict().build(text, dict::type::ru);

            h->ui->lcdDict->display(time);
            h->buildDict = false;
        }
    if(h->spellcheck){ // spelling check
        QStringList err;
        QString text = h->ui->textInput->toPlainText();
        dict a;
        a.build(text, dict::type::current);
        err = a.check(dict::type::ru);

        h->ui->listErrors->addItems(err);
        h->spellcheck = false;
        }
    }

}
