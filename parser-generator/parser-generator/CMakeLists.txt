# CMakeList.txt : CMake project for parser-generator, include source and define
# project specific logic here.
#

find_package(fmt CONFIG REQUIRED)

# Add source to this project's executable.
add_library (parser-generator "ValidateGrammar.ixx" "Grammar.ixx" "Parser.ixx" "Lexer.ixx"  "ParserGenerator.ixx" "Regex.ixx" "Automata.ixx" "ConvertRegexToNFA.ixx" )
target_link_libraries(parser-generator fmt::fmt)


# TODO: Add tests and install targets if needed.
