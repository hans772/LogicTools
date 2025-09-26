#include "parse_tree.hpp"
#include <iostream>
#include <stack>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <memory>

std::string remove_surrounding_spaces_and_braces(std::string exp)
{
    if (!exp.length())
        return exp;

    //! Remove spaces

    while (!exp.empty() && isspace(exp.front()))
        exp.erase(exp.begin());
    while (!exp.empty() && isspace(exp.back()))
        exp.pop_back();

    //! If the expression doesnt start and end with the same bracket, then they aren't redundant.
    if ((exp[0] != '(' || exp.back() != ')') && (exp[0] != '{' || exp.back() != '}') && (exp[0] != '[' || exp.back() != ']'))
        return exp;

    //! checks if the parentheses stack is empty even after removing the first and last element.
    std::stack<char> par_stack;

    for (int i = 1; i < exp.length() - 1; i++)
    {
        for (char c : "({[")
        {
            if (exp[i] == c)
                par_stack.push(c);
        }
        for (char c : "]})")
        {
            if ((exp[i] == c))
            {
                if ((!par_stack.empty()) && (par_stack.top() == exp[i] - 1))
                    par_stack.pop();
                else
                    return exp;
            }
        }
    }

    if (par_stack.empty())
        return exp.substr(1, exp.length() - 2);
    else
        return exp;
}

std::string infix_to_prefix(std::string exp)
{
    // performs the same operations as creating a tree, but makes a string instead.

    std::string tempexp = remove_surrounding_spaces_and_braces(exp);
    while (tempexp.length() != exp.length())
    {
        exp = tempexp;
        tempexp = remove_surrounding_spaces_and_braces(exp);
    }

    std::stringstream ss;
    std::stack<char> par_stack;

    for (int i = 0; i < exp.length(); i++)
    {
        for (char c : "({[")
        {
            if (exp[i] == c)
                par_stack.push(c);
        }
        for (char c : "]})")
        {
            if (exp[i] == c)
                par_stack.pop();
        }

        for (char c : "+*>")
        {
            if ((exp[i] == c) && par_stack.empty())
            {
                //shifting the variables around

                ss << (char)c << ' ' << infix_to_prefix(exp.substr(0, i)) << ' ' << infix_to_prefix(exp.substr(i + 1, exp.length() - i - 1));
                return ss.str();
            }
        }
    }

    if (exp[0] == TreeNode::NEGATION)
    {
        ss << '~' << ' ' << infix_to_prefix(exp.substr(1, exp.length() - 1));
        return ss.str();
    }

    return exp;
}

bool check_satisfiability(std::string filepath)
{
    std::ifstream infile(filepath);
    if (!infile.is_open())
    {
        std::cerr << "Error: could not open file at" << filepath << std::endl;
        return 1;
    }

    std::string line;
    int numVars = 0, numClauses = 0;
    std::vector<std::vector<int>> clauses;

    while (std::getline(infile, line))
    {
        if (line.empty() || line[0] == 'c')
        {
            continue;
        }
        if (line[0] == 'p')
        {
            std::string tmp;
            std::stringstream ss(line);
            ss >> tmp >> tmp >> numVars >> numClauses; // p cnf vars clauses (we do not need p and cnf)
        }
        else
        {
            std::stringstream ss(line);
            int lit;
            std::vector<int> clause;
            while (ss >> lit)
            {
                if (lit == 0)
                    break; // end of clause
                clause.push_back(lit);
            }
            if (!clause.empty())
            {
                clauses.push_back(clause);
            }
        }
    }

    // after converting all clauses, we check all 2^n possibilities

    infile.close();

    unsigned long long total = 1 << numVars; // 2^n possible assignments (max 63 vars)

    for (int mask = 0; mask < total; mask++)
    {
        std::vector<bool> atoms(numVars);
        for (int i = 0; i < numVars; i++)
        {
            atoms[i] = (mask >> i) & 1;
        }
        bool arr_sat = true;
        for (const auto &clause : clauses)
        {
            bool sat = false;

            for (const auto &literal : clause)
            {
                int var = abs(literal) - 1;
                bool value = atoms[var];
                if (literal < 0)
                    value = !value;

                if (value)
                {
                    sat = true;
                    break;
                }
            }

            if (!sat)
            {
                arr_sat = false;
                break;
            }
        }

        if (arr_sat)
        {
            return true;
        }
    }

    return false;
}

TreeNode::TreeNode(TreeNode::NodeType type) : type(type) {}
TreeNode::~TreeNode() = default;

TreeNode::NodeType TreeNode::get_node_type(char c)
{
    if (c == NEGATION)
        return NEGATION;
    if (c == IMPLICATION)
        return IMPLICATION;
    if (c == DISJUNCTION)
        return DISJUNCTION;
    if (c == CONJUNCTION)
        return CONJUNCTION;
    else
        return ATOM;
}

Atom::Atom(std::string name) : TreeNode(TreeNode::ATOM), name(name) {};

ParseTree::ParseTree() {};

ParseTree::ParseTree(std::string expression, ParseTree::NotationType n_type)
{
    if (n_type == INFIX)
        head = create_tree_infix(expression);
    if (n_type == PREFIX)
    {
        std::queue<std::string> pref_queue = create_prefix_queue(expression);
        head = create_tree_prefix(pref_queue);
    }
}

std::unique_ptr<TreeNode> ParseTree::create_tree_infix(std::string expression)
{
    std::string tempexp = remove_surrounding_spaces_and_braces(expression);
    while (tempexp.length() != expression.length())
    {
        expression = tempexp;
        tempexp = remove_surrounding_spaces_and_braces(expression);
    }

    std::stack<char> par_stack;

    for (char c : ">*+")
    {
        for (int i = 0; i < expression.length(); i++)
        {
            for (char c1 : "({[")
            {
                if (expression[i] == c1)
                    par_stack.push(c1);
            }
            for (char c1 : "]})")
            {
                if (expression[i] == c1)
                    par_stack.pop();
            }

            if ((expression[i] == c) && par_stack.empty())
            {
                std::unique_ptr<TreeNode> nn = std::make_unique<TreeNode>(TreeNode::get_node_type(expression[i]));
                nn->left = create_tree_infix(expression.substr(0, i));
                nn->right = create_tree_infix(expression.substr(i + 1, expression.length() - i - 1));

                return nn;
            }
        }
    }

    if (expression[0] == TreeNode::NEGATION)
    {
        std::unique_ptr<TreeNode> nn = std::make_unique<TreeNode>(TreeNode::NEGATION);
        nn->right = create_tree_infix(expression.substr(1, expression.length() - 1));

        return nn;
    }

    atoms[expression] = false;
    return std::make_unique<Atom>(expression);
}

std::queue<std::string> ParseTree::create_prefix_queue(std::string expression)
{
    std::stringstream ss(expression);
    std::string token;

    std::queue<std::string> pref_queue;
    while (ss >> token)
    {
        pref_queue.push(token);
    }

    return pref_queue;
}

std::unique_ptr<TreeNode> ParseTree::create_tree_prefix(std::queue<std::string> &pref_queue)
{

    std::string token = pref_queue.front();
    pref_queue.pop();

    for (char c : ">*+")
    {
        if (token[0] == c)
        {
            std::unique_ptr<TreeNode> nn = std::make_unique<TreeNode>(TreeNode::get_node_type(c));

            nn->left = create_tree_prefix(pref_queue);
            nn->right = create_tree_prefix(pref_queue);

            return nn;
        }
    }

    if (token[0] == '~')
    {
        std::unique_ptr<TreeNode> nn = std::make_unique<TreeNode>(TreeNode::NEGATION);
        nn->right = create_tree_prefix(pref_queue);

        return nn;
    }

    atoms[token] = false;
    return std::make_unique<Atom>(token);
}

std::string ParseTree::get_prefix() const
{
    return head->get_prefix();
}

std::string TreeNode::get_prefix() const
{
    std::stringstream ss;
    ss << (char)type << ' ';
    if (left)
        ss << left->get_prefix() << ' ';
    ss << right->get_prefix();

    return ss.str();
}

std::string Atom::get_prefix() const
{
    return name;
}

std::string ParseTree::get_infix() const
{
    return head->get_infix();
}

std::string TreeNode::get_infix() const
{
    std::stringstream ss;
    ss << "(";
    if (left)
        ss << left->get_infix() << ' ';
    ss << (char)type << ' ';
    ss << right->get_infix();
    ss << ")";

    return ss.str();
}

std::string Atom::get_infix() const
{
    return name;
}

int ParseTree::calculate_height() const
{
    return head->calculate_height(0);
}

int TreeNode::calculate_height(int height) const
{
    if (left)
    {
        return std::max(left->calculate_height(height + 1), right->calculate_height(height + 1));
    }
    if (!right)
        return height;
    else
        return right->calculate_height(height + 1);
}

bool ParseTree::calculate() { return head->calculate(atoms); }

bool TreeNode::calculate(std::map<std::string, bool> &atoms)
{
    switch (type)
    {
    case CONJUNCTION:
        return left->calculate(atoms) && right->calculate(atoms);
    case DISJUNCTION:
        return left->calculate(atoms) || right->calculate(atoms);
    case IMPLICATION:
        return !(left->calculate(atoms)) || right->calculate(atoms);
    case NEGATION:
        return !(right->calculate(atoms));
    default:
        return false;
    }
}

bool Atom::calculate(std::map<std::string, bool> &atoms)
{
    return atoms.at(name);
}

void ParseTree::set_atoms()
{
    for (auto it : atoms)
    {
        char inp;
        std::cout << "Enter Truth Value for " << it.first << " (1/0 ; t/f ; T/F): ";
        std::cin >> inp;
        if ((inp == '0') || (inp == 'f') || (inp == 'F'))
            atoms[it.first] = false;
        else
            atoms[it.first] = true;
    }
}

std::unique_ptr<TreeNode> TreeNode::clone() const
{

    auto n = std::make_unique<TreeNode>(type);
    if (left)
        n->left = left->clone();
    if (right)
        n->right = right->clone();
    return n;
}

std::unique_ptr<TreeNode> Atom::clone() const
{
    auto n = std::make_unique<Atom>(name);
    return n;
}

void TreeNode::IMPL_FREE(std::unique_ptr<TreeNode> &node)
{
    switch (node->type)
    {
    case CONJUNCTION:
    case DISJUNCTION:
    {
        IMPL_FREE(node->left);
        IMPL_FREE(node->right);
        break;
    }
    case IMPLICATION:
    {
        node->type = DISJUNCTION;

        std::unique_ptr<TreeNode> ln = std::make_unique<TreeNode>(NEGATION);
        IMPL_FREE(node->left);
        ln->right = std::move(node->left);
        node->left = std::move(ln);
        IMPL_FREE(node->right);

        break;
    }
    case NEGATION:
    {
        IMPL_FREE(node->right);
        break;
    }
    }
    return;
}

std::unique_ptr<TreeNode> TreeNode::DISTR(std::unique_ptr<TreeNode> l, std::unique_ptr<TreeNode> r)
{
    if (l->type == CONJUNCTION)
    {
        std::unique_ptr<TreeNode> nn = std::make_unique<TreeNode>(CONJUNCTION);
        nn->left = DISTR(std::move(l->left), r->clone());
        nn->right = DISTR(std::move(l->right), std::move(r));
        return nn;
    }

    if (r->type == CONJUNCTION)
    {
        std::unique_ptr<TreeNode> nn = std::make_unique<TreeNode>(CONJUNCTION);
        nn->left = DISTR(l->clone(), std::move(r->left));
        nn->right = DISTR(std::move(l), std::move(r->right));
        return nn;
    }

    std::unique_ptr<TreeNode> nn = std::make_unique<TreeNode>(DISJUNCTION);
    nn->left = std::move(l);
    nn->right = std::move(r);
    return nn;
}

void TreeNode::CNF(std::unique_ptr<TreeNode> &node)
{

    switch (node->type)
    {
    case CONJUNCTION:
    {
        CNF(node->left);
        CNF(node->right);

        break;
    }
    case DISJUNCTION:
    {
        CNF(node->right);
        CNF(node->left);
        node = DISTR(std::move(node->left), std::move(node->right));

        break;
    }
    case NEGATION:
    {
        CNF(node->right);
        break;
    }
    }

    return;
}

void TreeNode::NNF(std::unique_ptr<TreeNode> &node)
{
    switch (node->type)
    {
    case CONJUNCTION:
    case DISJUNCTION:
    {
        NNF(node->left);
        NNF(node->right);

        break;
    }
    case NEGATION:
        switch (node->right->type)
        {
        case NEGATION:
        {
            node = std::move(node->right->right);
            NNF(node);
            break;
        }
        case CONJUNCTION:
        {
            node->type = DISJUNCTION;

            std::unique_ptr<TreeNode> lnn = std::make_unique<TreeNode>(NEGATION);
            std::unique_ptr<TreeNode> rnn = std::make_unique<TreeNode>(NEGATION);

            lnn->right = std::move(node->right->left);
            rnn->right = std::move(node->right->right);
            node->right.release();

            NNF(lnn);
            NNF(rnn);

            node->left = std::move(lnn);
            node->right = std::move(rnn);

            break;
        }
        case DISJUNCTION:
        {
            node->type = CONJUNCTION;

            std::unique_ptr<TreeNode> lnn = std::make_unique<TreeNode>(NEGATION);
            std::unique_ptr<TreeNode> rnn = std::make_unique<TreeNode>(NEGATION);

            lnn->right = std::move(node->right->left);
            rnn->right = std::move(node->right->right);
            node->right.release();

            NNF(lnn);
            NNF(rnn);

            node->left = std::move(lnn);
            node->right = std::move(rnn);

            break;
        }
        case ATOM:
        {
            return;
        }
        }
    }
    return;
}

std::string TreeNode::get_cnf(std::map<std::string, int> &atoms)
{

    if (type == NEGATION)
        return '~' + right->get_cnf(atoms);

    std::set<std::string> vals;
    if (left->type == type)
    {
        left->get_cnf(vals, atoms);
    }
    else
        vals.emplace(left->get_cnf(atoms));

    if (right->type == type)
    {
        right->get_cnf(vals, atoms);
    }
    else
        vals.emplace(right->get_cnf(atoms));

    std::stringstream ss;

    while (!vals.begin()->size())
        vals.erase(vals.begin());

    auto it = vals.begin();
    ss << '(';
    ss << *it;
    it++;

    while (it != vals.end())
    {
        std::string tok = *it;
        ss << ' ' << (char)type << ' ';
        ss << tok;
        it++;
    }
    ss << ')';

    return ss.str();
}

void TreeNode::get_cnf(std::set<std::string> &vals, std::map<std::string, int> &atoms)
{
    std::stringstream ss;

    if (left->type == type)
    {
        left->get_cnf(vals, atoms);
    }
    else
        vals.emplace(left->get_cnf(atoms));

    if (right->type == type)
    {
        right->get_cnf(vals, atoms);
    }
    else
        vals.emplace(right->get_cnf(atoms));
}

std::string Atom::get_cnf(std::map<std::string, int> &atoms)
{
    if (atoms.find(name) != atoms.end())
        return std::to_string(atoms[name]);
    return std::to_string(atoms[name] = (atoms.size() + 1));
}

void TreeNode::print_tree(const std::wstring &prefix, bool isLeft, bool isRoot)
{
    if (isRoot)
    {
        std::wcout << L"";
    }
    else
    {
        std::wcout << prefix << (isLeft ? L"├──" : L"└──");
    }
    std::wcout << ' ' << (char)type << std::endl;

    if (left)
        left->print_tree(!isRoot ? prefix + (isLeft ? L"│   " : L"    ") : L"", true, false);
    right->print_tree(!isRoot ? prefix + (isLeft ? L"│   " : L"    ") : L"", false, false);
}

void Atom::print_tree(const std::wstring &prefix, bool isLeft, bool isRoot)
{
    if (isRoot)
    {
        std::wcout << L"";
    }
    else
    {
        std::wcout << prefix << (isLeft ? L"├──" : L"└──");
    }
    std::wcout << ' ' << name.c_str() << std::endl;
}

void ParseTree::print_tree()
{
    head->print_tree(L"", false, true);
}