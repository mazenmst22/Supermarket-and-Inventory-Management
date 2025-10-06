/*
 * Supermarket management and Inventory management APP
 *
 * PROJECT OVERVIEW:
 * A comprehensive console-based supermarket management system that handles inventory operations,
 * sales processing, and role-based access control with export functionality.
 *
 * ARCHITECTURE & DESIGN PATTERNS:
 *
 * Tools:
 * > ScreenManager - Handles UI/UX aspects
 *   1) clearScreen() - Clears console for better readability
 *   2) pauseForUser() - Controls flow by waiting for user input
 *   3) clearInputBuffer() - Ensures clean input handling
 *
 * > TimeTools - Provides timestamp functionality
 *   1) now_timestamp() - Generates unique timestamps for file naming
 *
 * > FileExporter - Manages file operations
 *   1) exportToFile() - Safely exports content to files with validation
 *
 * Product:
 * 1) Core data structure representing supermarket products
 * 2) toString() - Formats product info for console display
 * 3) toFileString() - Formats product info for file storage
 *
 * Interface (SOLID principle - Interface Segregation):
 * > IPrintable - Defines contract for printable entities
 *   - Virtual destructor for proper cleanup
 *   - Pure virtual functions for abstraction
 *
 * > IInventoryOperations - Defines inventory management operations
 *   - Follows Single Responsibility Principle
 *
 * Inventory:
 * - Implements both IInventoryOperations and IPrintable interfaces
 * - Uses std::map for efficient product lookup by ID
 * - Provides complete CRUD operations with validation
 * - Includes stock level warnings (empty, low stock, full)
 * - Enforces business rules (max quantity 100)
 *
 * Receipt:
 * - Implements IPrintable interface
 * - Manages sales transactions and calculates totals
 * - Provides formatted receipt generation
 * - Supports clearing and status checking
 *
 * InputHandler:
 * - Centralized input validation system
 * - Type-safe input methods for different data types
 * - Error handling and user feedback
 * - Prevents common input errors
 *
 * Main Menu (Strategy Pattern):
 * - Abstract base class for role-specific menus
 * - Polymorphic menu system for different user roles
 *
 * Roles (Role-Based Access Control):
 * > AdminMenu - Full system access
 *   - All inventory operations
 *   - Sales processing
 *   - Export capabilities
 *
 * > InventoryManagerMenu - Inventory-focused access
 *   - Product management
 *   - Restocking
 *   - Inventory exports
 *
 * > CashierMenu - Sales-focused access
 *   - Product sales only
 *   - Receipt generation
 *   - Inventory viewing
 *
 * APP:
 * > SupermarketApp - Main application controller
 *   - Coordinates all system components
 *   - Manages application lifecycle
 *   - Handles role-based menu routing
 *
 * KEY FEATURES:
 * 1. Role-Based Access Control (RBAC) with three distinct roles
 * 2. Comprehensive Inventory Management with validation
 * 3. Sales Processing with automatic stock updates
 * 4. Smart Warnings for stock levels (empty, low, full)
 * 5. File Export for both inventory and receipts
 * 6. Input Validation and error handling
 * 7. Clean Console Interface with proper screen management
 * 8. Timestamp-based unique file naming
 *
 * DESIGN PRINCIPLES APPLIED:
 * - SOLID Principles (Single Responsibility, Interface Segregation)
 * - DRY (Don't Repeat Yourself) - reusable components
 * - Encapsulation - data hiding and proper access control
 * - Polymorphism - role-based menu system
 * - Composition over Inheritance - component-based architecture
 *
 * DATA FLOW:
 * User Input → InputHandler → Role Menu → Inventory/Receipt → File Export
 *
 * ERROR HANDLING:
 * - Input validation for all user entries
 * - File operation success/failure feedback
 * - Business rule enforcement (max quantities, stock levels)
 * - Graceful error recovery
 *
 * FILE MANAGEMENT:
 * - Automatic timestamp-based file naming
 * - Validation of file operations
 * - Human-readable and machine-parsable formats
 *
 * SECURITY ASPECTS:
 * - Role-based operation restrictions
 * - Input sanitization and validation
 * - No direct data access - only through interfaces
 *
 * EXTENSIBILITY:
 * - Easy to add new roles by extending MainMenu
 * - Simple to modify business rules (like max quantity)
 * - Can easily extend export formats (JSON, XML, etc.)
 * - Modular design allows for database integration
 *
 * USAGE SCENARIOS:
 * 1. Small to medium supermarket daily operations
 * 2. Inventory tracking and management
 * 3. Sales transaction processing
 * 4. Reporting and record keeping
 * 5. Multi-user environment with different access levels
 *
 * TECHNICAL STACK:
 * - C++ Standard Library (iostream, string, vector, map, fstream, etc.)
 * - Cross-platform compatible (except system("cls") - Windows specific)
 * - No external dependencies
 *
 * MAINTENANCE:
 * - Clear separation of concerns
 * - Well-documented code structure
 * - Easy to modify and extend
 * - Comprehensive error handling
 */
#include <iostream>
#include <string>
#include <limits>
#include <cstdlib>
#include <map>
#include <vector>
#include <fstream>
#include <ctime>
#include <chrono>
#include <sstream>

using namespace std;

// --------------------------------------Tools
class ScreenManager {
public:
    static void clearScreen() {
        system("cls"); // Windows only
    }

    static void pauseForUser() {
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    }

    static void clearInputBuffer() {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
};

class TimeTools {
public:
    static string now_timestamp() {
        auto t = chrono::system_clock::now();
        time_t tt = chrono::system_clock::to_time_t(t);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", localtime(&tt));
        return string(buf);
    }
};

class FileExporter {
public:
    static bool exportToFile(const string& content, const string& filepath) {
        ofstream out(filepath);
        if (!out.is_open()) {
            return false;
        }
        out << content;
        out.close();
        return true;
    }
};

// --------------------------------------Product
struct Product {
    int id{};
    string name;
    int quantity{};
    double price{};

    string toString() const {
        stringstream ss;
        ss << "ID: " << id << " | Name: " << name
           << " | Qty: " << quantity << " | Price: " << price;
        return ss.str();
    }

    string toFileString() const {
        stringstream ss;
        ss << id << " " << name << " " << quantity << " " << price;
        return ss.str();
    }
};

// --------------------------------------Interface (SOLID)
class IPrintable {
public:
    virtual ~IPrintable() = default;
    virtual string getFileContent() const = 0;
    virtual bool printToFile(const string& filepath) const {
        return FileExporter::exportToFile(getFileContent(), filepath);
    }
};

class IInventoryOperations {
public:
    virtual ~IInventoryOperations() = default;
    virtual bool insertProduct(const Product& p) = 0;
    virtual bool deleteProduct(int id) = 0;
    virtual bool restockProduct(int id, int amount) = 0;
    virtual bool sellProduct(int id, int amount, Product& outTaken) = 0;
    virtual void showInventory() const = 0;
};

// --------------------------------------Inventory
class Inventory : public IInventoryOperations, public IPrintable {
    map<int, Product> products;

public:
    bool insertProduct(const Product& p) override {
        if (products.count(p.id)) {
            cout << " X Product already exists.\n";
            return false;
        }
        if (p.quantity > 100) {
            cout << " X Quantity cannot exceed 100.\n";
            return false;
        }
        products[p.id] = p;
        cout << "Product inserted successfully. :D \n";
        return true;
    }

    bool deleteProduct(int id) override {
        if (products.erase(id)) {
            cout << " Product deleted successfully. :D \n";
            return true;
        }
        cout << " X Product not found.\n";
        return false;
    }

    bool restockProduct(int id, int amount) override {
        if (!products.count(id)) {
            cout << " X Product not found.\n";
            return false;
        }
        auto& prod = products[id];
        if (prod.quantity + amount > 100) {
            cout << " X Cannot restock beyond 100.\n";
            return false;
        }
        prod.quantity += amount;
        cout << "Restocked successfully.:D Current quantity: " << prod.quantity << "\n";
        return true;
    }

    bool sellProduct(int id, int amount, Product& outTaken) override {
        if (!products.count(id)) {
            cout << " X Product not found.\n";
            return false;
        }
        auto& prod = products[id];
        if (prod.quantity < amount) {
            cout << " X Not enough stock.\n";
            return false;
        }
        prod.quantity -= amount;
        outTaken = prod;
        outTaken.quantity = amount;

        //Warnings!!
        if (prod.quantity == 0)
            cout << " X Product '" << prod.name << "' is now EMPTY!\n";
        else if (prod.quantity < 20)
            cout << "  Product '" << prod.name << "' is SHORT and needs refilling!\n";
        else if (prod.quantity == 100)
            cout << " Product '" << prod.name << "' is FULL. :D\n";

        cout << "Sale successful. :D\n";
        return true;
    }

    void showInventory() const override {
        cout << "\n=== INVENTORY STATUS ===\n";
        if (products.empty()) {
            cout << "No products.\n";
            return;
        }
        for (auto& [id, p] : products) {
            cout << p.toString() << '\n';
        }
    }

    string getFileContent() const override {
        stringstream content;
        content << "=== INVENTORY EXPORT ===\n";
        content << "Timestamp: " << TimeTools::now_timestamp() << "\n\n";
        for (auto& [id, p] : products) {
            content << p.toFileString() << "\n";
        }
        return content.str();
    }

    bool productExists(int id) const {
        return products.count(id) > 0;
    }
};

// --------------------------------------Reciept
class Reciept : public IPrintable {
    vector<Product> soldItems;
    double total = 0.0;

public:
    void addItem(const string& name, int qty, double price) {
        Product p;
        p.name = name;
        p.quantity = qty;
        p.price = price;
        soldItems.push_back(p);
        total += price * qty;
    }

    string getFileContent() const override {
        stringstream content;
        content << "===== RECEIPT =====\n";
        content << "Timestamp: " << TimeTools::now_timestamp() << "\n";
        content << "-------------------\n";
        for (auto& p : soldItems) {
            content << p.name << " x" << p.quantity << " @ " << p.price
                   << " = " << (p.quantity * p.price) << "\n";
        }
        content << "-------------------\n";
        content << "Total: " << total << "\n";
        content << "===================\n";
        return content.str();
    }

    void clear() {
        soldItems.clear();
        total = 0.0;
    }

    bool isEmpty() const {
        return soldItems.empty();
    }
};

// --------------------------------------Handling the input
class InputHandler {
public:
    static int getIntInput(const string& prompt) {
        int value;
        while (true) {
            cout << prompt;
            cin >> value;
            if (cin.fail()) {
                cin.clear();
                ScreenManager::clearInputBuffer();
                cout << " X Invalid input. Please enter a number.\n";
            } else {
                ScreenManager::clearInputBuffer();
                return value;
            }
        }
    }

    static double getDoubleInput(const string& prompt) {
        double value;
        while (true) {
            cout << prompt;
            cin >> value;
            if (cin.fail()) {
                cin.clear();
                ScreenManager::clearInputBuffer();
                cout << "X Invalid input. Please enter a number.\n";
            } else {
                ScreenManager::clearInputBuffer();
                return value;
            }
        }
    }

    static string getStringInput(const string& prompt) {
        string value;
        cout << prompt;
        getline(cin, value);
        return value;
    }

    static Product getProductInput() {
        Product p;
        p.id = getIntInput("Enter product ID: ");
        cout << "Enter product name: ";
        getline(cin, p.name);
        p.quantity = getIntInput("Enter quantity: ");
        p.price = getDoubleInput("Enter price: ");
        return p;
    }
};

// --------------------------------------Main Menu
class MainMenu {
protected:
    Inventory& inventory;
    Reciept& receipt;

public:
    MainMenu(Inventory& inv, Reciept& rec) : inventory(inv), receipt(rec) {}
    virtual ~MainMenu() = default;
    virtual void show() = 0;
};

// --------------------------------------Roles
class AdminMenu : public MainMenu {
public:
    AdminMenu(Inventory& inv, Reciept& rec) : MainMenu(inv, rec) {}

    void show() override {
        while (true) {
            ScreenManager::clearScreen();
            cout << "\n=== ADMIN MENU ===\n";
            cout << "1. Insert Product\n2. Delete Product\n3. Restock\n4. Sell\n";
            cout << "5. Show Inventory\n6. Export Inventory\n7. Export Receipt\n8. Back\n";
            cout << "Choice: ";

            int choice = InputHandler::getIntInput("");

            if (choice == 8) break;

            processChoice(choice);
        }
    }

private:
    void processChoice(int choice) {
        ScreenManager::clearScreen();
        switch (choice) {
            case 1: insertProduct(); break;
            case 2: deleteProduct(); break;
            case 3: restockProduct(); break;
            case 4: sellProduct(); break;
            case 5: showInventory(); break;
            case 6: exportInventory(); break;
            case 7: exportReceipt(); break;
            default: cout << " X Invalid choice.\n"; break;
        }
        ScreenManager::pauseForUser();
    }

    void insertProduct() {
        cout << "=== INSERT PRODUCT ===\n";
        Product p = InputHandler::getProductInput();
        inventory.insertProduct(p);
    }

    void deleteProduct() {
        cout << "=== DELETE PRODUCT ===\n";
        int id = InputHandler::getIntInput("Enter product ID to delete: ");
        inventory.deleteProduct(id);
    }

    void restockProduct() {
        cout << "=== RESTOCK PRODUCT ===\n";
        int id = InputHandler::getIntInput("Enter product ID: ");
        int amount = InputHandler::getIntInput("Enter amount to restock: ");
        inventory.restockProduct(id, amount);
    }

    void sellProduct() {
        cout << "=== SELL PRODUCT ===\n";
        int id = InputHandler::getIntInput("Enter product ID: ");
        int amount = InputHandler::getIntInput("Enter quantity to sell: ");
        Product sold;
        if (inventory.sellProduct(id, amount, sold)) {
            receipt.addItem(sold.name, amount, sold.price);
        }
    }

    void showInventory() {
        inventory.showInventory();
    }

    void exportInventory() {
        string filename = "inventory_" + TimeTools::now_timestamp() + ".txt";
        if (inventory.printToFile(filename)) {
            cout << "Inventory exported to " << filename << " :D\n";
        } else {
            cout << " X Failed to export inventory.\n";
        }
    }

    void exportReceipt() {
        if (receipt.isEmpty()) {
            cout << " X No items in receipt to export.\n";
            return;
        }
        string filename = "receipt_" + TimeTools::now_timestamp() + ".txt";
        if (receipt.printToFile(filename)) {
            cout << "Receipt exported to " << filename << " :D\n";
        } else {
            cout << " X Failed to export receipt.\n";
        }
    }
};

class InventoryManagerMenu : public MainMenu {
public:
    InventoryManagerMenu(Inventory& inv, Reciept& rec) : MainMenu(inv, rec) {}

    void show() override {
        while (true) {
            ScreenManager::clearScreen();
            cout << "\n=== INVENTORY MANAGER MENU ===\n";
            cout << "1. Insert Product\n2. Delete Product\n3. Restock\n";
            cout << "4. Show Inventory\n5. Export Inventory\n6. Back\n";
            cout << "Choice: ";

            int choice = InputHandler::getIntInput("");

            if (choice == 6) break;

            processChoice(choice);
        }
    }

private:
    void processChoice(int choice) {
        ScreenManager::clearScreen();
        switch (choice) {
            case 1: insertProduct(); break;
            case 2: deleteProduct(); break;
            case 3: restockProduct(); break;
            case 4: showInventory(); break;
            case 5: exportInventory(); break;
            default: cout << "⚠️ Invalid choice.\n"; break;
        }
        ScreenManager::pauseForUser();
    }

    void insertProduct() {
        cout << "=== INSERT PRODUCT ===\n";
        Product p = InputHandler::getProductInput();
        inventory.insertProduct(p);
    }

    void deleteProduct() {
        cout << "=== DELETE PRODUCT ===\n";
        int id = InputHandler::getIntInput("Enter product ID to delete: ");
        inventory.deleteProduct(id);
    }

    void restockProduct() {
        cout << "=== RESTOCK PRODUCT ===\n";
        int id = InputHandler::getIntInput("Enter product ID: ");
        int amount = InputHandler::getIntInput("Enter amount to restock: ");
        inventory.restockProduct(id, amount);
    }

    void showInventory() {
        inventory.showInventory();
    }

    void exportInventory() {
        string filename = "inventory_" + TimeTools::now_timestamp() + ".txt";
        if (inventory.printToFile(filename)) {
            cout << "Inventory exported to " << filename << " :D\n";
        } else {
            cout << " X Failed to export inventory.\n";
        }
    }
};

class CashierMenu : public MainMenu {
public:
    CashierMenu(Inventory& inv, Reciept& rec) : MainMenu(inv, rec) {}

    void show() override {
        while (true) {
            ScreenManager::clearScreen();
            cout << "\n=== CASHIER MENU ===\n";
            cout << "1. Sell Product\n2. Show Inventory\n3. Export Receipt\n4. Back\n";
            cout << "Choice: ";

            int choice = InputHandler::getIntInput("");

            if (choice == 4) break;

            processChoice(choice);
        }
    }

private:
    void processChoice(int choice) {
        ScreenManager::clearScreen();
        switch (choice) {
            case 1: sellProduct(); break;
            case 2: showInventory(); break;
            case 3: exportReceipt(); break;
            default: cout << "X Invalid choice.\n"; break;
        }
        ScreenManager::pauseForUser();
    }

    void sellProduct() {
        cout << "=== SELL PRODUCT ===\n";
        int id = InputHandler::getIntInput("Enter product ID: ");
        int amount = InputHandler::getIntInput("Enter quantity to sell: ");
        Product sold;
        if (inventory.sellProduct(id, amount, sold)) {
            receipt.addItem(sold.name, amount, sold.price);
        }
    }

    void showInventory() {
        inventory.showInventory();
    }

    void exportReceipt() {
        if (receipt.isEmpty()) {
            cout << "X No items in receipt to export.\n";
            return;
        }
        string filename = "receipt_" + TimeTools::now_timestamp() + ".txt";
        if (receipt.printToFile(filename)) {
            cout << " Receipt exported to " << filename << " :D\n";
        }
        else {
            cout << " X Failed to export receipt.\n";
        }
    }
};

// --------------------------------------APP
class SupermarketApp {
private:
    Inventory inventory;
    Reciept receipt;

public:
    void run() {
        while (true) {
            ScreenManager::clearScreen();
            showMainMenu();

            int role = InputHandler::getIntInput("Enter choice: ");

            if (role == 4) {
                cout << "Goodbye! :D\n";
                break;
            }

            handleRoleSelection(role);
        }
    }

private:
    void showMainMenu() {
        cout << "==============================\n";
        cout << "   SUPERMARKET LOGIN MENU\n";
        cout << "==============================\n";
        cout << "1. Admin\n";
        cout << "2. Inventory Manager\n";
        cout << "3. Cashier\n";
        cout << "4. Exit\n";
        cout << "------------------------------\n";
    }

    void handleRoleSelection(int role) {
        unique_ptr<MainMenu> menu;

        switch (role) {
            case 1:
                menu = make_unique<AdminMenu>(inventory, receipt);
                break;
            case 2:
                menu = make_unique<InventoryManagerMenu>(inventory, receipt);
                break;
            case 3:
                menu = make_unique<CashierMenu>(inventory, receipt);
                break;
            default:
                cout << " X Invalid choice.\n";
                ScreenManager::pauseForUser();
                return;
        }

        menu->show();
    }
};

// --------------------------------------Main
int main() {
    SupermarketApp app;
    app.run();
    return 0;

}
//APP MADE BY : MAZEN THABET :)