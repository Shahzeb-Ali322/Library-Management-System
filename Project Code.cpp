#include<iostream>
#include<fstream>
#include<sstream>
#include<vector>
#include<string>
#include<algorithm>
#include<exception>
#include<limits>
#ifdef _WIN32
#include<windows.h>
#include<conio.h>
#endif
#ifndef _WIN32
#include<sys/ioctl.h>
#include<unistd.h>
#endif
using namespace std;
// ANSI Color Constants
const string RESET   = "\033[0m";
const string BOLD    = "\033[1m";
const string RED     = "\033[31m";
const string GREEN   = "\033[32m";
const string YELLOW  = "\033[33m";
const string BLUE    = "\033[34m";
const string MAGENTA = "\033[35m";
const string CYAN    = "\033[36m";
const string WHITE   = "\033[37m";
const int BOX_WIDTH = 44;
// Console Width Detection
int getConsoleWidth() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
        return (int)w.ws_col;
#endif
    return 80;
}
// Centering Helpers
string boxPad() {
    int margin = (getConsoleWidth() - BOX_WIDTH - 2) / 2;
    if (margin < 0) margin = 0;
    return string(margin, ' ');
}
string plainPad(int len) {
    int margin = (getConsoleWidth() - len) / 2;
    if (margin < 0) margin = 0;
    return string(margin, ' ');
}
// Console Utility Functions
void enableColors() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hOut, &mode);
    SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}
void pressAnyKey() {
    string msg = "Press any key to return to the main menu...";
    cout<<"\n"<<plainPad((int)msg.size())<<YELLOW<<msg<<RESET;
#ifdef _WIN32
    _getch();
#else
    system("stty raw -echo");
    getchar();
    system("stty cooked echo");
#endif
    cout << endl;
}
// String Formatting Helpers
string padRight(const string& s, int width) {
    if ((int)s.size() >= width) return s.substr(0, width);
    return s + string(width - s.size(), ' ');
}
string centerText(const string& s, int width) {
    int gap = width - (int)s.size();
    if (gap <= 0) return s.substr(0, width);
    int left = gap / 2, right = gap - left;
    return string(left, ' ') + s + string(right, ' ');
}
// Box Drawing Functions
void boxBorder(const string& color) {
    cout << boxPad() << color << "+" << string(BOX_WIDTH, '=') << "+" << RESET << endl;
}
void boxLine(const string& text, const string& borderColor, const string& textColor) {
    cout << boxPad() << borderColor << "|" << RESET
         << textColor << centerText(text, BOX_WIDTH) << RESET
         << borderColor << "|" << RESET << endl;
}
void menuLine(const string& text) {
    cout << boxPad() << CYAN << "|" << RESET
         << WHITE << padRight(" " + text, BOX_WIDTH) << RESET
         << CYAN << "|" << RESET << endl;
}
void sectionHeader(const string& text, const string& color) {
    string content = " " + text + " ";
    int dashes = BOX_WIDTH - (int)content.size();
    if (dashes < 0) dashes = 0;
    int left = dashes / 2, right = dashes - left;
    string line = string(left, '-') + content + string(right, '-');
    cout << "\n" << plainPad((int)line.size()) << color << line << RESET << endl;
}
void printCenteredMsg(const string& colorCode, const string& text) {
    cout << "\n" << plainPad((int)text.size()) << colorCode << text << RESET << endl;
}
//Exception Handling:
class LibraryException : public exception {
protected: string msg;
public:
    LibraryException(string m) : msg(m) {}
    const char* what() const noexcept override { return msg.c_str(); }
};
class FileException          : public LibraryException { public: FileException(string m)          : LibraryException("File Error: " + m) {} };
class InvalidInputException  : public LibraryException { public: InvalidInputException(string m)   : LibraryException("Invalid Input: " + m) {} };
class ItemNotFoundException  : public LibraryException { public: ItemNotFoundException(string m)   : LibraryException("Item Not Found: " + m) {} };
class MemberNotFoundException: public LibraryException { public: MemberNotFoundException(string m) : LibraryException("Member Not Found: " + m) {} };
class BorrowLimitException   : public LibraryException { public: BorrowLimitException(string m)    : LibraryException("Borrow Limit Exceeded: " + m) {} };
class DuplicateEntryException: public LibraryException { public: DuplicateEntryException(string m) : LibraryException("Duplicate Entry: " + m) {} };
// Abstraction: Abstract base class defining the common interface for all library items
class LibraryItem {
protected:
    string itemID, title;
    bool isIssued;
    static int totalItems;
public:
    LibraryItem(string id, string t) : itemID(id), title(t), isIssued(false) { totalItems++; }
    virtual ~LibraryItem() { totalItems--; }
    // Pure virtual functions enforce abstraction — subclasses must implement these
    virtual void displayDetails() const = 0;
    virtual string getType()      const = 0;
    virtual string serialize()    const = 0;
    // Encapsulation: private data exposed through public getters/setters
    string getID()           const { return itemID; }
    string getTitle()        const { return title; }
    bool   getIssuedStatus() const { return isIssued; }
    void   setIssued(bool s)       { isIssued = s; }
    static int getTotalItems()     { return totalItems; }
};
int LibraryItem::totalItems = 0;
// Inheritance: Book inherits from LibraryItem and provides concrete implementation
class Book : public LibraryItem {
private:
    // Encapsulation: book-specific data hidden from outside
    string author, isbn, genre;
public:
    Book(string id, string t, string a, string i, string g)
        : LibraryItem(id, t), author(a), isbn(i), genre(g) {}
    // Polymorphism: overrides base class pure virtual function
    void displayDetails() const override {
        string plain = "[Book] ID: " + itemID + " | Title: " + title
                     + " | Author: " + author + " | ISBN: " + isbn
                     + " | Genre: " + genre + " | Status: "
                     + (isIssued ? "Issued" : "Available");
        cout << plainPad((int)plain.size())
             << CYAN << "[Book] " << RESET
             << "ID: " << itemID << " | Title: " << title
             << " | Author: " << author << " | ISBN: " << isbn
             << " | Genre: " << genre << " | Status: "
             << (isIssued ? RED : GREEN) << (isIssued ? "Issued" : "Available")
             << RESET << endl;
    }
    string getType()   const override { return "Book"; }
    string serialize() const override {
        stringstream ss;
        ss << "Book," << itemID << "," << title << "," << author << ","
           << isbn << "," << genre << "," << isIssued;
        return ss.str();
    }
    string getAuthor() const { return author; }
    string getISBN()   const { return isbn; }
    // Operator Overloading: custom stream output for Book
    friend ostream& operator<<(ostream& out, const Book& b);
};
ostream& operator<<(ostream& out, const Book& b) {
    out << "Book{" << b.itemID << ", " << b.title << ", " << b.author << "}";
    return out;
}
// Inheritance: Magazine inherits from LibraryItem and provides concrete implementation
class Magazine : public LibraryItem {
private:
    // Encapsulation: magazine-specific data hidden from outside
    string publisher;
    int issueNumber;
public:
    Magazine(string id, string t, string p, int num)
        : LibraryItem(id, t), publisher(p), issueNumber(num) {}
   // Polymorphism: overrides base class pure virtual function
    void displayDetails() const override {
        string plain = "[Magazine] ID: " + itemID + " | Title: " + title
                     + " | Publisher: " + publisher
                     + " | Issue #: " + to_string(issueNumber)
                     + " | Status: " + (isIssued ? "Issued" : "Available");
        cout << plainPad((int)plain.size())
             << MAGENTA << "[Magazine] " << RESET
             << "ID: " << itemID << " | Title: " << title
             << " | Publisher: " << publisher << " | Issue #: " << issueNumber
             << " | Status: " << (isIssued ? RED : GREEN)
             << (isIssued ? "Issued" : "Available") << RESET << endl;
    }
    string getType()   const override { return "Magazine"; }
    string serialize() const override {
        stringstream ss;
        ss << "Magazine," << itemID << "," << title << "," << publisher
           << "," << issueNumber << "," << isIssued;
        return ss.str();
    }
    string getPublisher()   const { return publisher; }
    int    getIssueNumber() const { return issueNumber; }
};
// Abstraction: Abstract base class for all people in the system
class Person {
protected:
    string id, name;
public:
    Person(string i, string n) : id(i), name(n) {}
    virtual ~Person() {}
    // Pure virtual functions enforce abstraction
    virtual void   displayInfo() const = 0;
    virtual string getRole()     const = 0;
    // Encapsulation: id and name exposed through public getters only
    string getID()   const { return id; }
    string getName() const { return name; }
};
// Inheritance: Member inherits from Person
class Member : public Person {
private:
    // Encapsulation: borrow list is private and managed through controlled methods
    vector<string> borrowedItemIDs;
    static int memberCount;
public:
    static const int MAX_BORROW_LIMIT = 3;
    Member(string i, string n) : Person(i, n) { memberCount++; }
    // Polymorphism: overrides Person's pure virtual displayInfo
    void displayInfo() const override {
        string plain = "[Member] ID: " + id + " | Name: " + name
                     + " | Books Borrowed: " + to_string(borrowedItemIDs.size())
                     + "/" + to_string(MAX_BORROW_LIMIT);
        cout << plainPad((int)plain.size())
             << BLUE << "[Member] " << RESET
             << "ID: " << id << " | Name: " << name
             << " | Books Borrowed: " << borrowedItemIDs.size()
             << "/" << MAX_BORROW_LIMIT << endl;
    }
    string getRole() const override { return "Member"; }
    bool canBorrow() const { return (int)borrowedItemIDs.size() < MAX_BORROW_LIMIT; }
    // Exception Handling: throws BorrowLimitException if borrow limit is exceeded
    void addBorrowedItem(const string& itemID) {
        if (!canBorrow())
            throw BorrowLimitException(name + " has already reached the maximum borrow limit of "
                                       + to_string(MAX_BORROW_LIMIT) + " items.");
        borrowedItemIDs.push_back(itemID);
    }
    void removeBorrowedItem(const string& itemID) {
        auto it = find(borrowedItemIDs.begin(), borrowedItemIDs.end(), itemID);
        if (it != borrowedItemIDs.end()) borrowedItemIDs.erase(it);
    }
    vector<string> getBorrowedItems() const { return borrowedItemIDs; }
    string serializeBorrowed() const {
        stringstream ss;
        for (size_t i = 0; i < borrowedItemIDs.size(); i++) {
            ss << borrowedItemIDs[i];
            if (i != borrowedItemIDs.size() - 1) ss << ";";
        }
        return ss.str();
    }
    static int getMemberCount() { return memberCount; }
};
int Member::memberCount = 0;
// Inheritance: Librarian inherits from Person
class Librarian : public Person {
public:
    Librarian(string i, string n) : Person(i, n) {}
    // Polymorphism: overrides Person's pure virtual displayInfo
    void displayInfo() const override {
        string plain = "[Librarian] ID: " + id + " | Name: " + name;
        cout << plainPad((int)plain.size())
             << YELLOW << "[Librarian] " << RESET
             << "ID: " << id << " | Name: " << name << endl;
    }
    string getRole() const override { return "Librarian"; }
};
// Encapsulation: Library class bundles all data and operations into a single unit
class Library {
private:
    // Private data members — only accessible through public methods
    vector<LibraryItem*> items;
    vector<Member>       members;
    const string booksFile       = "books.csv";
    const string membersFile     = "members.csv";
    const string transactionFile = "transactions.txt";
    // Exception Handling: throws ItemNotFoundException if item is missing
    LibraryItem* findItemByID(const string& id) {
        for (auto item : items) if (item->getID() == id) return item;
        throw ItemNotFoundException("No catalogue item with ID \"" + id + "\"");
    }
    // Exception Handling: throws MemberNotFoundException if member is missing
    Member* findMemberByID(const string& id) {
        for (auto& m : members) if (m.getID() == id) return &m;
        throw MemberNotFoundException("No member with ID \"" + id + "\"");
    }
    bool itemExists  (const string& id) const { for (auto item : items)      if (item->getID() == id) return true; return false; }
    bool memberExists(const string& id) const { for (const auto& m : members) if (m.getID() == id)    return true; return false; }

public:
    // File Handling: load saved data on startup
    Library() {
        try { loadData(); }
        catch (const exception& e) {
            string w = string("Warning while loading saved data: ") + e.what();
            cout << plainPad((int)w.size()) << YELLOW << w << RESET << endl;
        }
    }
    ~Library() { for (auto item : items) delete item; items.clear(); }
    // Exception Handling: validates input and throws on duplicates or empty fields
    void addBook(const string& id, const string& title, const string& author,
                 const string& isbn, const string& genre) {
        if (id.empty() || title.empty()) throw InvalidInputException("Book ID and Title cannot be empty.");
        if (itemExists(id)) throw DuplicateEntryException("An item with ID \"" + id + "\" already exists.");
        items.push_back(new Book(id, title, author, isbn, genre));
        logTransaction("ADD_BOOK,id=" + id + ",title=" + title);
    }
    // Exception Handling: validates input and throws on duplicates or empty fields
    void addMagazine(const string& id, const string& title,
                     const string& publisher, int issueNumber) {
        if (id.empty() || title.empty()) throw InvalidInputException("Magazine ID and Title cannot be empty.");
        if (itemExists(id)) throw DuplicateEntryException("An item with ID \"" + id + "\" already exists.");
        items.push_back(new Magazine(id, title, publisher, issueNumber));
        logTransaction("ADD_MAGAZINE,id=" + id + ",title=" + title);
    }
    // Exception Handling: prevents removal of currently issued items
    void removeItem(const string& id) {
        LibraryItem* item = findItemByID(id);
        if (item->getIssuedStatus())
            throw InvalidInputException("Item \"" + id + "\" is currently issued, return it before removing.");
        items.erase(remove_if(items.begin(), items.end(), [&](LibraryItem* it) {
            if (it->getID() == id) { delete it; return true; } return false;
        }), items.end());
        logTransaction("REMOVE_ITEM,id=" + id);
    }
    // Exception Handling: throws if item is already issued or member cannot borrow more
    void issueItem(const string& itemID, const string& memberID) {
        LibraryItem* item   = findItemByID(itemID);
        Member*      member = findMemberByID(memberID);
        if (item->getIssuedStatus())
            throw InvalidInputException("Item \"" + itemID + "\" is already issued to someone else.");
        member->addBorrowedItem(itemID);
        item->setIssued(true);
        logTransaction("ISSUE,item=" + itemID + ",member=" + memberID);
    }
    // Exception Handling: throws if item was not marked as issued
    void returnItem(const string& itemID, const string& memberID) {
        LibraryItem* item   = findItemByID(itemID);
        Member*      member = findMemberByID(memberID);
        if (!item->getIssuedStatus())
            throw InvalidInputException("Item \"" + itemID + "\" was not marked as issued.");
        member->removeBorrowedItem(itemID);
        item->setIssued(false);
        logTransaction("RETURN,item=" + itemID + ",member=" + memberID);
    }
    // Polymorphism: calls displayDetails() on each item — actual type (Book/Magazine) determines output
    void searchItem(const string& keyword) const {
        bool found = false;
        string lk = keyword;
        transform(lk.begin(), lk.end(), lk.begin(), ::tolower);
        for (auto item : items) {
            string lt = item->getTitle();
            transform(lt.begin(), lt.end(), lt.begin(), ::tolower);
            if (lt.find(lk) != string::npos || item->getID() == keyword) {
                item->displayDetails(); found = true;
            }
        }
        if (!found) {
            string msg = "No items found matching \"" + keyword + "\".";
            cout << plainPad((int)msg.size()) << YELLOW << msg << RESET << endl;
        }
    }
    // Polymorphism: calls displayDetails() on each item — actual type determines output
    void displayAllItems() const {
        if (items.empty()) {
            string msg = "The catalogue is currently empty.";
            cout << plainPad((int)msg.size()) << YELLOW << msg << RESET << endl;
            return;
        }
        sectionHeader("All Library Items (" + to_string(items.size()) + ")", CYAN);
        for (auto item : items) item->displayDetails();
    }
    // Exception Handling: throws on empty fields or duplicate member ID
    void registerMember(const string& id, const string& name) {
        if (id.empty() || name.empty()) throw InvalidInputException("Member ID and Name cannot be empty.");
        if (memberExists(id)) throw DuplicateEntryException("A member with ID \"" + id + "\" already exists.");
        members.push_back(Member(id, name));
        logTransaction("REGISTER_MEMBER,id=" + id + ",name=" + name);
    }
    void displayAllMembers() const {
        if (members.empty()) {
            string msg = "No registered members yet.";
            cout << plainPad((int)msg.size()) << YELLOW << msg << RESET << endl;
            return;
        }
        sectionHeader("All Members (" + to_string(members.size()) + ")", BLUE);
        for (const auto& m : members) m.displayInfo();
    }
    // File Handling: persists items and members to CSV files
    void saveData() {
        ofstream bookOut(booksFile);
        if (!bookOut.is_open()) throw FileException("Could not open \"" + booksFile + "\" for writing.");
        for (auto item : items) bookOut << item->serialize() << "\n";
        bookOut.close();
        ofstream memberOut(membersFile);
        if (!memberOut.is_open()) throw FileException("Could not open \"" + membersFile + "\" for writing.");
        for (const auto& m : members)
            memberOut << m.getID() << "," << m.getName() << "," << m.serializeBorrowed() << "\n";
        memberOut.close();
        string msg = "Data saved successfully (" + to_string(items.size()) + " items, "
                   + to_string(members.size()) + " members).";
        cout << plainPad((int)msg.size()) << GREEN << msg << RESET << endl;
    }
    // File Handling: reads items and members from CSV files on startup
    void loadData() {
        ifstream bookIn(booksFile);
        if (!bookIn.is_open()) {
            string msg = "No existing \"" + booksFile + "\" found, starting with an empty catalogue.";
            cout << plainPad((int)msg.size()) << YELLOW << msg << RESET << endl;
        } else {
            string line;
            while (getline(bookIn, line)) {
                if (line.empty()) continue;
                // Exception Handling: skip corrupted or unrecognized lines with a warning
                try {
                    stringstream ss(line); vector<string> tokens; string token;
                    while (getline(ss, token, ',')) tokens.push_back(token);
                    if (tokens.size() < 2) throw FileException("Corrupted line skipped: \"" + line + "\"");
                    if (tokens[0] == "Book" && tokens.size() >= 7) {
                        Book* b = new Book(tokens[1], tokens[2], tokens[3], tokens[4], tokens[5]);
                        if (tokens[6] == "1") b->setIssued(true);
                        items.push_back(b);
                    } else if (tokens[0] == "Magazine" && tokens.size() >= 6) {
                        Magazine* mg = new Magazine(tokens[1], tokens[2], tokens[3], stoi(tokens[4]));
                        if (tokens[5] == "1") mg->setIssued(true);
                        items.push_back(mg);
                    } else { throw FileException("Unrecognized record type skipped: \"" + line + "\""); }
                } catch (const exception& e) {
                    string w = string("Warning: ") + e.what();
                    cout << plainPad((int)w.size()) << YELLOW << w << RESET << endl;
                }
            }
            bookIn.close();
        }
        ifstream memberIn(membersFile);
        if (!memberIn.is_open()) {
            string msg = "No existing \"" + membersFile + "\" found, starting with no members.";
            cout << plainPad((int)msg.size()) << YELLOW << msg << RESET << endl;
        } else {
            string line;
            while (getline(memberIn, line)) {
                if (line.empty()) continue;
                // Exception Handling: skip corrupted member records with a warning
                try {
                    stringstream ss(line); vector<string> tokens; string token;
                    while (getline(ss, token, ',')) tokens.push_back(token);
                    if (tokens.size() < 2) throw FileException("Corrupted member record: \"" + line + "\"");
                    Member m(tokens[0], tokens[1]);
                    if (tokens.size() >= 3 && !tokens[2].empty()) {
                        stringstream bs(tokens[2]); string bid;
                        while (getline(bs, bid, ';')) if (!bid.empty()) m.addBorrowedItem(bid);
                    }
                    members.push_back(m);
                } catch (const exception& e) {
                    string w = string("Warning: ") + e.what();
                    cout << plainPad((int)w.size()) << YELLOW << w << RESET << endl;
                }
            }
            memberIn.close();
        }
    }
    // File Handling: appends every action to the transaction log
    void logTransaction(const string& message) {
        ofstream log(transactionFile, ios::app);
        if (!log.is_open()) { cerr << YELLOW << "Warning: could not write to transaction log." << RESET << endl; return; }
        log << message << "\n";
        log.close();
    }
    // Static method: uses static counters from LibraryItem and Member
    static void printSystemStats() {
        sectionHeader("System Statistics", MAGENTA);
        string l1 = "Total items currently loaded in memory: " + to_string(LibraryItem::getTotalItems());
        string l2 = "Total members created this session:     " + to_string(Member::getMemberCount());
        cout << plainPad((int)l1.size()) << WHITE << "Total items currently loaded in memory: "
             << RESET << BOLD << YELLOW << LibraryItem::getTotalItems() << RESET << endl;
        cout << plainPad((int)l2.size()) << WHITE << "Total members created this session:     "
             << RESET << BOLD << YELLOW << Member::getMemberCount()     << RESET << endl;
    }
};
// Menu Display
void showMenu() {
    boxBorder(CYAN);
    boxLine("LIBRARY MANAGEMENT SYSTEM", CYAN, BOLD + YELLOW);
    boxBorder(CYAN);
    vector<string> options = {
        " 1. Add Book",              " 2. Add Magazine",
        " 3. Remove Item",           " 4. Issue Item to Member",
        " 5. Return Item",           " 6. Search Item (by title/ID)",
        " 7. Display All Items",     " 8. Register New Member",
        " 9. Display All Members",   "10. View System Statistics",
        "11. Save Data Now",         " 0. Save & Exit"
    };
    for (auto& opt : options) menuLine(opt);
    boxBorder(CYAN);
    string prompt = "Enter your choice: ";
    cout << plainPad((int)prompt.size()) << BOLD << GREEN << prompt << RESET;
}
// Exception Handling: throws InvalidInputException on non-numeric menu input
int getValidatedChoice() {
    int choice;
    cin >> choice;
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        throw InvalidInputException("Menu choice must be a number.");
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return choice;
}
string getLine(const string& prompt) {
    cout << plainPad((int)prompt.size()) << CYAN << prompt << RESET;
    string value;
    getline(cin, value);
    return value;
}
void printWelcomeBanner() {
    boxBorder(MAGENTA);
    boxLine("WELCOME", MAGENTA, BOLD + CYAN);
    boxLine("Library Management System", MAGENTA, WHITE);
    boxBorder(MAGENTA);
}
void printGoodbyeBanner() {
    boxBorder(MAGENTA);
    boxLine("GOODBYE!", MAGENTA, BOLD + CYAN);
    boxLine("Thanks for using the Library System", MAGENTA, WHITE);
    boxBorder(MAGENTA);
}
int main() {
    enableColors();
    Library library;
    bool running = true;
    clearScreen();
    printWelcomeBanner();
    pressAnyKey();
    // Main application loop
    while (running) {
        clearScreen();
        // Exception Handling: catches all LibraryException and std::exception types
        try {
            showMenu();
            int choice = getValidatedChoice();
            clearScreen();
            switch (choice) {
                case 1: {
                    sectionHeader("Add Book", CYAN);
                    string id     = getLine("Enter Book ID   : ");
                    string title  = getLine("Enter Title     : ");
                    string author = getLine("Enter Author    : ");
                    string isbn   = getLine("Enter ISBN      : ");
                    string genre  = getLine("Enter Genre     : ");
                    library.addBook(id, title, author, isbn, genre);
                    printCenteredMsg(GREEN, "Book added successfully!");
                    break;
                }
                case 2: {
                    sectionHeader("Add Magazine", MAGENTA);
                    string id        = getLine("Enter Magazine ID  : ");
                    string title     = getLine("Enter Title        : ");
                    string publisher = getLine("Enter Publisher    : ");
                    string issueStr  = getLine("Enter Issue Number : ");
                    int issueNumber;
                    // Exception Handling: catches invalid issue number conversion
                    try { issueNumber = stoi(issueStr); }
                    catch (...) { throw InvalidInputException("Issue number must be a whole number."); }
                    library.addMagazine(id, title, publisher, issueNumber);
                    printCenteredMsg(GREEN, "Magazine added successfully!");
                    break;
                }
                case 3: {
                    sectionHeader("Remove Item", RED);
                    string id = getLine("Enter Item ID to remove : ");
                    library.removeItem(id);
                    printCenteredMsg(GREEN, "Item removed successfully!");
                    break;
                }
                case 4: {
                    sectionHeader("Issue Item to Member", YELLOW);
                    string itemID   = getLine("Enter Item ID   : ");
                    string memberID = getLine("Enter Member ID : ");
                    library.issueItem(itemID, memberID);
                    printCenteredMsg(GREEN, "Item issued successfully!");
                    break;
                }
                case 5: {
                    sectionHeader("Return Item", YELLOW);
                    string itemID   = getLine("Enter Item ID   : ");
                    string memberID = getLine("Enter Member ID : ");
                    library.returnItem(itemID, memberID);
                    printCenteredMsg(GREEN, "Item returned successfully!");
                    break;
                }
                case 6: {
                    sectionHeader("Search Item", CYAN);
                    string keyword = getLine("Enter title or ID to search : ");
                    cout << endl;
                    library.searchItem(keyword);
                    break;
                }
                case 7:
                    library.displayAllItems();
                    break;
                case 8: {
                    sectionHeader("Register New Member", BLUE);
                    string id   = getLine("Enter Member ID   : ");
                    string name = getLine("Enter Member Name : ");
                    library.registerMember(id, name);
                    printCenteredMsg(GREEN, "Member registered successfully!");
                    break;
                }
                case 9:
                    library.displayAllMembers();
                    break;
                case 10:
                    Library::printSystemStats();
                    break;
                case 11:
                    library.saveData();
                    break;
                case 0:
                    printCenteredMsg(YELLOW, "Saving data and exiting...");
                    library.saveData();
                    running = false;
                    break;
                default:
                    throw InvalidInputException("Choice must be between 0 and 11.");
            }
            if (running) pressAnyKey();
        } catch (const LibraryException& e) {
            string msg = string("[Error] ") + e.what();
            printCenteredMsg(RED, msg);
            pressAnyKey();
        } catch (const exception& e) {
            string msg = string("[Unexpected Error] ") + e.what();
            printCenteredMsg(RED, msg);
            pressAnyKey();
        }
    }
    clearScreen();
    printGoodbyeBanner();
}
