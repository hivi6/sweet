#include <map>
#include <iostream>
using namespace std;

// ==================================================
// Position
// ==================================================

static const string EMPTY_STRING = "";

struct Position
{
    string fname;      // name of the file
    const string &src; // content of the file
    int idx, ln, col;  // current index, line number and column number

    Position()
        : fname{"<stdin>"}, src{EMPTY_STRING}, idx{0}, ln{1}, col{1} {}
    Position(string filename, string &source)
        : fname{filename}, src{source}, idx{0}, ln{1}, col{1} {}
    Position(string filename, string &source, int index, int line, int column)
        : fname{filename}, src{source}, idx{index}, ln{line}, col{column} {}

    void advance()
    {
        if (idx >= src.size())
            return;

        if (src[idx] == '\n')
        {
            ln++;
            col = 0;
        }
        idx++;
        col++;
    }

    char getChar()
    {
        if (idx >= src.size())
            return 0;
        return src[idx];
    }

    void reset()
    {
        idx = 0;
        ln = 1;
        col = 1;
    }
};

ostream &operator<<(ostream &out, const Position &pos)
{
    out << pos.fname << ":" << pos.idx << ":" << pos.ln << ":" << pos.col;
    return out;
}

// ==================================================
// Token
// ==================================================

enum class TokenType
{
    TT_LITERAL,
    TT_VARIABLE,

    TT_PRINT,
    TT_GOTO,
    TT_IF,
    TT_LABEL,

    TT_EQUAL,
    TT_SEMI_COLON,
    TT_LPAREN,
    TT_RPAREN,

    TT_EQUAL_EQUAL,
    TT_LESS,
    TT_LESS_EQUAL,
    TT_GREATER,
    TT_GREATER_EQUAL,
    TT_PLUS,
    TT_MINUS,
    TT_MULTIPLY,
    TT_DIVIDE,
};

ostream &operator<<(ostream &out, TokenType type)
{
    static const map<TokenType, string> TokenTypeToString = {
        {TokenType::TT_LITERAL, "TT_LITERAL"},
        {TokenType::TT_VARIABLE, "TT_VARIABLE"},
        {TokenType::TT_PRINT, "TT_PRINT"},
        {TokenType::TT_GOTO, "TT_GOTO"},
        {TokenType::TT_IF, "TT_IF"},
        {TokenType::TT_LABEL, "TT_LABEL"},
        {TokenType::TT_EQUAL, "TT_EQUAL"},
        {TokenType::TT_SEMI_COLON, "TT_SEMI_COLON"},
        {TokenType::TT_LPAREN, "TT_LPAREN"},
        {TokenType::TT_RPAREN, "TT_RPAREN"},
        {TokenType::TT_EQUAL_EQUAL, "TT_EQUAL_EQUAL"},
        {TokenType::TT_LESS, "TT_LESS"},
        {TokenType::TT_LESS_EQUAL, "TT_LESS_EQUAL"},
        {TokenType::TT_GREATER, "TT_GREATER"},
        {TokenType::TT_GREATER_EQUAL, "TT_GREATER_EQUAL"},
        {TokenType::TT_PLUS, "TT_PLUS"},
        {TokenType::TT_MINUS, "TT_MINUS"},
        {TokenType::TT_MULTIPLY, "TT_MULTIPLY"},
        {TokenType::TT_DIVIDE, "TT_DIVIDE"},
    };
    cout << TokenTypeToString.at(type);
    return out;
};

static const map<string, TokenType> KEYWORDS = {
    {"print", TokenType::TT_PRINT},
    {"goto", TokenType::TT_GOTO},
    {"if", TokenType::TT_IF},
    {"label", TokenType::TT_LABEL},
};

static const map<string, TokenType> SYMBOLS = {
    {"=", TokenType::TT_EQUAL},
    {";", TokenType::TT_SEMI_COLON},
    {"(", TokenType::TT_LPAREN},
    {")", TokenType::TT_RPAREN},
    {"==", TokenType::TT_EQUAL_EQUAL},
    {"<", TokenType::TT_LESS},
    {"<=", TokenType::TT_LESS_EQUAL},
    {">", TokenType::TT_GREATER},
    {">=", TokenType::TT_GREATER_EQUAL},
    {"+", TokenType::TT_PLUS},
    {"-", TokenType::TT_MINUS},
    {"*", TokenType::TT_MULTIPLY},
    {"/", TokenType::TT_DIVIDE},
};

struct Token
{
    TokenType typ;
    string lex;
    Position startPos, endPos;

    Token(TokenType type, string lexical, Position start, Position end)
        : typ{type}, lex{lexical}, startPos{start}, endPos{end} {}
};

ostream &operator<<(ostream &out, const Token &token)
{
    out << token.typ << " '" << token.lex << "' "
        << token.startPos << " " << token.endPos;
    return out;
}

// ==================================================
// Error
// ==================================================

enum class ErrorType
{
    ILLEGAL_CHARACTER_ERROR,
    NO_ERROR,
};

static const map<ErrorType, string> ERROR_NAME = {
    {ErrorType::ILLEGAL_CHARACTER_ERROR, "IllegalCharacterError"},
    {ErrorType::NO_ERROR, "NoError"},
};

struct Error
{
    ErrorType typ;             // error type
    string deets;              // details regarding the error
    Position startPos, endPos; // start and end positions

    Error()
        : typ{ErrorType::NO_ERROR} {}
    Error(ErrorType type, string details, Position start, Position end)
        : typ{type}, deets{details}, startPos{start}, endPos{end} {}
};

ostream &operator<<(ostream& out, const Error& error)
{
    out << ERROR_NAME.at(error.typ) << ": " << error.deets;
    return out;
}

// ==================================================
// Lexer
// ==================================================

int main()
{
    Error e;
    cout << e << endl;
    return 0;
}