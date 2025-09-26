#include <iostream>
#include "parse_tree.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <algorithm>
#include <fstream>

void replace_all(std::string &str, const std::string &from, const std::string &to)
{
    if (from.empty())
        return;
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos)
    {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
}

int main()
{

    // MAIN ENTRYPOINT INTO THE CODE, HERE THE PROGRAM READS THE INFIX NOTATION AND CONVERTS IT INTO A BINARY PARSE TREE

    // to print the binary tree, uncomment these lines and use ParseTree.print_tree()

    // SetConsoleOutputCP(CP_UTF8);
    // _setmode(_fileno(stdout), _O_U16TEXT);

    std::string exp;

    std::cout << "Enter Fully Paranthesized Infix Expression: ";
    getline(std::cin, exp);
    // exp = "(((((A * B) + (~C * D)) > ((E + ~B) * (~A + C))) * (~D + B)) > (A + (~E * C)))";

    ParseTree pt(exp, ParseTree::INFIX);

    // Q1 GETTING PREFIX EXPRESSION FROM INFIX EXPRESSION

    std::string pref_exp = pt.get_prefix();
    std::cout << pref_exp << std::endl;
    std::cout << infix_to_prefix(exp) << std::endl;

    // Q2 CREATING A ROOTED BINARY TREE FROM PREFIX EXPRESSION

    ParseTree ppt(pref_exp, ParseTree::PREFIX);

    // Q3 OUTPUTTING INFIX EXPRESSION FROM PREFIX PARSE TREE

    std::string inf_exp = pt.get_infix();
    std::cout << inf_exp << std::endl;

    // Q4 CALCULATING THE HEIGHT OF THE GIVEN PARSE TREE

    std::cout << "Height of infix tree is: " << pt.calculate_height() << std::endl;
    std::cout << "Height of prefix tree is: " << ppt.calculate_height() << std::endl;

    // Q5 CALCULATING THE TRUTH VALUE OF THE GIVEN PARSE TREE

    pt.set_atoms();
    std::cout << "For the given truth values, the function evaluates to: " << (pt.calculate() ? "True" : "False") << '\n';

    // Q6 CREATING CNF TREE FROM EXISTING TREE

    TreeNode::IMPL_FREE(pt.head);

    TreeNode::NNF(pt.head);

    TreeNode::CNF(pt.head);

    std::map<std::string, int> cnf_atoms;
    std::string cnf_exp = pt.head->get_cnf(cnf_atoms);

    if (cnf_exp.length() < 75)
        std::cout << cnf_exp << '\n';
    else
        std::cout << "The CNF form has been output into cnf_output.cnf\n";

    std::string cnf_file = cnf_exp;
    replace_all(cnf_file, " * ", " 0\n");
    replace_all(cnf_file, " + ", " ");
    replace_all(cnf_file, "~", "-");
    replace_all(cnf_file, "(", "");
    replace_all(cnf_file, ")", "");

    std::ofstream ofs("cnf_output.cnf");

    ofs << "c This is the CNF of the input you provided.\n";
    ofs << "p cnf " << cnf_atoms.size() << ' ' << std::count(cnf_exp.begin(), cnf_exp.end(), '*') + 1 << '\n';
    ofs << cnf_file;
    ofs << " 0";
    ofs.close();

    // Q7 CHECKING SATISFIABILITY OF CNF

    std::cout << "The given Expression is: " << (check_satisfiability("cnf_output.cnf") ? "Satisfiable" : "Not Satisfiable") << std::endl;
    std::wcout << L"Program Ended Successfully\n";

    SetConsoleOutputCP(CP_UTF8);
    _setmode(_fileno(stdout), _O_U16TEXT);
    pt.print_tree();
}