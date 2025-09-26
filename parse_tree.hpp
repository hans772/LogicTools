#ifndef PARSE_TREE
#define PARSE_TREE

#include <string>
#include <queue>
#include <map>
#include <set>
#include <memory>

/*!
    This function is used to remove leading and following spaces from the given string
    It also a removes redundant pair of brackets from the expression.
*/
std::string remove_surrounding_spaces_and_braces(std::string exp);

/*!
    This function is used to convert an infix expression into a prefix expression directly without the use of parse trees.
*/
std::string infix_to_prefix(std::string exp);

/*!
    This function is used to check the satisfiability of a CNF file
    The satisfiability of a CNF expression is the ability to select a specific combination of truth values of the given variables
        such that the given expression evaluates to True
    This problem is considered NP-Complete, ie, all 2^n possibilities must be explored to determine the answer in the worst case.
    given that m is the number of variables and n is the number of clauses.

    Time Complexity : O(m * n * 2^m)
    Space Complexity: O(m * n)

    Output: boolean : True if the cnf expression is valid/satisfiable.
*/
bool check_satisfiability(std::string filepath);

//! A class which represents a node in a parse tree
/*!
    This class is used to create parse trees, it has the structure of a node in a binary tree.
*/
struct TreeNode
{

    enum NodeType
    {
        //! Negation node
        /*! Negation nodes have only right branches, the left branch is left null */
        NEGATION = '~',

        //! Implication node
        IMPLICATION = '>',

        //! Conjunction node
        CONJUNCTION = '*',

        //! Disjunction node
        DISJUNCTION = '+',

        //! Atom node
        /*! Atoms overload their own name over this type */
        ATOM = ' '
    };

    //! A static function to get the corresponding NodeType from a given character
    static NodeType get_node_type(char c);

    //! Right and Left node pointer;
    /*!
        The right and left pointers point to the right and left node of each node in the parse tree.
        They are unique pointers to handle memory management;
    */
    std::unique_ptr<TreeNode> right, left;

    //! Type of the node.
    /*!< Refers to the type of the node in the parse tree. */
    NodeType type;

    //! A constructor to create a Tree node of a specific type
    TreeNode(TreeNode::NodeType type);
    ~TreeNode();

    //! A recursive function used to clone a subtree
    /*!
        This function creates a completely new tree with the same properties as the current from the given node
    */
    virtual std::unique_ptr<TreeNode> clone() const;

    //! A recursive function used to print the tree.
    /*!
        This function requires
        ```SetConsoleOutputCP(CP_UTF8);```
        ```_setmode(_fileno(stdout), _O_U16TEXT);```
        to be run prior to printing the tree as it uses special UTF-8 characters.

        normal cout will not be functional if these are enabled
    */
    virtual void print_tree(const std::wstring &prefix, bool isLeft, bool isRoot);

    //! A recursive function to get the prefix representation of the node
    /*!
        This function yields the prefix expression in the form
        `operator` `left_subtree` `right_subtree`
        without the use of any bracketing.
    */
    virtual std::string get_prefix() const;

    //! A recursive function to get the infix representation of the node
    /*!
        This function yields the infix expression in the form
        (`left_subtree` `operator` `right_subtree`)
        without the use of any bracketing.
    */
    virtual std::string get_infix() const;

    //! A recursive function to get the height of the tree branching from the current node
    /*!
        This function yields the height using a DFS algorithm.
        \param height the recursive variable height which acts as a counter for the depth of the tree, must be 0 initially
    */
    int calculate_height(int height) const;

    //! A recursive function which calculates the truth value of the tree branching from the current node
    /*!
        This function takes a map which maps the variable name to the truth value of the specific variable,
        \param atoms the dictionary of atoms whose truth values will be retrieved to calculate the final output.
    */
    virtual bool calculate(std::map<std::string, bool> &atoms);

    //! A recursive function which returns the IMPL_FREE (free of implications) form of subtree of the given node.
    /*!
        This function removes implications by replacing A > B with !A + B

        this function transforms the given tree
        Time complexity: O(n)
        Space complexity: O(n)

        \param node the unique pointer to the node of the tree you want to transform
    */
    static void IMPL_FREE(std::unique_ptr<TreeNode> &node);

    //! A recursive function which converts the given tree into the NNF (negations moved inwards) form of subtree of the given node.
    /*!
        This function moves all negations inwards, in the end only atoms will have negations as parents.
        this function transforms the given tree

        \param node the unique pointer to the node of the tree you want to transform
    */
    static void NNF(std::unique_ptr<TreeNode> &node);

    //! A recursive function which returns the DISTR (disjunction distributed with conjunctions) form of subtree of the given node.
    /*!
        Distributes a disjunctions using conjunctions.

        \param l the unique pointer to the left node of the disjunction you want to transform
        \param r the unique pointer to the right node of the disjunction you want to transform
    */
    static std::unique_ptr<TreeNode> DISTR(std::unique_ptr<TreeNode> l, std::unique_ptr<TreeNode> r);

    //! A recursive function which returns the CNF of the subtree of the given node.
    /*!
        This function converts the given tree into CNF form, this can cause variable explosion, which would cause the height and width of the tree to become exponentially larger
        leading to large time and space complexities.

        this function transforms the given tree.

        Time complexity: exponential
        Space complexity: exponential

        \param node the unique pointer to the node of the tree you want to transform
    */
    static void CNF(std::unique_ptr<TreeNode> &);

    //! A function which takes a tree which has been converted into CNF and returns the consolidated cnf string
    /*!
        \param atoms the dictionary of atoms to integeres present in the cnf. Must be empty when running the function externally.
    */
    virtual std::string get_cnf(std::map<std::string, int> &atoms);

    //! A function which takes a tree which has been converted into CNF and returns the consolidated cnf string
    /*!
        This function is used to consolidate the atoms / clauses of the same operation, it is only called when both the parent and the child
        are performing the same operation (Conjunction or Disjunction)

        \param atoms the dictionary of atoms to integeres present in the cnf.
        \param vals the set of values to be grouped together into one continuous operation.

    */
    void get_cnf(std::set<std::string> &vals, std::map<std::string, int> &atoms);
};

struct Atom : TreeNode
{
    //! Name of the atom
    /*!
    name of the atom in the node.
    this name overrides the type name for ATOM anywhere.
    */
    std::string name;

    //! Creates an atom node
    /*!
        \param name the name of the atom to be created
    */
    Atom(std::string name);

    //! Prints the name of the atom
    void print_tree(const std::wstring &prefix, bool isLeft, bool isRoot) override;

    //! Returns the name of the atom
    std::string get_prefix() const override;

    //! Returns the name of the atom
    std::string get_infix() const override;

    //! Returns the corresponding truth value of the atom in the given map
    /*!
        \param atoms the map of atoms from which truth values will be retrieved to calculate
    */
    bool calculate(std::map<std::string, bool> &atoms) override;

    //! Returns the name of the atom
    /*!
        \param atoms the dictionary of atoms to integeres present in the cnf. This class will add keys to this map
    */
    std::string get_cnf(std::map<std::string, int> &atoms) override;

    //! Returns a unique pointer to an atom to preserve data types
    std::unique_ptr<TreeNode> clone() const;
};

class ParseTree
{

private:
    //! The map of all the atoms and its associated truth values.
    std::map<std::string, bool> atoms;

    //! This function creates a binary parse tree from a given infix expression
    /*!
        This function uses a priority order of + < * < >
        The function creates only the required amount of nodes.
        However to allow for arbitrary formatting and bracketing, extra optimizations are run,
        which cause the time complexity to increase.

        when n is the length of the expression:

        Time complexity (without optimizations): O(n^2)

        when n is the number of nodes:

        Space complexity: O(n)

        \param expression the infix expression to be parsed

    */
    std::unique_ptr<TreeNode> create_tree_infix(std::string expression);

    //! This function creates a queue of tokens from a given prefix expression
    /*!
        This function converts a given prefix expression into a queue of expressions
        Each expression must be separated by spaces.

        Time complexity : O(n)
        Space complexity : O(n)

        \param expression the expression to be split.
    */
    std::queue<std::string> create_prefix_queue(std::string expression);

    //! This function creates a binary tree from the given prefix token queue
    /*!
        This functions takes in a queue of expressions and converts it into a parse tree

        Time complexity : O(n)
        Space complexity : O(n)

        \param pref_queue the prefix queue of expressions, basically the expression tokenized with delimiter of space.
        \param atoms the dictionary of atoms.
    */
    std::unique_ptr<TreeNode> create_tree_prefix(std::queue<std::string> &pref_queue);

public:
    std::unique_ptr<TreeNode> head;

    enum NotationType
    {
        //! INFIX notation type
        INFIX,

        //! PREFIX (Polish notation type)
        PREFIX,

        //! POSTFIX (Reverse polish notation type (yet to be implemented))
        POSTFIX
    };

    ParseTree();

    //! The constructor for the parse tree using an expression
    /*!
        The constructor calls the respective method based on the type of notation passed into the constructor.

        \param expression The logical expression to be converted into a parse tree, must be in the specified notation
        \param n_type The notation type of the given logical expression.

    */
    ParseTree(std::string expression, ParseTree::NotationType n_type);

    //! This recursive function returns the infix notation expression of the parse tree
    /*!
        Stringstreams are used for string building
        Each node is only visited once and hence:
        Time Complexity : O(n)
        Space Complexity : O(n)
    */
    std::string get_infix() const;

    //! This recursive function returns the prefix notation expression of the parse tree
    /*!
        Stringstreams are used for string building
        Each node is only visited once and hence:
        Time Complexity : O(n)
        Space Complexity : O(n)
    */
    std::string get_prefix() const;

    //! This recursive function returns the height of the parse tree
    /*!
        DFS is employed to find the height
        Each node is only visited once and hence:
        Time Complexity : O(n)
        Space Complexity : O(n)
    */
    int calculate_height() const;

    //! This recursive function returns the output of the parse tree from the atoms set by the set_atoms() function.
    /*!
        Each node is only visited once and hence:
        Time Complexity : O(n)
        Space Complexity : O(n)
    */
    bool calculate();

    //! This function sets the truth values of the atoms employed in the parse tree
    /*!
        Time Complexity : O(n)
        Space Complexity : O(n)
    */
    void set_atoms();

    //! This recursive function prints the tree in a beautified manner
    /*!
        This function requires
        ```SetConsoleOutputCP(CP_UTF8);```
        ```_setmode(_fileno(stdout), _O_U16TEXT);```
        to be run prior to printing the tree as it uses special UTF-8 characters.

        normal cout will not be functional if these are enabled
    */
    void print_tree();
};

#endif