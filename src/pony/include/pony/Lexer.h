//===- Lexer.h - Lexer for the Pony language 
-------------------------------===//

#ifndef PONY_LEXER_H
#define PONY_LEXER_H

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

namespace pony {

/// Structure definition a location in a file.
struct Location {
  std::shared_ptr<std::string> file;  ///< filename.
  int line;                           ///< line number.
  int col;                            ///< column number.
};

// List of Token returned by the lexer.
enum Token : int {
  tok_semicolon = ';',
  tok_parenthese_open = '(',
  tok_parenthese_close = ')',
  tok_bracket_open = '{',
  tok_bracket_close = '}',
  tok_sbracket_open = '[',
  tok_sbracket_close = ']',

  tok_eof = -1,

  tok_return = -2,
  tok_var = -3,
  tok_def = -4,

  tok_identifier = -5,
  tok_number = -6,
};

/// The Lexer is an abstract base class providing all the facilities that the
/// Parser expects. It goes through the stream one token at a time and keeps
/// track of the location in the file for debugging purpose.
/// It relies on a subclass to provide a `readNextLine()` method. The subclass
/// can proceed by reading the next line from the standard input or from a
/// memory mapped file.
class Lexer {
 public:
  /// Create a lexer for the given filename. The filename is kept only for
  /// debugging purpose (attaching a location to a Token).
  Lexer(std::string filename)
      : lastLocation(
            {std::make_shared<std::string>(std::move(filename)), 0, 0}) {}
  virtual ~Lexer() = default;

  /// Look at the current token in the stream.
  Token getCurToken() { return curTok; }

  /// Move to the next token in the stream and return it.
  Token getNextToken() { return curTok = getTok(); }

  /// Move to the next token in the stream, asserting on the current token
  /// matching the expectation.
  void consume(Token tok) {
    assert(tok == curTok && "consume Token mismatch expectation");
    getNextToken();
  }

  /// Return the current identifier (prereq: getCurToken() == tok_identifier)
  llvm::StringRef getId() {
    assert(curTok == tok_identifier);
    return identifierStr;
  }

  double getValue() {
    assert(curTok == tok_number);
    return numVal;
  }

  /// Return the location for the beginning of the current token.
  Location getLastLocation() { return lastLocation; }

  // Return the current line in the file.
  int getLine() { return curLineNum; }

  // Return the current column in the file.
  int getCol() { return curCol; }

 private:
  /// Delegate to a derived class fetching the next line. Returns an empty
  /// string to signal end of file (EOF). Lines are expected to always finish
  /// with "\n"
  virtual llvm::StringRef readNextLine() = 0;

  // TODO: Implement function getNextChar().
  int getNextChar() {
    /*
     *
     *  Write your code here.
     *
     */
    // ======== Part 1 modified begin ========
    if (curLineBuffer.empty()) {
      curLineBuffer = readNextLine();

      if (curLineBuffer.empty())
        return EOF;

      ++curLineNum;
      curCol = 0;
    }

    char nextChar = curLineBuffer.front();
    curLineBuffer = curLineBuffer.drop_front();
    ++curCol;

    return nextChar;
    // ======== Part 1 modified end ========
  }

  ///  Return the next token from standard input.
  Token getTok() {
    // Skip any whitespace.
    while (lastChar != tok_eof &&
           isspace(static_cast<unsigned char>(lastChar)))
      lastChar = Token(getNextChar());

    // Save the current location before reading the token characters.
    lastLocation.line = curLineNum;
    lastLocation.col = curCol;

    // TODO: 补充成员函数getTok()。
    //       1. 能够识别“return”、“def”和“var”三个关键字，需要注意其各类大小写不规范书写形式；
    //       2. 能够识别标识符（函数名，变量名等）：
    //          • 标识符由字母、数字或下划线组成；
    //          • 标识符以字母或下划线开头；
    //          • 标识符中有数字时，数字不可连续出现
    //          例如：有效的标识符可以是 ab32c, _b, placeholder 等。
    //       3.
    //       在识别每种Token的同时，将其存放在某种数据结构中，以便最终在终端输出
    //
    // Hints: 1. 在实现第1，2点时，可参考getTok()函数中现有的识别数字的方法。
    //        2. 一些有用的函数:  isalpha(); isalnum();
    /*
     *
     *  Write your code here.
     *
     */
    // ======== Part 1 modified begin ========
    if ((lastChar != tok_eof &&
         isalpha(static_cast<unsigned char>(lastChar))) ||
        lastChar == '_') {
      identifierStr.clear();

      do {
        identifierStr += static_cast<char>(lastChar);
        lastChar = Token(getNextChar());
      } while ((lastChar != tok_eof &&
                isalnum(static_cast<unsigned char>(lastChar))) ||
               lastChar == '_');

      std::string lowerStr = identifierStr;
      for (char &c : lowerStr)
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));

      if (lowerStr == "return") return tok_return;
      if (lowerStr == "def") return tok_def;
      if (lowerStr == "var") return tok_var;

      return tok_identifier;
    }
    // ======== Part 1 modified end ========

    // TODO: 3.
    // 改进识别数字的方法，使编译器可以识别并在终端报告非法数字，非法表示包括：9.9.9，9..9，.999，..9，9..，9e01等。
    if ((lastChar != tok_eof &&
         isdigit(static_cast<unsigned char>(lastChar))) ||
        lastChar == '.') {
      // ======== Part 1 modified begin ========
      std::string numStr;
      bool valid = true;

      if (lastChar == '.') {
        valid = false;
        numStr += static_cast<char>(lastChar);
        lastChar = Token(getNextChar());
      } else {
        while (lastChar != tok_eof &&
               isdigit(static_cast<unsigned char>(lastChar))) {
          numStr += static_cast<char>(lastChar);
          lastChar = Token(getNextChar());
        }

        if (lastChar == '.') {
          numStr += '.';
          lastChar = Token(getNextChar());

          if (lastChar == tok_eof ||
              !isdigit(static_cast<unsigned char>(lastChar))) {
            valid = false;
          } else {
            while (lastChar != tok_eof &&
                   isdigit(static_cast<unsigned char>(lastChar))) {
              numStr += static_cast<char>(lastChar);
              lastChar = Token(getNextChar());
            }
          }
        }

        if (lastChar == '.') {
          valid = false;
          while (lastChar == '.' ||
                 (lastChar != tok_eof &&
                  isdigit(static_cast<unsigned char>(lastChar)))) {
            numStr += static_cast<char>(lastChar);
            lastChar = Token(getNextChar());
          }
        }
      }

      if (lastChar == 'e' || lastChar == 'E') {
        valid = false;
        do {
          numStr += static_cast<char>(lastChar);
          lastChar = Token(getNextChar());
        } while ((lastChar != tok_eof &&
                  isalnum(static_cast<unsigned char>(lastChar))) ||
                 lastChar == '.');
      }

      if (!valid) {
        llvm::errs() << "Lexical error: invalid number \"" << numStr
                     << "\" at line " << lastLocation.line
                     << ", col " << lastLocation.col << "\n";
        return tok_eof;
      }

      numVal = strtod(numStr.c_str(), nullptr);
      return tok_number;
      // ======== Part 1 modified end ========
    }

    if (lastChar == '#') {
      // Comment until end of line.
      do {
        lastChar = Token(getNextChar());
      } while (lastChar != EOF && lastChar != '\n' && lastChar != '\r');

      if (lastChar != EOF) return getTok();
    }

    // Check for end of file. Don't eat the EOF.
    if (lastChar == EOF) return tok_eof;

    // Otherwise, just return the character as its ascii value.
    Token thisChar = Token(lastChar);
    lastChar = Token(getNextChar());
    return thisChar;
  }

  /// The last token read from the input.
  Token curTok = tok_eof;

  /// Location for `curTok`.
  Location lastLocation;

  /// If the current Token is an identifier, this string contains the value.
  std::string identifierStr;

  /// If the current Token is a number, this contains the value.
  double numVal = 0;

  /// The last value returned by getNextChar(). We need to keep it around as we
  /// always need to read ahead one character to decide when to end a token and
  /// we can't put it back in the stream after reading from it.
  Token lastChar = Token(' ');

  /// Keep track of the current line number in the input stream
  int curLineNum = 0;

  /// Keep track of the current column number in the input stream
  int curCol = 0;

  /// Buffer supplied by the derived class on calls to `readNextLine()`
  llvm::StringRef curLineBuffer = "\n";
};

/// A lexer implementation operating on a buffer in memory.
class LexerBuffer final : public Lexer {
 public:
  LexerBuffer(const char *begin, const char *end, std::string filename)
      : Lexer(std::move(filename)), current(begin), end(end) {}

 private:
  /// Provide one line at a time to the Lexer, return an empty string when
  /// reaching the end of the buffer.
  llvm::StringRef readNextLine() override {
    auto *begin = current;
    while (current <= end && *current && *current != '\n') ++current;
    if (current <= end && *current) ++current;
    llvm::StringRef result{begin, static_cast<size_t>(current - begin)};
    return result;
  }
  const char *current, *end;
};
}  // namespace pony

#endif  // PONY_LEXER_H