/* 
 * Devin Capdevila
 * CS499 - Course Planner (Enhanced v2)
 * Improvements Applied:
 * 1. Single traversal insert (no duplicate search)
 * 2. Smart pointers (memory safe)
 * 3. Input normalization consistency
 * 4. Notes on BST limitations
 * 5. Safer string parsing
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>

using namespace std;


// Utility

string toUpper(string str) {
    transform(str.begin(), str.end(), str.begin(),
              [](unsigned char c) { return toupper(c); });
    return str;
}

string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";

    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}


// Data Model

struct Course {
    string courseNumber;
    string courseTitle;
    vector<string> prerequisites;
};


// BST Node (Smart Pointer)
struct Node {
    Course course;
    unique_ptr<Node> left;
    unique_ptr<Node> right;

    Node(const Course& c) : course(c) {}
};


// Course BST

class CourseBST {
private:
    unique_ptr<Node> root;

    // Improved insert (handles duplicates internally)
    bool addNode(unique_ptr<Node>& node, const Course& course) {
        if (!node) {
            node = make_unique<Node>(course);
            return true;
        }

        if (course.courseNumber == node->course.courseNumber) {
            return false; // duplicate
        }
        else if (course.courseNumber < node->course.courseNumber) {
            return addNode(node->left, course);
        }
        else {
            return addNode(node->right, course);
        }
    }

    void inOrder(const unique_ptr<Node>& node) const {
        if (node) {
            inOrder(node->left);
            cout << node->course.courseNumber << " - "
                 << node->course.courseTitle << endl;
            inOrder(node->right);
        }
    }

    Course* findCourse(Node* node, const string& courseNumber) const {
        if (!node) return nullptr;

        if (node->course.courseNumber == courseNumber)
            return &(node->course);

        if (courseNumber < node->course.courseNumber)
            return findCourse(node->left.get(), courseNumber);

        return findCourse(node->right.get(), courseNumber);
    }

    int countNodes(const unique_ptr<Node>& node) const {
        if (!node) return 0;
        return 1 + countNodes(node->left) + countNodes(node->right);
    }

public:
    // Insert without double traversal
    bool insert(const Course& course) {
        bool inserted = addNode(root, course);
        if (!inserted) {
            cout << "Duplicate skipped: " << course.courseNumber << endl;
        }
        return inserted;
    }

    void printInOrder() const {
        if (!root) {
            cout << "No courses available." << endl;
            return;
        }
        inOrder(root);
    }

    // Search assumes input already normalized
    Course* search(const string& courseNumber) const {
        return findCourse(root.get(), courseNumber);
    }

    int getCourseCount() const {
        return countNodes(root);
    }
};


// File Loader
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

            replace(line.begin(), line.end(), '\t', ',');

            stringstream ss(line);
            string token;
            vector<string> tokens;

            while (getline(ss, token, ',')) {
                tokens.push_back(trim(token));
            }

            if (tokens.size() < 2) {
                cout << "Invalid line " << lineNumber << endl;
                continue;
            }

            Course course;
            course.courseNumber = toUpper(tokens[0]);
            course.courseTitle = tokens[1];

            for (size_t i = 2; i < tokens.size(); i++) {
                if (!tokens[i].empty()) {
                    course.prerequisites.push_back(toUpper(tokens[i]));
                }
            }

            bst.insert(course);
        }

        cout << "Data loaded successfully." << endl;
        return true;
    }
};

//main program
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
                courseNum = toUpper(trim(courseNum));

                Course* course = bst.search(courseNum);

                if (course) {
                    cout << course->courseNumber << " - "
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
