#ifndef DICT_H
#define DICT_H
#include <vector>
#include <QString>
#include <QStringList>

class dict
{
public:
    enum type {
        en = 0,
        ru,
        current
    };
    dict();
    double build(QString& wordList, dict::type type = dict::type::ru);
    QStringList check(dict::type type);
private:
    long unsigned getVec(dict::type type, std::vector<QString>*& vec);
    QStringList format1(QString& t);
    void ASCIIToCyrillic(QStringList &s);
    void QSlistToVector(QStringList& lst, std::vector<QString>* vec);
};

#endif // DICT_H
