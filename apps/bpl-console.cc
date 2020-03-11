#include <parser/parser.h>
#include <db/db.h>
#include <query/query.h>
#include <iostream>
#include <istream>
#include <fstream>
using namespace bpl;

void print_help() {
  puts("usage: bpl [options]\n"
       "\t-h, --help\tprint this guide and exit\n"
       "\t-q, --quiet\tstart in silent mode (default verbose)\n");
  puts("bpl shell:\n"
       "\t\\q, \\quit\tclose the shell\n"
       "\t\\h, \\help\tprint this guide\n"
       "\t\\s, \\switch\ttoggle between db mode (|:) and query mode (?-)\n"
       "\t\\v, \\verbose\ttoggle between silent mode and verbose mode (default verbose)\n");

  puts("bpl shell, db mode:\n"
       "\tJust type the rule and facts you wish to insert. Order of insertion is used as order of matching.\n");
  puts("bpl shell, query mode:\n"
       "\tJust type the queries you wish to perform. Queries are allowed to define new function (whose scope will outlive the query), but not new predicates.\n");

  puts("bpl limitations and known issues:\n"
       "\t- Infix operators are not allowed\n"
       "\t- Integer arithmetic is not allowed\n"
       "\t- No syntactic sugar for lists\n"
       "\t- Placeholder _ isn't supported in any expression\n"
       "\t- Cut is not implemented (yet)\n"
       "\t- Malformed input is currently handled poorly and might cause bpl to crash\n"
       "\t- Line editing is not implemented (running rlwrap bpl is suggested instead)\n"
       "\t- Last call optimization is not implemented (yet)\n"
  );
}

void load_from_file(db &storage, std::string_view cmd) {
  while (cmd.front() != ' ')cmd.remove_prefix(1);
  cmd = parser::trim(cmd);
  std::ifstream infile(std::string(cmd).c_str());
  int imported = 0;
  for (std::string line; std::getline(infile, line);) {
    std::string_view cmd = parser::trim(line);
    if (!cmd.empty() && cmd.front() != '%') {
      storage.insert_clause(parser::parse_clause(cmd));
      ++imported;
    }
  }
}

int interactive_shell(bool verbose = true) {
  constexpr std::string_view dbmode_prompt = "|: ", querymode_prompt = "?- ";
  bool dbmode = true;
  db storage(verbose);

  for (std::string line; std::cout << (dbmode ? dbmode_prompt : querymode_prompt), std::getline(std::cin, line);) {
    std::string_view cmd = parser::trim(line);
    if (cmd.empty())continue;
    if (cmd.front() == '\\') {
      cmd.remove_prefix(1);
      switch (cmd.front()) {
        case 'q':return (0);
        case 's':dbmode ^= 1;
          break;
        case 'h':print_help();
          break;
        case 'v':storage.toggle_verbosity();
          break;
        case 'l':load_from_file(storage, cmd);
          break;
      }
    } else {
      if (dbmode) {
        storage.insert_clause(parser::parse_clause(cmd));
      } else {
        //querymode
        query::execute_query(storage, parser::parse_term_list(cmd));
      }
    }
  }
  return 1;
}

void print_version(bool hint) {
  puts("\e[3m"
       "Burrell's Prolog version 1.0.4-linux-noarch [2020/03/11] for x86_64\n"
       "Copyright (c) 2020 by Luca Cavalleri");
  if (hint) puts("hint: type \\help to get more info");
  puts("\e[0m");
}

bool search_arg(std::string_view arg, int argc, char *argv[]) {
  for (int i = 1; i < argc; ++i)if (std::string_view(argv[i]) == arg)return true;
  return false;
}

int main(int argc, char *argv[]) {
  if (search_arg("-h", argc, argv) || search_arg("--help", argc, argv)) {
    print_version(false);
    print_help();
    exit(0);
  }
  print_version(true);
  bool quiet = search_arg("-q", argc, argv) || search_arg("--quiet", argc, argv);
  return interactive_shell(!quiet);
}