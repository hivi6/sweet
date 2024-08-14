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

static const map<string, TokenType> OPERATORS = {
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

void printAstVariable(AstVariable *variable, string prefix)
{
    cout << "AstVariable" << endl;
    cout << prefix << "| " << endl;
    cout << prefix << "+-" << variable->tokenVariable << endl;
}

void printAstLiteral(AstLiteral *literal, string prefix)
{
    cout << "AstLiteral" << endl;
    cout << prefix << "| " << endl;
    cout << prefix << "+-" << literal->tokenLiteral << endl;
}

void printAstPrimary(AstPrimary *primary, string prefix)
{
    cout << "AstPrimary" << endl;
    cout << prefix << "| " << endl;
    cout << prefix << "+-";
    switch (primary->type)
    {
    case AstType::AST_VARIABLE:
        printAstVariable(primary->astVariable, prefix + "  ");
        break;
    case AstType::AST_LITERAL:
        printAstLiteral(primary->astLiteral, prefix + "  ");
        break;
    default:
        cerr << "Error: This is very much unexpected. PANICING!!!" << endl;
        exit(1);
    }
}

void printAstExpression(AstExpression *expression, string prefix)
{
    cout << "AstExpression" << endl;
    cout << prefix << "| " << endl;
    cout << prefix << "+-";
    printAstPrimary(expression->left, prefix + "| ");
    cout << prefix << "| " << endl;
    cout << prefix << "+-" << expression->tokenOperator << endl;
    cout << prefix << "| " << endl;
    cout << prefix << "+-";
    printAstPrimary(expression->right, prefix + "  ");
}

void printAstGoto(AstGoto *astGoto, string prefix)
{
    cout << "AstGoto" << endl;
    cout << prefix << "| " << endl;
    cout << prefix << "+-" << astGoto->tokenGoto << endl;
    cout << prefix << "| " << endl;
    cout << prefix << "+-";
    printAstVariable(astGoto->astVariable, prefix + "| ");
    cout << prefix << "| " << endl;
    cout << prefix << "+-" << astGoto->tokenSemiColon << endl;
}

void printAstLabel(AstLabel *label, string prefix)
{
    cout << "AstLabel" << endl;
    cout << prefix << "| " << endl;
    cout << prefix << "+-" << label->tokenLabel << endl;
    cout << prefix << "| " << endl;
    cout << prefix << "+-";
    printAstVariable(label->astVariable, prefix + "| ");
    cout << prefix << "| " << endl;
    cout << prefix << "+-" << label->tokenSemiColon << endl;
}

void printAstPrint(AstPrint *print, string prefix)
{
    cout << "AstPrint" << endl;
    cout << prefix << "| " << endl;
    cout << prefix << "+-" << print->tokenPrint << endl;
    cout << prefix << "| " << endl;
    cout << prefix << "+-";
    printAstExpression(print->astExpression, prefix + "| ");
    cout << prefix << "| " << endl;
    cout << prefix << "+-" << print->tokenSemiColon << endl;
}

void printAstAssign(AstAssign *assign, string prefix)
{
    cout << "AstAssign" << endl;
    cout << prefix << "| " << endl;
    cout << prefix << "+-";
    printAstVariable(assign->astVariable, prefix + "| ");
    cout << prefix << "|" << endl;
    cout << prefix << "+-" << assign->tokenEqual << endl;
    cout << prefix << "|" << endl;
    cout << prefix << "+-";
    printAstExpression(assign->astExpression, prefix + "| ");
    cout << prefix << "|" << endl;
    cout << prefix << "+-" << assign->tokenSemiColon << endl;
}

void printAstStatement(AstStatement *statement, string prefix)
{
    cout << "AstStatement" << endl;
    cout << prefix << "| " << endl;
    cout << prefix << "+-";
    switch (statement->type)
    {
    case AstType::AST_PRINT:
        printAstPrint(statement->astPrint, prefix + "  ");
        break;
    // case AstType::AST_IF:
    //     printAstIf(statement->astIf, spaces + 1);
    //     break;
    case AstType::AST_GOTO:
        printAstGoto(statement->astGoto, prefix + "  ");
        break;
    case AstType::AST_ASSIGN:
        printAstAssign(statement->astAssign, prefix + "  ");
        break;
    case AstType::AST_LABEL:
        printAstLabel(statement->astLabel, prefix + "  ");
        break;
    default:
        cerr << "Error: This is very much unexpected. PANICING!!!" << endl;
        exit(1);
    }
}

void printAstProgram(AstProgram *program, string prefix = "")
{
    cout << "AstProgram" << endl;
    for (int i = 0; i < program->statements.size(); i++)
    {
        cout << prefix << "| " << endl;
        cout << prefix << "+-";
        auto tempPrefix = (i == program->statements.size() - 1 ? prefix + "  "
                                                               : prefix + "| ");
        printAstStatement(program->statements[i], tempPrefix);
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
            auto res = parseAssign();
            if (res == nullptr)
                return nullptr;
            auto ast = new AstStatement;
            ast->type = AstType::AST_ASSIGN;
            ast->astAssign = res;
            return ast;
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
            auto res = parsePrint();
            if (res == nullptr)
                return nullptr;
            auto ast = new AstStatement;
            ast->type = AstType::AST_PRINT;
            ast->astPrint = res;
            return ast;
        }
        // something went wrong, unexprected token
        return (AstStatement *)unexpectedError(tokens[cur]);
        return nullptr;
    }

    AstPrint *parsePrint()
    {
        auto printToken = tokens[cur++];
        auto astExpression = parseExpression();
        if (astExpression == nullptr)
            return nullptr;
        
        if (cur >= tokens.size())
            return (AstPrint *)eofError(tokens.back(), ";");
        if (tokens[cur].typ != TokenType::TT_SEMI_COLON)
            return (AstPrint *)unexpectedError(tokens[cur], ";");
        auto semiColonToken = tokens[cur++];

        auto res = new AstPrint;
        res->tokenPrint = printToken;
        res->astExpression = astExpression;
        res->tokenSemiColon = semiColonToken;
        return res;
    }

    AstLabel *parseLabel()
    {
        auto labelToken = tokens[cur++];
        auto astVariable = parseVariable();
        if (astVariable == nullptr)
            return nullptr;
        if (cur >= tokens.size())
            return (AstLabel *)eofError(tokens.back(), ";");
        if (tokens[cur].typ != TokenType::TT_SEMI_COLON)
            return (AstLabel *)unexpectedError(tokens[cur], ";");
        auto semiColonToken = tokens[cur++];
        auto res = new AstLabel;
        res->tokenLabel = labelToken;
        res->astVariable = astVariable;
        res->tokenSemiColon = semiColonToken;
        return res;
    }

    AstGoto *parseGoto()
    {
        auto gotoToken = tokens[cur++];
        auto astVariable = parseVariable();
        if (astVariable == nullptr)
            return nullptr;
        if (cur >= tokens.size())
            return (AstGoto *)eofError(tokens.back(), ";");
        if (tokens[cur].typ != TokenType::TT_SEMI_COLON)
            return (AstGoto *)unexpectedError(tokens[cur], ";");
        auto semiColonToken = tokens[cur++];
        auto res = new AstGoto;
        res->tokenGoto = gotoToken;
        res->astVariable = astVariable;
        res->tokenSemiColon = semiColonToken;
        return res;
    }

    AstVariable *parseVariable()
    {
        if (cur >= tokens.size())
            return (AstVariable *)eofError(tokens.back(), "variable");
        if (tokens[cur].typ != TokenType::TT_VARIABLE)
            return (AstVariable *)unexpectedError(tokens[cur], "variable");
        auto res = new AstVariable;
        res->tokenVariable = tokens[cur++];
        return res;
    }

    AstAssign *parseAssign()
    {
        auto astVariable = parseVariable();
        if (astVariable == nullptr)
            return nullptr;

        if (cur >= tokens.size())
            return (AstAssign *)eofError(tokens.back(), "=");
        if (tokens[cur].typ != TokenType::TT_EQUAL)
            return (AstAssign *)unexpectedError(tokens[cur], "=");
        auto equalToken = tokens[cur++];

        auto astExpression = parseExpression();
        if (astExpression == nullptr)
            return nullptr;

        if (cur >= tokens.size())
            return (AstAssign *)eofError(tokens.back(), ";");
        if (tokens[cur].typ != TokenType::TT_SEMI_COLON)
            return (AstAssign *)unexpectedError(tokens[cur], ";");
        auto semiColonToken = tokens[cur++];

        auto res = new AstAssign;
        res->astVariable = astVariable;
        res->tokenEqual = equalToken;
        res->astExpression = astExpression;
        res->tokenSemiColon = semiColonToken;
        return res;
    }

    AstExpression *parseExpression()
    {
        auto left = parsePrimary();
        if (left == nullptr)
            return nullptr;

        // check if the operator exists
        if (cur >= tokens.size())
            return (AstExpression *)eofError(tokens.back(), "operator");
        bool isOperator = false;
        for (auto op : OPERATORS)
        {
            if (op.second == tokens[cur].typ)
            {
                isOperator = true;
                break;
            }
        }
        if (!isOperator)
            return (AstExpression *)unexpectedError(tokens[cur], "operator");
        auto op = tokens[cur++];

        auto right = parsePrimary();
        if (left == nullptr)
            return nullptr;

        auto res = new AstExpression;
        res->left = left;
        res->right = right;
        res->tokenOperator = op;
        return res;
    }

    AstPrimary *parsePrimary()
    {
        if (cur >= tokens.size())
            return (AstPrimary *)eofError(tokens.back(), "primary");
        if (tokens[cur].typ == TokenType::TT_VARIABLE)
        {
            auto astVariable = parseVariable();
            if (astVariable == nullptr)
                return nullptr;
            auto res = new AstPrimary;
            res->type = AstType::AST_VARIABLE;
            res->astVariable = astVariable;
            return res;
        }
        if (tokens[cur].typ == TokenType::TT_LITERAL)
        {
            auto astLiteral = parseLiteral();
            if (astLiteral == nullptr)
                return nullptr;
            auto res = new AstPrimary;
            res->type = AstType::AST_VARIABLE;
            res->astLiteral = astLiteral;
            return res;
        }
    }

    AstLiteral *parseLiteral()
    {
        if (cur >= tokens.size())
            return (AstLiteral *)eofError(tokens.back(), "literal");
        if (tokens[cur].typ != TokenType::TT_LITERAL)
            return (AstLiteral *)unexpectedError(tokens[cur], "literal");
        auto res = new AstLiteral;
        res->tokenLiteral = tokens[cur++];
        return res;
    }

    // helper functions

    void *eofError(Token token, string expected)
    {
        string details = "expected '" + expected + "', instead reached eof.";
        auto error = Error(ErrorType::EOF_ERROR, details,
                           token.startPos, token.endPos);
        results.errors.push_back(error);
        return nullptr;
    }

    void *unexpectedError(Token token, string expected = "")
    {
        string details = "unexpected token '" + token.lex + "' found";
        if (expected != "")
            details += ", was expecting '" + expected + "'.";
        auto error = Error(ErrorType::UNEXPECTED_TOKEN_ERROR, details,
                           token.startPos, token.endPos);
        results.errors.push_back(error);
        return nullptr;
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