/* 
 * Devin Capdevila
 * CS499 - Course Planner (Enhanced)
 * SNHU
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

// Data Model 
// Stores course info and prerequisites
struct Course {
    string courseNumber;          // e.g., CS300
    string courseTitle;           // course name
    vector<string> prerequisites; // prereq course IDs
};

// Convert string to uppercase for consistent comparisons
string toUpper(string str) {
    transform(str.begin(), str.end(), str.begin(),
              [](unsigned char c) { return toupper(c); });
    return str;
}

//  BST Node
struct Node {
    Course course;
    Node* left;
    Node* right;

    Node(Course c) : course(c), left(nullptr), right(nullptr) {}
};

// Binary Search Tree 
// Stores courses in sorted order by courseNumber
class CourseBST {
private:
    Node* root;

    // Insert node in correct BST position
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

    // In-order traversal (sorted output)
    void inOrder(Node* node) const {
        if (node) {
            inOrder(node->left);
            cout << node->course.courseNumber << ", "
                 << node->course.courseTitle << endl;
            inOrder(node->right);
        }
    }

    // Recursive search for a course
    Course* findCourse(Node* node, const string& courseNumber) const {
        if (!node) return nullptr;

        if (node->course.courseNumber == courseNumber)
            return &(node->course);

        if (courseNumber < node->course.courseNumber)
            return findCourse(node->left, courseNumber);

        return findCourse(node->right, courseNumber);
    }

    // Free all nodes (avoid memory leaks)
    void deleteTree(Node* node) {
        if (node) {
            deleteTree(node->left);
            deleteTree(node->right);
            delete node;
        }
    }

    // Count total nodes in tree
    int countNodes(Node* node) const {
        if (!node) return 0;
        return 1 + countNodes(node->left) + countNodes(node->right);
    }

public:
    CourseBST() : root(nullptr) {}
    ~CourseBST() { deleteTree(root); }

    // Insert course (skip duplicates)
    bool insert(Course course) {
        if (search(course.courseNumber) != nullptr) {
            cout << "Duplicate skipped: "
                 << course.courseNumber << endl;
            return false;
        }
        addNode(root, course);
        return true;
    }

    // Print all courses in order
    void printInOrder() const {
        if (!root) {
            cout << "No courses available." << endl;
            return;
        }
        inOrder(root);
    }

    // Search (case-insensitive)
    Course* search(const string& courseNumber) const {
        return findCourse(root, toUpper(courseNumber));
    }

    // Return total number of courses
    int getCourseCount() const {
        return countNodes(root);
    }
};

// File Loader 
// Reads file and loads courses into BST
class FileLoader {
public:
    static bool loadCourses(const string& filename, CourseBST& bst) {
        ifstream file(filename);

        if (!file.is_open()) {
            cout << "Error opening file: " << filename << endl;
            return false;
        }

        string line;
        int lineNumber = 0;

        while (getline(file, line)) {
            lineNumber++;

            if (line.empty()) continue;

            // Normalize delimiters
            replace(line.begin(), line.end(), '\t', ',');

            stringstream ss(line);
            string token;
            vector<string> tokens;

            // Split line into tokens
            while (getline(ss, token, ',')) {
                token.erase(0, token.find_first_not_of(" \t\r\n"));
                token.erase(token.find_last_not_of(" \t\r\n") + 1);
                tokens.push_back(token);
            }

            // Validate minimum fields
            if (tokens.size() < 2) {
                cout << "Invalid line " << lineNumber << endl;
                continue;
            }

            Course course;
            course.courseNumber = toUpper(tokens[0]);
            course.courseTitle = tokens[1];

            // Add prerequisites
            for (size_t i = 2; i < tokens.size(); i++) {
                if (!tokens[i].empty()) {
                    course.prerequisites.push_back(toUpper(tokens[i]));
                }
            }

            bst.insert(course);
        }

        file.close();
        cout << "Data loaded successfully." << endl;
        return true;
    }
};

// Main Program
// Menu-driven interface for user interaction
int main() {
    CourseBST bst;
    bool loaded = false;
    string filename;

    cout << "Welcome to the course planner." << endl;

    while (true) {
        cout << "\nMenu:\n";
        cout << "1. Load Data Structure\n";
        cout << "2. Print Course List\n";
        cout << "3. Print Course\n";
        cout << "4. Show Course Count\n";
        cout << "9. Exit\n";
        cout << "Enter choice: ";

        string input;
        getline(cin, input);

        if (input == "1") {
            cout << "Enter filename: ";
            getline(cin, filename);

            loaded = FileLoader::loadCourses(filename, bst);
        }

        else if (input == "2") {
            if (!loaded) {
                cout << "Load data first." << endl;
            } else {
                cout << "\nCourse List:\n";
                bst.printInOrder();
            }
        }

        else if (input == "3") {
            if (!loaded) {
                cout << "Load data first." << endl;
            } else {
                cout << "Enter course number: ";
                string courseNum;
                getline(cin, courseNum);

                Course* course = bst.search(courseNum);

                if (course) {
                    cout << course->courseNumber << ", "
                         << course->courseTitle << endl;

                    cout << "Prerequisites: ";
                    if (course->prerequisites.empty()) {
                        cout << "None";
                    } else {
                        for (size_t i = 0; i < course->prerequisites.size(); i++) {
                            cout << course->prerequisites[i];
                            if (i < course->prerequisites.size() - 1)
                                cout << ", ";
                        }
                    }
                    cout << endl;
                } else {
                    cout << "Course not found." << endl;
                }
            }
        }

        else if (input == "4") {
            if (!loaded) {
                cout << "Load data first." << endl;
            } else {
                cout << "Total Courses: "
                     << bst.getCourseCount() << endl;
            }
        }

        else if (input == "9") {
            cout << "Goodbye!" << endl;
            break;
        }

        else {
            cout << "Invalid option." << endl;
        }
    }

    return 0;
}