#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PASSWORD "12345"

typedef struct Book {
    char title[100];
    char author[100];
    char edition[50];
    char callNumber[30];
    int rowNumber;          // ✅ Row number
    int cupboardNumber;     // ✅ Cupboard number
    struct Book *left, *right;
} Book;
typedef struct IssueLog {
    char personName[100];
    char regNo[20];
    char callNumber[30];
    char date[15]; // Format: YYYY-MM-DD
} IssueLog;

Book *root = NULL;

// -------- Function Declarations --------
Book* insertBook(Book* root, Book* newBook);
Book* deleteBook(Book* root, char callNumber[]);
Book* searchBook(Book* root, char title[]);
void displayBooks(Book* root);
void saveBooksToFile(Book* root, FILE *fp);
void loadBooksFromFile();
void issueBook();
void addBook();
void deleteBookByHead();
void searchBookByUser();
void displayIssuedBooks();
void insertIssueLog(IssueLog newLog);
void sortIssueLogsByDate();

// -------- Utility Functions --------
int authenticate() {
    char input[10];
    printf("Enter password: ");
    scanf("%s", input);
    return strcmp(input, PASSWORD) == 0;
}

// -------- Tree Book Functions --------
Book* insertBook(Book* root, Book* newBook) {
    if (root == NULL) return newBook;
    if (strcmp(newBook->title, root->title) < 0)
        root->left = insertBook(root->left, newBook);
    else
        root->right = insertBook(root->right, newBook);
    return root;
}

Book* minValueNode(Book* node) {
    Book* current = node;
    while (current && current->left != NULL)
        current = current->left;
    return current;
}

Book* deleteBook(Book* root, char callNumber[]) {
    if (root == NULL) return root;
    if (strcmp(callNumber, root->callNumber) < 0)
        root->left = deleteBook(root->left, callNumber);
    else if (strcmp(callNumber, root->callNumber) > 0)
        root->right = deleteBook(root->right, callNumber);
    else {
        if (root->left == NULL) {
            Book* temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            Book* temp = root->left;
            free(root);
            return temp;
        }
        Book* temp = minValueNode(root->right);
        strcpy(root->title, temp->title);
        strcpy(root->author, temp->author);
        strcpy(root->edition, temp->edition);
        strcpy(root->callNumber, temp->callNumber);
        root->right = deleteBook(root->right, temp->callNumber);
    }
    return root;
}

Book* searchBook(Book* root, char title[]) {
    if (root == NULL) return NULL;
    if (strcmp(title, root->title) == 0)
        return root;
    if (strcmp(title, root->title) < 0)
        return searchBook(root->left, title);
    else
        return searchBook(root->right, title);
}
void displayBooks(Book* root) {
    if (root == NULL) return;
    displayBooks(root->left);
    printf("Title: %s\nAuthor: %s\nEdition: %s\nCall Number: %s\n", 
           root->title, root->author, root->edition, root->callNumber);
    printf("Row Number: %d, Cupboard Number: %d\n\n", 
           root->rowNumber, root->cupboardNumber);
    displayBooks(root->right);
}/*void displayBooks(Book* root) {
    if (root == NULL) return;
    displayBooks(root->left);
    printf("Title: %s\nAuthor: %s\nEdition: %s\nCall Number: %s\n\n",
           root->title, root->author, root->edition, root->callNumber);
    displayBooks(root->right);
}*/

// -------- File Handling --------
void saveBooksToFile(Book* root, FILE *fp) {
    if (root == NULL) return;
    fwrite(root, sizeof(Book), 1, fp);
    saveBooksToFile(root->left, fp);
    saveBooksToFile(root->right, fp);
}

void loadBooksFromFile() {
    FILE *fp = fopen("books.dat", "rb");
    if (fp == NULL) return;
    Book temp;
    while (fread(&temp, sizeof(Book), 1, fp)) {
        Book *newBook = (Book*)malloc(sizeof(Book));
        *newBook = temp;
        newBook->left = newBook->right = NULL;
        root = insertBook(root, newBook);
    }
    fclose(fp);
}
Book* searchBookByCallNumber(Book* root, const char* callNumber) {
    if (root == NULL) return NULL;
    if (strcmp(root->callNumber, callNumber) == 0) return root;

    Book* leftResult = searchBookByCallNumber(root->left, callNumber);
    if (leftResult) return leftResult;

    return searchBookByCallNumber(root->right, callNumber);
}

// -------- Issue Book & Logs --------
void issueBook() {
    if (!authenticate()) {
        printf("Access denied!\n");
        return;
    }
    
    char callNumber[30];
    printf("Enter call number of book to issue: ");
    scanf(" %[^\n]", callNumber);
    Book* book = searchBookByCallNumber(root, callNumber);
     if (book == NULL) {
	     printf("Book not found!\n");
	     return;
     }
    IssueLog log;
    strcpy(log.callNumber, callNumber);
    printf("Enter person name: ");
    scanf(" %[^\n]", log.personName);
    printf("Enter registration number: ");
    scanf(" %[^\n]", log.regNo);
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(log.date, 15, "%Y-%m-%d", tm_info);
    insertIssueLog(log);
    printf("Book issued successfully!\n");
}
void insertIssueLog(IssueLog newLog) {
    FILE *fp = fopen("issues.dat", "ab");
    fwrite(&newLog, sizeof(IssueLog), 1, fp);
    fclose(fp);
    sortIssueLogsByDate();
}
int compareDate(const char *d1, const char *d2) {
    return strcmp(d2, d1); // decreasing order
}
void sortIssueLogsByDate() {
    IssueLog logs[100];
    int count = 0;
    FILE *fp = fopen("issues.dat", "rb");
    if (fp == NULL) return;
    while (fread(&logs[count], sizeof(IssueLog), 1, fp))
        count++;
    fclose(fp);
    for (int i = 0; i < count-1; i++)
        for (int j = i+1; j < count; j++)
            if (compareDate(logs[i].date, logs[j].date) > 0) {
                IssueLog temp = logs[i];
                logs[i] = logs[j];
                logs[j] = temp;
            }

    fp = fopen("issues.dat", "wb");
    fwrite(logs, sizeof(IssueLog), count, fp);
    fclose(fp);
}
void displayIssuedBooks() {
    FILE *fp = fopen("issues.dat", "rb");
    if (fp == NULL) {
        printf("No issued books yet.\n");
        return;
    }

    IssueLog log;
    printf("\n--- Issued Books ---\n");

    while (fread(&log, sizeof(IssueLog), 1, fp)) {
        // Find book by call number in the tree
        Book *book = searchBookByCallNumber(root, log.callNumber);
        if (book) {
            printf("Title: %s\n", book->title);
        } else {
            printf("Title: Not found in system\n");
        }

        printf("Call Number: %s\n", log.callNumber);
        printf("Issued To: %s (Reg No: %s)\n", log.personName, log.regNo);
        printf("Date of Issue: %s\n\n", log.date);
    }
    fclose(fp);
}
void addBook() {
    if (!authenticate()) {
        printf("Access denied!\n");
        return;
    }
    Book *newBook = (Book*)malloc(sizeof(Book));
    printf("Enter book title: ");
    scanf(" %[^\n]", newBook->title);
    printf("Enter book author: ");
    scanf(" %[^\n]", newBook->author);
    printf("Enter book edition: ");
    scanf(" %[^\n]", newBook->edition);
    printf("Enter call number: ");
    scanf(" %[^\n]", newBook->callNumber);
    newBook->left = newBook->right = NULL;
    root = insertBook(root, newBook);
    printf("Enter row number: ");
    scanf("%d", &newBook->rowNumber);
    printf("Enter cupboard number: ");
    scanf("%d", &newBook->cupboardNumber);
    FILE *fp = fopen("books.dat", "wb");
    saveBooksToFile(root, fp);
    fclose(fp);
    printf("Book added successfully.\n");
}

void deleteBookByHead() {
    if (!authenticate()) {
        printf("Access denied!\n");
        return;
    }
    char callNumber[30];
    printf("Enter call number of book to delete: ");
    scanf(" %[^\n]", callNumber);
    root = deleteBook(root, callNumber);
    FILE *fp = fopen("books.dat", "wb");
    saveBooksToFile(root, fp);
    fclose(fp);
    printf("Book deleted successfully.\n");
}

void searchBookByUser() {
    char title[100];
    printf("Enter book title to search: ");
    scanf(" %[^\n]", title);
    Book *book = searchBook(root, title);
    if (book) {
        printf("Title: %s\nAuthor: %s\nEdition: %s\nCall Number: %s\n",
               book->title, book->author, book->edition, book->callNumber);
    } else {
        printf("Book not found.\n");
    }
}
void returnBook() {
    if (!authenticate()) {
        printf("Access denied!\n");
        return;
    }

    char callNumber[30], regNo[20];
    printf("Enter call number of returned book: ");
    scanf(" %[^\n]", callNumber);
    printf("Enter registration number of person: ");
    scanf(" %[^\n]", regNo);

    FILE *fp = fopen("issues.dat", "rb");
    FILE *temp = fopen("temp.dat", "wb");
    int found = 0;

    if (!fp || !temp) {
        printf("Error opening files.\n");
        return;
    }

    IssueLog log;
    while (fread(&log, sizeof(IssueLog), 1, fp)) {
        if (strcmp(log.callNumber, callNumber) == 0 &&
            strcmp(log.regNo, regNo) == 0) {
            found = 1;
            continue; // skip writing this entry
        }
        fwrite(&log, sizeof(IssueLog), 1, temp);
    }

    fclose(fp);
    fclose(temp);

    remove("issues.dat");
    rename("temp.dat", "issues.dat");

    if (found)
        printf("Book return processed and record deleted.\n");
    else
        printf("Matching issue record not found.\n");
}

// -------- Main Menu --------
int main() {
    loadBooksFromFile();

    int choice;
    do {
        printf("\n==== Library System ====\n");
        printf("1. Add Book (Head only)\n");
        printf("2. Delete Book (Head only)\n");
        printf("3. Search Book (User)\n");
        printf("4. Display All Books\n");
        printf("5. Issue Book (Head only)\n");
        printf("6. Display Issued Books\n");
        printf("7. Return Book (Head only)\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        switch(choice) {
            case 1: addBook(); break;
            case 2: deleteBookByHead(); break;
            case 3: searchBookByUser(); break;
            case 4: displayBooks(root); break;
            case 5: issueBook(); break;
            case 6: displayIssuedBooks(); break;
            case 0: printf("Goodbye!\n"); break;
            case 7: returnBook(); break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);

    return 0;
}

