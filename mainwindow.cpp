#include "mainwindow.h"
#include "ui_mainwindow.h"

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
    spell = false;
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
    spell = true;
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
static std::array<char*, 33> RusAlphabet = {
    "а", "б", "в", "г", "д", "е", "ё", "ж", "з", "и",
    "й", "к", "л", "м", "н", "о", "п", "р", "с", "т",
    "у", "ф", "х", "ц", "ч", "ш", "щ", "ъ", "ы", "ь",
    "э", "ю", "я"
};


static std::array<char, 26> EngAlphabet = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'y', 'z'
};

static std::array<QChar, 16> ASCII = {'a', 'A', 'B', 'c', 'C', 'e', 'E', 'H', 'K', 'M', 'o', 'O', 'p', 'P', 'T', 'y'};
static std::array<char*, 16> UTF16 = {"а", "А", "В", "с", "С", "е", "Е", "Н", "К", "М", "о", "О", "р", "Р", "Т", "у"};

void ASCIItoUTF16(QStringList &s){ // if russian text only! make it understand if there is other language present
    for (auto i = 0; i < s.size(); i++)
        for(auto p = 0; p < ASCII.size(); p++){
            int pos = 0;
            while (pos != -1){
                pos = s[i].lastIndexOf(ASCII[p]); // found position of ascii
                if(pos >= 0) s[i].replace(pos, 1, UTF16[p]);
            }
        }
}

void findWordID(QString& s, std::vector<QString> *wbuf){
    QString ch = s.toLower();
    ch.resize(1);
    for(auto i = 0; i < RusAlphabet.size(); i++)
        if(ch.compare(RusAlphabet[i]) == 0 && (s.size() > 1)) wbuf[i].push_back(s);
}


void MainWindow::on_textPreview_textChanged() { // make ascii to utf16 convertor
    buildDict = true;
}

QStringList textOperation(QString& t){
    t.replace('\n', " ");
    t.remove(".");
    t.remove(",");
    t.remove("?");
    return t.split(" ");

}

void QSlistToVector(QStringList& lst, std::vector<QString>* vec){
    for(auto i = 0; i < lst.size(); i++)
        findWordID(lst[i], vec);
}

void formatWord(QString& s, QTextDocument* doc){
    QTextCursor newCursor(doc);
          while (!newCursor.isNull() && !newCursor.atEnd()) {
              newCursor = doc->find(s, newCursor);

              if (!newCursor.isNull()) {
                  newCursor.movePosition(QTextCursor::WordRight,
                                         QTextCursor::KeepAnchor);
                  QTextCharFormat fmt;
                  fmt.setUnderlineStyle(QTextCharFormat::SingleUnderline);
                  newCursor.mergeCharFormat(fmt);
              }
          }
}

void Spelling::operator()(){
    while(!h->terminate)
        if(h->buildDict){
            auto start = std::chrono::system_clock::now();
            h->RusWords->clear();
            h->EngWords->clear();
            QString text = h->ui->textPreview->toPlainText();
            QStringList textList = textOperation(text);
            ASCIItoUTF16(textList);
            textList.sort();
            QSlistToVector(textList, h->RusWords);

            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> result = end - start;
            h->ui->lcdDict->display(result.count());
            h->buildDict = false;
        }
    else if(h->spell){ // spelling check
        for(auto i = 0; i < 33; i++)
            h->Current[i].clear();
        QStringList err;
        QTextDocument *asDoc = h->ui->textInput->document();
        QString text = h->ui->textInput->toPlainText();
        QStringList textList = textOperation(text);
        ASCIItoUTF16(textList);
        textList.sort();
        QSlistToVector(textList, h->Current);

        // spelling check
        for( auto i = 0; i < 33; i++)
            if(!h->Current[i].empty()) // if we even have words starting at i
                for(int p = 0; p < h->Current[i].size(); p++)
                    for(int j = 0; j < h->RusWords[i].size(); j++){
                        auto res = h->Current[i][p].compare(h->RusWords[i][j]);
                        if(res != 0) err.append(h->Current[i][p]); // mark as wrong spelled
                    }
        h->ui->listErrors->addItems(err);
        h->spell = false;
        }

}
