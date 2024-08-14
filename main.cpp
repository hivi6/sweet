#include <map>
#include <vector>
#include <memory>
#include <fstream>
#include <iostream>
using namespace std;

// ==================================================
// Position
// ==================================================

static string EMPTY_STRING = "";

struct Position
{
    string fname;     // name of the file
    string &src;      // content of the file
    int idx, ln, col; // current index, line number and column number

    Position()
        : fname{"<stdin>"}, src{EMPTY_STRING}, idx{0}, ln{1}, col{1} {}
    Position(string filename, string &source)
        : fname{filename}, src{source}, idx{0}, ln{1}, col{1} {}
    Position(string filename, string &source, int index, int line, int column)
        : fname{filename}, src{source}, idx{index}, ln{line}, col{column} {}

    void operator=(const Position &pos)
    {
        fname = pos.fname;
        src = pos.src;
        idx = pos.idx;
        ln = pos.ln;
        col = pos.col;
    }

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
    out << pos.fname << ":" << pos.ln << ":" << pos.col;
    return out;
}

// ==================================================
// Token
// ==================================================

enum struct TokenType
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
    out << TokenTypeToString.at(type);
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

    Token() {}
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

enum struct ErrorType
{
    ILLEGAL_CHARACTER_ERROR,
    UNEXPECTED_TOKEN_ERROR,
    EOF_ERROR,
    NO_ERROR,
};

static const map<ErrorType, string> ERROR_NAME = {
    {ErrorType::ILLEGAL_CHARACTER_ERROR, "IllegalCharacterError"},
    {ErrorType::UNEXPECTED_TOKEN_ERROR, "UnexpectedTokenError"},
    {ErrorType::EOF_ERROR, "EndOfFileError"},
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

    void operator=(const Error &error)
    {
        typ = error.typ;
        deets = error.deets;
        startPos = error.startPos;
        endPos = error.endPos;
    }
};

ostream &operator<<(ostream &out, const Error &error)
{
    out << error.startPos << " " << ERROR_NAME.at(error.typ)
        << ": " << error.deets;
    return out;
}

// ==================================================
// Lexer
// ==================================================

struct LexerResult
{
    vector<Token> value;
    vector<Error> errors;
};

struct Lexer
{
    Lexer(string filename, string &source) : currentPos{filename, source} {}

    LexerResult tokenize()
    {
        while (currentPos.idx < currentPos.src.size())
        {
            getToken();
        }
        return result;
    }

private:
    LexerResult result;
    Position currentPos;

    void getToken()
    {
        auto previousPos = currentPos;
        // checking if whitespace
        if (currentPos.src[currentPos.idx] == ' ' ||
            currentPos.src[currentPos.idx] == '\n' ||
            currentPos.src[currentPos.idx] == '\t')
        {
            currentPos.advance();
        }
        // checking if literal
        else if (currentPos.idx < currentPos.src.size() &&
                 isdigit(currentPos.src[currentPos.idx]))
        {
            string lexical = "";
            while (currentPos.idx < currentPos.src.size() &&
                   isdigit(currentPos.src[currentPos.idx]))
            {
                lexical.push_back(currentPos.src[currentPos.idx]);
                currentPos.advance();
            }
            result.value.push_back(
                Token(TokenType::TT_LITERAL, lexical, previousPos, currentPos));
        }
        // checking if variable
        else if (currentPos.idx < currentPos.src.size() &&
                 (isalpha(currentPos.src[currentPos.idx]) ||
                  currentPos.src[currentPos.idx] == '_'))
        {
            string lexical = "";
            while (currentPos.idx < currentPos.src.size() &&
                   (isalpha(currentPos.src[currentPos.idx]) ||
                    isdigit(currentPos.src[currentPos.idx]) ||
                    currentPos.src[currentPos.idx] == '_'))
            {

                lexical.push_back(currentPos.src[currentPos.idx]);
                currentPos.advance();
            }
            // check if the variable is a keyword
            if (KEYWORDS.count(lexical))
            {
                result.value.push_back(Token(KEYWORDS.at(lexical), lexical,
                                             previousPos, currentPos));
            }
            else
            {
                result.value.push_back(Token(TokenType::TT_VARIABLE, lexical,
                                             previousPos, currentPos));
            }
        }
        // checking if symbol
        else if (
            currentPos.src[currentPos.idx] == '=' ||
            currentPos.src[currentPos.idx] == ';' ||
            currentPos.src[currentPos.idx] == '(' ||
            currentPos.src[currentPos.idx] == ')' ||
            currentPos.src[currentPos.idx] == '<' ||
            currentPos.src[currentPos.idx] == '>' ||
            currentPos.src[currentPos.idx] == '+' ||
            currentPos.src[currentPos.idx] == '-' ||
            currentPos.src[currentPos.idx] == '*' ||
            currentPos.src[currentPos.idx] == '/')
        {
            string lexical = string(1, currentPos.src[currentPos.idx]);
            currentPos.advance();
            // checking if < or > or = follows by =
            if ((lexical == "=" || lexical == "<" || lexical == ">") &&
                currentPos.src[currentPos.idx] == '=')
            {
                lexical.push_back(currentPos.src[currentPos.idx]);
                currentPos.advance();
            }
            result.value.push_back(
                Token(SYMBOLS.at(lexical), lexical, previousPos, currentPos));
        }
        else
        {
            string details = "unexpected character '" +
                             string(1, currentPos.src[currentPos.idx]) +
                             "' found.";
            currentPos.advance();
            result.errors.push_back(Error(ErrorType::ILLEGAL_CHARACTER_ERROR,
                                          details, previousPos, currentPos));
        }
    }
};

// ==================================================
// AST
// ==================================================

enum class AstType
{
    AST_PROGRAM,
    AST_STATEMENT,
    AST_ASSIGN,
    AST_LABEL,
    AST_GOTO,
    AST_IF,
    AST_PRINT,
    AST_EXPRESSION,
    AST_PRIMARY,
    AST_VARIABLE,
    AST_LITERAL
};

struct AstStatement;

struct AstLiteral
{
    Token tokenLiteral;
};

struct AstVariable
{
    Token tokenVariable;
};

struct AstPrimary
{
    AstType type;
    union
    {
        AstLiteral *astLiteral;
        AstVariable *astVariable;
    };

    ~AstPrimary()
    {
        switch (type)
        {
        case AstType::AST_LITERAL:
            delete astLiteral;
            break;
        case AstType::AST_VARIABLE:
            delete astVariable;
            break;
        default:
            cerr << "Error: This is very much unexpected. PANICING!!!" << endl;
            exit(1);
        }
    }
};

struct AstExpression
{
    AstPrimary *left;
    Token tokenOperator;
    AstPrimary *right;

    ~AstExpression()
    {
        delete left;
        delete right;
    }
};

struct AstPrint
{
    Token tokenPrint;
    AstExpression *astExpression;
    Token tokenSemiColon;

    ~AstPrint()
    {
        delete astExpression;
    }
};

struct AstIf
{
    Token tokenIf;
    Token tokenLParen;
    AstExpression *astExpression;
    Token tokenRParen;
    AstStatement *astStatement;

    ~AstIf()
    {
        delete astExpression;
        delete astStatement;
    }
};

struct AstGoto
{
    Token tokenGoto;
    AstVariable *astVariable;
    Token tokenSemiColon;

    ~AstGoto()
    {
        delete astVariable;
    }
};

struct AstLabel
{
    Token tokenLabel;
    AstVariable *astVariable;
    Token tokenSemiColon;

    ~AstLabel()
    {
        delete astVariable;
    }
};

struct AstAssign
{
    AstVariable *astVariable;
    Token tokenEqual;
    AstExpression *astExpression;
    Token tokenSemiColon;

    ~AstAssign()
    {
        delete astVariable;
        delete astExpression;
    }
};

struct AstStatement
{
    AstType type;
    union
    {
        AstPrint *astPrint;
        AstIf *astIf;
        AstGoto *astGoto;
        AstLabel *astLabel;
        AstAssign *astAssign;
    };

    ~AstStatement()
    {
        switch (type)
        {
        case AstType::AST_PRINT:
            delete astPrint;
            break;
        case AstType::AST_IF:
            delete astIf;
            break;
        case AstType::AST_GOTO:
            delete astGoto;
            break;
        case AstType::AST_LABEL:
            delete astLabel;
            break;
        case AstType::AST_ASSIGN:
            delete astAssign;
            break;
        default:
            cerr << "Error: This is very much unexpected. PANICING!!!" << endl;
            exit(1);
        }
    }
};

struct AstProgram
{
    vector<AstStatement *> statements;

    ~AstProgram()
    {
        for (auto statement : statements)
        {
            delete statement;
        }
    }
};

void printAstVariable(AstVariable *variable, int spaces = 0)
{
    string space(spaces * 2, ' ');
    cout << "AstVariable" << endl;
    cout << space << "| " << endl;
    cout << space << "+-" << variable->tokenVariable << endl;
}

void printAstGoto(AstGoto *astGoto, int spaces = 0)
{
    string space(spaces * 2, ' ');
    cout << "AstGoto" << endl;
    cout << space << "| " << endl;
    cout << space << "+-";
    printAstVariable(astGoto->astVariable, spaces + 1);
}

void printAstLabel(AstLabel *label, int spaces = 0)
{
    string space(spaces * 2, ' ');
    cout << "AstLabel" << endl;
    cout << space << "| " << endl;
    cout << space << "+-";
    printAstVariable(label->astVariable, spaces + 1);
}

void printAstStatement(AstStatement *statement, int spaces = 0)
{
    string space(spaces * 2, ' ');
    cout << "AstStatement" << endl;
    cout << space << "| " << endl;
    cout << space << "+-";
    switch (statement->type)
    {
    // case AstType::AST_PRINT:
    //     printAstPrint(statement->astPrint, spaces + 1);
    //     break;
    // case AstType::AST_IF:
    //     printAstIf(statement->astIf, spaces + 1);
    //     break;
    case AstType::AST_GOTO:
        printAstGoto(statement->astGoto, spaces + 1);
        break;
    // case AstType::AST_ASSIGN:
    //     printAstAssign(statement->astAssign, spaces + 1);
    //     break;
    case AstType::AST_LABEL:
        printAstLabel(statement->astLabel, spaces + 1);
        break;
    default:
        cerr << "Error: This is very much unexpected. PANICING!!!" << endl;
        exit(1);
    }
}

void printAstProgram(AstProgram *program, int spaces = 0)
{
    string space(spaces * 2, ' ');
    cout << "AstProgram" << endl;
    for (auto statement : program->statements)
    {
        cout << space << "| " << endl;
        cout << space << "+-";
        printAstStatement(statement, spaces + 1);
    }
}

// ==================================================
// Parser
// ==================================================

struct ParserResult
{
    shared_ptr<AstProgram> value = nullptr;
    vector<Error> errors;
};

struct Parser
{
    Parser(vector<Token> program) : cur{0}, tokens{program} {}

    ParserResult parse()
    {
        AstProgram *program = new AstProgram;
        while (cur < tokens.size())
        {
            auto statement = parseStatement();
            if (statement == nullptr)
            {
                delete program;
                program = nullptr;
                break;
            }
            program->statements.push_back(statement);
        }
        results.value = shared_ptr<AstProgram>(program);
        return results;
    }

private:
    int cur;
    vector<Token> tokens;
    ParserResult results;

    AstStatement *parseStatement()
    {
        if (tokens[cur].typ == TokenType::TT_VARIABLE)
        {
        }
        if (tokens[cur].typ == TokenType::TT_LABEL)
        {
            auto res = parseLabel();
            if (res == nullptr)
                return nullptr;
            auto ast = new AstStatement;
            ast->type = AstType::AST_LABEL;
            ast->astLabel = res;
            return ast;
        }
        if (tokens[cur].typ == TokenType::TT_GOTO)
        {
            auto res = parseGoto();
            if (res == nullptr)
                return nullptr;
            auto ast = new AstStatement;
            ast->type = AstType::AST_GOTO;
            ast->astGoto = res;
            return ast;
        }
        if (tokens[cur].typ == TokenType::TT_IF)
        {
        }
        if (tokens[cur].typ == TokenType::TT_PRINT)
        {
        }
        // something went wrong, unexprected token
        string details = "unexpected token '" + tokens[cur].lex + "'.";
        auto error = Error(ErrorType::UNEXPECTED_TOKEN_ERROR, details,
                           tokens[cur].startPos, tokens[cur].endPos);
        results.errors.push_back(error);
        return nullptr;
    }

    AstLabel *parseLabel()
    {
        auto labelToken = tokens[cur++];
        auto astVariable = parseVariable();
        if (astVariable == nullptr)
        {
            return nullptr;
        }
        if (cur >= tokens.size())
        {
            string details = "expected ';', instead reached eof.";
            auto error = Error(ErrorType::EOF_ERROR, details,
                               tokens.back().startPos, tokens.back().endPos);
            results.errors.push_back(error);
            return nullptr;
        }
        if (tokens[cur].typ != TokenType::TT_SEMI_COLON)
        {
            string details = "unexpected token '" + tokens[cur].lex +
                             "' found, was expecting a ';'.";
            auto error = Error(ErrorType::UNEXPECTED_TOKEN_ERROR, details,
                               tokens[cur].startPos, tokens[cur].endPos);
            results.errors.push_back(error);
            return nullptr;
        }
        auto semiColonToken = tokens[cur++];
        auto res = new AstLabel;
        res->tokenLabel = labelToken;
        res->astVariable = astVariable;
        res->tokenSemiColon = semiColonToken;
        return res;
    }

    AstGoto *parseGoto()
    {
        auto labelToken = tokens[cur++];
        auto astVariable = parseVariable();
        if (astVariable == nullptr)
        {
            return nullptr;
        }
        if (cur >= tokens.size())
        {
            string details = "expected ';', instead reached eof.";
            auto error = Error(ErrorType::EOF_ERROR, details,
                               tokens.back().startPos, tokens.back().endPos);
            results.errors.push_back(error);
            return nullptr;
        }
        if (tokens[cur].typ != TokenType::TT_SEMI_COLON)
        {
            string details = "unexpected token '" + tokens[cur].lex +
                             "' found, was expecting a ';'.";
            auto error = Error(ErrorType::UNEXPECTED_TOKEN_ERROR, details,
                               tokens[cur].startPos, tokens[cur].endPos);
            results.errors.push_back(error);
            return nullptr;
        }
        auto semiColonToken = tokens[cur++];
        auto res = new AstGoto;
        res->tokenGoto = labelToken;
        res->astVariable = astVariable;
        res->tokenSemiColon = semiColonToken;
        return res;
    }

    AstVariable *parseVariable()
    {
        if (cur >= tokens.size())
        {
            string details = "expected ';', instead reached eof.";
            auto error = Error(ErrorType::EOF_ERROR, details,
                               tokens.back().startPos, tokens.back().endPos);
            results.errors.push_back(error);
            return nullptr;
        }
        if (tokens[cur].typ != TokenType::TT_VARIABLE)
        {
            string details = "unexpected token '" + tokens[cur].lex +
                             "' found, was expecting a variable.";
            auto error = Error(ErrorType::UNEXPECTED_TOKEN_ERROR, details,
                               tokens[cur].startPos, tokens[cur].endPos);
            results.errors.push_back(error);
            return nullptr;
        }
        auto res = new AstVariable;
        res->tokenVariable = tokens[cur++];
        return res;
    }
};

int main(int argc, const char **argv)
{
    if (argc <= 1)
    {
        cerr << "Error: expected an input file." << endl;
        return 1;
    }
    string filename(argv[1]);

    ifstream fs(filename);
    if (!fs.good())
    {
        cerr << "Error: no such file '" << filename << "' exists." << endl;
        return 1;
    }

    string source, line;
    while (getline(fs, line))
    {
        source += line + "\n";
    }
    fs.close();

    cout << "content of " << filename << ":" << endl;
    cout << source << endl;

    Lexer lexer(filename, source);
    auto lexerResult = lexer.tokenize();
    if (lexerResult.errors.size())
    {
        for (auto error : lexerResult.errors)
        {
            cerr << error << endl;
        }
        return 1;
    }
    for (auto token : lexerResult.value)
    {
        cout << token << endl;
    }
    cout << endl;

    Parser parser(lexerResult.value);
    auto parserResult = parser.parse();
    if (parserResult.errors.size())
    {
        for (auto error : parserResult.errors)
        {
            cerr << error << endl;
        }
        return 1;
    }

    printAstProgram(parserResult.value.get());

    return 0;
}