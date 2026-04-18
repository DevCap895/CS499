/*Devin Capdevila
SNHU
CS300
15AUG2025
Project 2 
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

using namespace std;
// This program implements a course planner that allows users to load course data from a file,
// print a list of courses, and search for specific courses by their course number.
struct Course {
    string courseNumber;
    string courseTitle;
    vector<string> prerequisites;
};
// Node structure for the binary search tree
struct Node {
    Course course;
    Node* left;
    Node* right;
    Node(Course c) : course(c), left(nullptr), right(nullptr) {}
};
// Binary Search Tree class for managing courses
class CourseBST {
private:
    Node* root;

    void addNode(Node*& node, Course course) {
        if (!node) {
            node = new Node(course);
        }
        else if (course.courseNumber < node->course.courseNumber) {
            addNode(node->left, course);
        }
        else {
            addNode(node->right, course);
        }
    }
// In-order traversal to print courses in sorted order
    void inOrder(Node* node) {
        if (node) {
            inOrder(node->left);
            cout << node->course.courseNumber << ", " << node->course.courseTitle << endl;
            inOrder(node->right);
        }
    }
// Search for a course by its course number
    Course* findCourse(Node* node, const string& courseNumber) {
        if (!node) return nullptr;
        if (node->course.courseNumber == courseNumber) return &(node->course);
        else if (courseNumber < node->course.courseNumber) return findCourse(node->left, courseNumber);
        else return findCourse(node->right, courseNumber);
    }

    void deleteTree(Node* node) {
        if (node) {
            deleteTree(node->left);
            deleteTree(node->right);
            delete node;
        }
    }
// Destructor to clean up the tree
public:
    CourseBST() : root(nullptr) {}
    ~CourseBST() { deleteTree(root); }

    void insert(Course course) { addNode(root, course); }
    void printInOrder() { inOrder(root); }
    Course* search(const string& courseNumber) { return findCourse(root, courseNumber); }
};

string toUpper(string str) {
    transform(str.begin(), str.end(), str.begin(),
              [](unsigned char c) { return toupper(c); });
    return str;
}
// Function to load course data from a file into the binary search tree
void loadDataStructure(const string& filename, CourseBST& bst) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Cannot open file '" << filename << "'." << endl;
        return;
    }
// Read each line from the file, parse it, and insert into the BST
    string line;
    while (getline(file, line)) {
        replace(line.begin(), line.end(), '\t', ','); // handle tabs
        stringstream ss(line);
        string token;
        vector<string> tokens;

        while (getline(ss, token, ',')) {
            token.erase(0, token.find_first_not_of(" \t\r\n"));
            token.erase(token.find_last_not_of(" \t\r\n") + 1);
            tokens.push_back(token);
        }

        if (tokens.size() >= 2) {
            Course c;
            c.courseNumber = tokens[0];
            c.courseTitle = tokens[1];
            for (size_t i = 2; i < tokens.size(); ++i) {
                if (!tokens[i].empty()) c.prerequisites.push_back(tokens[i]);
            }
            bst.insert(c);
        }
    }

    file.close();
}
//Menu for the course planner application
//Allows users to load data, print course list, and search for specific courses
int main() {
    CourseBST bst;
    bool loaded = false;
    string filename;

    cout << "Welcome to the course planner." << endl;

    while (true) {
        cout << "1. Load Data Structure." << endl;
        cout << "2. Print Course List." << endl;
        cout << "3. Print Course." << endl;
        cout << "9. Exit" << endl;
        cout << "What would you like to do? ";

        string input;
        getline(cin, input);

        if (input == "1") {
            cout << "Enter filename: ";
            getline(cin, filename);
            loadDataStructure(filename, bst);
            loaded = true;
        }
        else if (input == "2") {
            if (!loaded) {
                cout << "Data structure is empty. Load the data first." << endl;
            } else {
                cout << "Here is a sample schedule:" << endl;
                bst.printInOrder();
            }
        }
        else if (input == "3") {
            if (!loaded) {
                cout << "Data structure is empty. Load the data first." << endl;
            } else {
                cout << "What course do you want to know about? ";
                string courseNum;
                getline(cin, courseNum);
                courseNum = toUpper(courseNum);
                Course* c = bst.search(courseNum);
                if (c) {
                    cout << c->courseNumber << ", " << c->courseTitle << endl;
                    cout << "Prerequisites: ";
                    if (c->prerequisites.empty()) {
                        cout << "None" << endl;
                    } else {
                        for (size_t i = 0; i < c->prerequisites.size(); ++i) {
                            cout << c->prerequisites[i];
                            if (i < c->prerequisites.size() - 1) cout << ", ";
                        }
                        cout << endl;
                    }
                } else {
                    cout << "Course not found." << endl;
                }
            }
        }
        else if (input == "9") {
            cout << "Thank you for using the course planner!" << endl;
            break;
        }
        else {
            cout << input << " is not a valid option." << endl;
        }
    }

    return 0;
}