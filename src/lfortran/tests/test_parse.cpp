#include <tests/doctest.h>

#include <iostream>
#include <sstream>
#include <chrono>

#include <lfortran/parser/parser.h>
#include <lfortran/parser/parser.tab.hh>

using LFortran::parse;
using LFortran::AST::ast_t;
using LFortran::AST::expr_t;
using LFortran::AST::Name_t;
using LFortran::AST::BaseWalkVisitor;

// Print any vector like iterable to a string
template <class T>
inline std::ostream &print_vec(std::ostream &out, T &d)
{
    out << "[";
    for (auto p = d.begin(); p != d.end(); p++) {
        if (p != d.begin())
            out << ", ";
        out << *p;
    }
    out << "]";
    return out;
}


namespace doctest {
    // Convert std::vector<T> to string for doctest
    template<typename T> struct StringMaker<std::vector<T>> {
        static String convert(const std::vector<T> &value) {
            std::ostringstream oss;
            print_vec(oss, value);
            return oss.str().c_str();
        }
    };
}


class CountVisitor : public BaseWalkVisitor<CountVisitor>
{
    int c_;
public:
    CountVisitor() : c_{0} {}
    void visit_Name(const Name_t &x) { c_ += 1; }
    int get_count() {
        return c_;
    }
};

int count(const ast_t &b) {
    CountVisitor v;
    v.visit_ast(b);
    return v.get_count();
}


TEST_CASE("Test longer parser (N = 500)") {
    int N;
    N = 500;
    std::string text;
    std::string t0 = "(a*z+3+2*x + 3*y - x/(z**2-4) - x**(y**z))";
    text.reserve(22542);
    text = t0;
    std::cout << "Construct" << std::endl;
    for (int i = 0; i < N; i++) {
        text.append(" * " + t0);
    }
    Allocator al(1024*1024);
    std::cout << "Parse" << std::endl;
    auto t1 = std::chrono::high_resolution_clock::now();
    auto result = parse(al, text);
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)
                     .count()
              << "ms" << std::endl;
    int c = count(*result);
    std::cout << "Count: " << c << std::endl;
    std::cout << "String size (bytes):      " << text.size() << std::endl;
    std::cout << "Allocator usage (bytes): " << al.size_current() << std::endl;
    CHECK(c == 4509);
}

std::vector<int> tokens(const std::string &input)
{
    LFortran::Tokenizer t;
    t.set_string(input);
    std::vector<int> tst;
    int token = yytokentype::END_OF_FILE + 1; // Something different from EOF
    while (token != yytokentype::END_OF_FILE) {
        LFortran::YYSTYPE y;
        token = t.lex(y);
        tst.push_back(token);
    }
    return tst;
}


TEST_CASE("Tokenizer 1") {
    std::string s = R"(subroutine
    x = y
    x = 2*y
    subroutine)";
    std::vector<int> ref = {
        yytokentype::KW_SUBROUTINE,
        yytokentype::KW_NEWLINE,
        yytokentype::IDENTIFIER,
        '=',
        yytokentype::IDENTIFIER,
        yytokentype::KW_NEWLINE,
        yytokentype::IDENTIFIER,
        '=',
        yytokentype::NUMERIC,
        '*',
        yytokentype::IDENTIFIER,
        yytokentype::KW_NEWLINE,
        yytokentype::KW_SUBROUTINE,
        yytokentype::END_OF_FILE,
    };
    CHECK(tokens(s) == ref);
}

TEST_CASE("Tokenizer 2") {
    std::string s = "2*x**3";
    std::vector<int> ref = {
        yytokentype::NUMERIC,
        '*',
        yytokentype::IDENTIFIER,
        yytokentype::POW,
        yytokentype::NUMERIC,
        yytokentype::END_OF_FILE,
    };
    CHECK(tokens(s) == ref);
}

TEST_CASE("Tokenizer 3") {
    std::string s = "(2*x**3)";
    std::vector<int> ref = {
        '(',
        yytokentype::NUMERIC,
        '*',
        yytokentype::IDENTIFIER,
        yytokentype::POW,
        yytokentype::NUMERIC,
        ')',
        yytokentype::END_OF_FILE,
    };
    CHECK(tokens(s) == ref);
}
