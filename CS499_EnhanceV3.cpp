/*
 * Devin Capdevila
 * CS499 - Course Planner (Enhanced v3 - File Database Updates)
 * SNHU
 *
 */

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// ---------- Utility ----------

string toUpper(string str) {
    transform(str.begin(), str.end(), str.begin(),
              [](unsigned char c) { return static_cast<char>(toupper(c)); });
    return str;
}

string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

static bool contains(const vector<string>& v, const string& x) {
    return find(v.begin(), v.end(), x) != v.end();
}

// ---------- Data Model ----------

struct Course {
    string courseNumber;              // e.g., CS300
    string courseTitle;               // e.g., Data Structures
    vector<string> prerequisites;     // e.g., ["CS200"]
};

// ---------- BST Node (Smart Pointer) ----------

struct Node {
    Course course;
    unique_ptr<Node> left;
    unique_ptr<Node> right;

    explicit Node(const Course& c) : course(c) {}
};

// ---------- Course BST ----------

class CourseBST {
private:
    unique_ptr<Node> root;

    // Returns true if inserted, false if duplicate
    bool addNode(unique_ptr<Node>& node, const Course& course) {
        if (!node) {
            node = make_unique<Node>(course);
            return true;
        }

        if (course.courseNumber == node->course.courseNumber) {
            return false;
        } else if (course.courseNumber < node->course.courseNumber) {
            return addNode(node->left, course);
        } else {
            return addNode(node->right, course);
        }
    }

    Course* findCourse(Node* node, const string& courseNumber) const {
        if (!node) return nullptr;

        if (node->course.courseNumber == courseNumber) {
            return &(node->course);
        }
        if (courseNumber < node->course.courseNumber) {
            return findCourse(node->left.get(), courseNumber);
        }
        return findCourse(node->right.get(), courseNumber);
    }

    void inOrderPrint(const unique_ptr<Node>& node) const {
        if (!node) return;
        inOrderPrint(node->left);
        cout << node->course.courseNumber << " - " << node->course.courseTitle << "\n";
        inOrderPrint(node->right);
    }

    void inOrderCollect(const unique_ptr<Node>& node, vector<Course>& out) const {
        if (!node) return;
        inOrderCollect(node->left, out);
        out.push_back(node->course); // copy Course
        inOrderCollect(node->right, out);
    }

    int countNodes(const unique_ptr<Node>& node) const {
        if (!node) return 0;
        return 1 + countNodes(node->left) + countNodes(node->right);
    }

public:
    bool insert(const Course& course) {
        bool inserted = addNode(root, course);
        if (!inserted) {
            cout << "Duplicate skipped: " << course.courseNumber << "\n";
        }
        return inserted;
    }

    Course* search(const string& normalizedCourseNumber) const {
        // Caller must provide normalized input (trim + toUpper)
        return findCourse(root.get(), normalizedCourseNumber);
    }

    void printInOrder() const {
        if (!root) {
            cout << "No courses available.\n";
            return;
        }
        inOrderPrint(root);
    }

    int getCourseCount() const { return countNodes(root); }

    vector<Course> getAllCourses() const {
        vector<Course> courses;
        inOrderCollect(root, courses);
        return courses;
    }
};

// ---------- File Loader / Saver ----------

class FileLoader {
public:
    static bool loadCourses(const string& filename, CourseBST& bst) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "Error opening file: " << filename << "\n";
            return false;
        }

        string line;
        int lineNumber = 0;

        while (getline(file, line)) {
            lineNumber++;
            if (trim(line).empty()) continue;

            // Accept tabs or commas; normalize tabs to commas
            replace(line.begin(), line.end(), '\t', ',');

            stringstream ss(line);
            string token;
            vector<string> tokens;

            while (getline(ss, token, ',')) {
                tokens.push_back(trim(token));
            }

            if (tokens.size() < 2) {
                cout << "Invalid line " << lineNumber << " (needs at least 2 fields)\n";
                continue;
            }

            Course course;
            course.courseNumber = toUpper(tokens[0]);
            course.courseTitle = tokens[1];

            // prerequisites (normalized + unique within the record)
            for (size_t i = 2; i < tokens.size(); i++) {
                string prereq = toUpper(tokens[i]);
                if (!prereq.empty() && !contains(course.prerequisites, prereq)) {
                    course.prerequisites.push_back(prereq);
                }
            }

            bst.insert(course);
        }

        cout << "Data loaded successfully from: " << filename << "\n";
        return true;
    }

    static bool saveCourses(const string& filename, const CourseBST& bst) {
        ofstream out(filename, ios::trunc);
        if (!out.is_open()) {
            cout << "Error writing file: " << filename << "\n";
            return false;
        }

        // Write deterministic, normalized output (sorted by courseNumber via in-order)
        vector<Course> courses = bst.getAllCourses();
        for (const Course& c : courses) {
            out << c.courseNumber << "," << c.courseTitle;

            for (const string& p : c.prerequisites) {
                out << "," << p;
            }
            out << "\n";
        }

        cout << "Database saved successfully to: " << filename << "\n";
        return true;
    }
};

// ---------- UI Helpers (Update Operations) ----------

static Course promptNewCourse() {
    Course c;

    cout << "Enter course number (e.g., CS300): ";
    getline(cin, c.courseNumber);
    c.courseNumber = toUpper(trim(c.courseNumber));

    cout << "Enter course title: ";
    getline(cin, c.courseTitle);
    c.courseTitle = trim(c.courseTitle);

    cout << "Enter prerequisites separated by commas (or blank for none): ";
    string prereqLine;
    getline(cin, prereqLine);

    stringstream ss(prereqLine);
    string token;
    while (getline(ss, token, ',')) {
        string prereq = toUpper(trim(token));
        if (!prereq.empty() && !contains(c.prerequisites, prereq)) {
            c.prerequisites.push_back(prereq);
        }
    }

    return c;
}

static void printCourseDetails(const Course& course) {
    cout << course.courseNumber << " - " << course.courseTitle << "\n";
    cout << "Prerequisites: ";
    if (course.prerequisites.empty()) {
        cout << "None";
    } else {
        for (size_t i = 0; i < course.prerequisites.size(); i++) {
            cout << course.prerequisites[i];
            if (i + 1 < course.prerequisites.size()) cout << ", ";
        }
    }
    cout << "\n";
}

// ---------- Main Program ----------

int main() {
    CourseBST bst;
    bool loaded = false;
    string filename; // source "database" file

    cout << "Welcome to the course planner.\n";

    while (true) {
        cout << "\nMenu:\n";
        cout << "1. Load Database File\n";
        cout << "2. Print Course List\n";
        cout << "3. Print Course\n";
        cout << "4. Show Course Count\n";
        cout << "5. Add New Course (in memory)\n";
        cout << "6. Update Course Title (in memory)\n";
        cout << "7. Add Prerequisite to Course (in memory)\n";
        cout << "8. Save Database File\n";
        cout << "9. Exit\n";
        cout << "Enter choice: ";

        string input;
        getline(cin, input);
        input = trim(input);

        if (input == "1") {
            cout << "Enter filename: ";
            getline(cin, filename);
            filename = trim(filename);

            // Note: This loads into the existing BST. If you want "fresh load",
            // run the program again or extend this to rebuild the BST.
            loaded = FileLoader::loadCourses(filename, bst);
        }
        else if (input == "2") {
            if (!loaded) {
                cout << "Load data first.\n";
            } else {
                cout << "\nCourse List:\n";
                bst.printInOrder();
            }
        }
        else if (input == "3") {
            if (!loaded) {
                cout << "Load data first.\n";
            } else {
                cout << "Enter course number: ";
                string courseNum;
                getline(cin, courseNum);
                courseNum = toUpper(trim(courseNum));

                Course* course = bst.search(courseNum);
                if (course) {
                    printCourseDetails(*course);
                } else {
                    cout << "Course not found.\n";
                }
            }
        }
        else if (input == "4") {
            if (!loaded) {
                cout << "Load data first.\n";
            } else {
                cout << "Total Courses: " << bst.getCourseCount() << "\n";
            }
        }
        else if (input == "5") {
            if (!loaded) {
                cout << "Load data first.\n";
            } else {
                Course newCourse = promptNewCourse();
                if (newCourse.courseNumber.empty() || newCourse.courseTitle.empty()) {
                    cout << "Course number and title are required.\n";
                } else {
                    bst.insert(newCourse);
                }
            }
        }
        else if (input == "6") {
            if (!loaded) {
                cout << "Load data first.\n";
            } else {
                cout << "Enter course number to update: ";
                string courseNum;
                getline(cin, courseNum);
                courseNum = toUpper(trim(courseNum));

                Course* course = bst.search(courseNum);
                if (!course) {
                    cout << "Course not found.\n";
                } else {
                    cout << "Current: ";
                    printCourseDetails(*course);

                    cout << "Enter new title: ";
                    string newTitle;
                    getline(cin, newTitle);
                    newTitle = trim(newTitle);

                    if (newTitle.empty()) {
                        cout << "Title not changed (blank input).\n";
                    } else {
                        course->courseTitle = newTitle;
                        cout << "Title updated.\n";
                    }
                }
            }
        }
        else if (input == "7") {
            if (!loaded) {
                cout << "Load data first.\n";
            } else {
                cout << "Enter course number to add prereq to: ";
                string courseNum;
                getline(cin, courseNum);
                courseNum = toUpper(trim(courseNum));

                Course* course = bst.search(courseNum);
                if (!course) {
                    cout << "Course not found.\n";
                } else {
                    cout << "Enter prerequisite course number: ";
                    string prereq;
                    getline(cin, prereq);
                    prereq = toUpper(trim(prereq));

                    if (prereq.empty()) {
                        cout << "No prereq added (blank input).\n";
                    } else if (contains(course->prerequisites, prereq)) {
                        cout << "Prerequisite already exists.\n";
                    } else {
                        course->prerequisites.push_back(prereq);
                        cout << "Prerequisite added.\n";
                    }
                }
            }
        }
        else if (input == "8") {
            if (!loaded) {
                cout << "Load data first.\n";
            } else if (filename.empty()) {
                cout << "No filename set. Load a database file first.\n";
            } else {
                FileLoader::saveCourses(filename, bst);
            }
        }
        else if (input == "9") {
            cout << "Goodbye!\n";
            break;
        }
        else {
            cout << "Invalid option.\n";
        }
    }

    return 0;
}