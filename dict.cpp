#include "dict.h"
#include <array>
#include <chrono>

static std::array<std::vector<QString>, 33> RusWords;
static std::array<std::vector<QString>, 26> EngWords;
static std::array<std::vector<QString>, 33> Current;

dict::dict()
{

}

static std::array<const char*, 33> RusAlphabet = {
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
static std::array<const char*, 16> Cyrillic = {"а", "А", "В", "с", "С", "е", "Е", "Н", "К", "М", "о", "О", "р", "Р", "Т", "у"};

void dict::ASCIIToCyrillic(QStringList &s){ // if russian text only! make it understand if there is other language present
    for (auto i = 0; i < s.size(); i++)
        for(auto p = 0lu; p < ASCII.size(); p++){
            int pos = 0;
            while (pos != -1){
                pos = s[i].lastIndexOf(ASCII[p]); // found position of ascii
                if(pos >= 0) s[i].replace(pos, 1, Cyrillic[p]);
            }
        }
}

void findWordID(QString& s, std::vector<QString> *wbuf){
    QString ch = s.toLower();
    ch.resize(1);
    for(auto i = 0lu; i < RusAlphabet.size(); i++)
        if(ch.compare(RusAlphabet[i]) == 0 && (s.size() > 1)) wbuf[i].push_back(s);
}

void dict::QSlistToVector(QStringList& lst, std::vector<QString>* vec){
    for(auto i = 0; i < lst.size(); i++)
        findWordID(lst[i], vec);
}

long unsigned dict::getVec(dict::type type, std::vector<QString>*& vec){
    switch(type){
    case type::en:{
        for (auto& i : EngWords)
                i.clear();
        vec = EngWords.data();
        return EngWords.size();
    }
    case type::ru:{
        for (auto& i : RusWords)
          i.clear();
        vec = RusWords.data();
        return RusWords.size();
    }
    case type::current:{
        for (auto& i : Current)
          i.clear();
        vec = Current.data();
        return Current.size();
    }
    }
    vec = nullptr;
    return 0;
}

double dict::build(QString& wordList, dict::type type) {
    auto start_t = std::chrono::system_clock::now();
    QStringList lst = format1(wordList);
    ASCIIToCyrillic(lst);
    lst.sort();

    std::vector<QString> *vec = nullptr;
    getVec(type, vec);

    QSlistToVector(lst, vec);


    auto end_t = std::chrono::system_clock::now();
    auto result_t = end_t - start_t;
    return result_t.count();
}

QStringList dict::format1(QString& t){
    t.replace('\n', " ");
    t.remove(".");
    t.remove(",");
    t.remove("?");
    return t.split(" ");
}

QStringList dict::check(dict::type type){
    QStringList err;
    std::vector<QString>* vec = nullptr;
    getVec(type, vec); // get vector to check against

    for( auto i = 0lu; i < Current.size(); i++)
        if(!Current.data()[i].empty()) // if we even have words starting at i
            for(auto p = 0lu; p < Current.data()[i].size(); p++) // words count
                for(auto j = 0lu; j < vec[i].size(); j++){ // words in dict count
                    auto res = Current.data()[i][p].compare(vec[i][j]);
                    if(res != 0) err.append(Current.data()[i][p]); // mark as wrong spelled
                }
    return err;
}
