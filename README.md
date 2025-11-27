# Spaazm Flights - Flight Reservation System

A modern C++ flight reservation system with Qt6 GUI, featuring persistent SQLite database, dynamic pricing, real-time seat selection, and comprehensive booking management.

## 📋 Table of Contents

- [Features](#-features)
- [Architecture Overview](#️-architecture-overview)
- [System Design](#-system-design)
- [Database Schema](#-database-schema)
- [OOP Design Patterns](#-oop-design-patterns)
- [Technologies Used](#️-technologies-used)
- [Installation](#-installation)
- [Usage Flow](#-usage-flow)
- [Code Structure](#-code-structure)
- [Dynamic Pricing Formula](#-dynamic-pricing-formula)
- [Database Operations](#️-database-operations)
- [Learning Outcomes](#-learning-outcomes)
- [Troubleshooting](#-troubleshooting)
- [Future Enhancements](#-future-enhancements)
- [License](#-license)
- [Author](#-author)

---

## ✨ Features

### Core Functionality
- ✅ **Persistent Flight Database**: 13,500 flights across 90 routes and 30 days
- ✅ **10 Major Indian Cities**: Mumbai, Delhi, Bangalore, Chennai, Kolkata, Hyderabad, Pune, Goa, Jaipur, Kochi
- ✅ **Complete Route Coverage**: All possible city pair combinations
- ✅ **Real-time Seat Availability**: Instant updates across all sessions
- ✅ **Passenger Information Management**: Name, email, and phone validation
- ✅ **Booking History**: Complete record of all reservations

### Dynamic Pricing Algorithm
Prices adjust based on multiple factors:
- **Seat Class**: Economy (1x) | Business (2x) | First (3x)
- **Demand**: +0% to +50% based on occupancy
- **Advance Booking**: Up to -50% discount (early) or +15% premium (last-minute)
- **Time of Day**: -10% (morning/night) to +30% (peak hours)

### User Interface
- **Modern Qt6 Design**: Clean, responsive interface with custom styling
- **Interactive Seat Map**: Visual representation with 100 seats per flight
- **Input Validation**: Email format and required field checks
- **Confirmation Dialogs**: Clear feedback for all operations

---

## 🏗️ Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                     PRESENTATION LAYER                      │
│                        (Qt6 Widgets)                        │
├─────────────────────────────────────────────────────────────┤
│  MainWindow  │  FlightCard  │  SeatButton  │  Dialogs     │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                      BUSINESS LOGIC                         │
│                   (ReservationSystem)                       │
├─────────────────────────────────────────────────────────────┤
│  Flight Search  │  Booking Management  │  Pricing Engine  │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                    DATA ACCESS LAYER                        │
│                      (SQLite3)                              │
├─────────────────────────────────────────────────────────────┤
│  flights  │  bookings  │  booked_seats                     │
└─────────────────────────────────────────────────────────────┘
```

### Three-Tier Architecture

1. **Presentation Layer** (GUI)
   - Qt6 Widgets for user interface
   - Event handling and user input validation
   - Visual feedback and error messages

2. **Business Logic Layer** (Backend)
   - Flight search and filtering
   - Seat availability management
   - Dynamic pricing calculations
   - Booking operations

3. **Data Layer** (Database)
   - SQLite3 for persistent storage
   - Flight schedules (permanent)
   - Booking records
   - Seat occupancy tracking

---

## 🎯 System Design

### Application Flow Diagram

```
┌───────────┐
│   START   │
└─────┬─────┘
      │
      ▼
┌─────────────────┐
│  Launch App     │──────► Initialize Database
│  Initialize DB  │        (if first run)
└────────┬────────┘
         │
         ▼
┌─────────────────────────────────────────┐
│         MAIN WINDOW                     │
│  ┌────────────┐  ┌──────────────┐      │
│  │   Search   │  │   Bookings   │      │
│  │   Flights  │  │   History    │      │
│  └─────┬──────┘  └──────┬───────┘      │
└────────┼─────────────────┼──────────────┘
         │                 │
    ┌────▼────┐       ┌────▼────┐
    │ Select  │       │  View   │
    │ Date &  │       │  Cancel │
    │ Route   │       │ Bookings│
    └────┬────┘       └─────────┘
         │
         ▼
    ┌────────────┐
    │  Display   │
    │  Available │
    │  Flights   │
    └─────┬──────┘
          │
          ▼
    ┌─────────────────┐
    │  Select Flight  │
    └────────┬────────┘
             │
             ▼
    ┌──────────────────────────────┐
    │   BOOKING DIALOG             │
    │  1. Enter Name/Email/Phone   │
    │  2. Select Class             │
    │  3. Choose Seat              │
    │  4. View Dynamic Price       │
    │  5. Confirm Booking          │
    └──────────┬───────────────────┘
               │
               ▼
    ┌──────────────────┐
    │  Save to DB      │
    │  Update Seats    │
    └─────────┬────────┘
              │
              ▼
    ┌─────────────────┐
    │  Confirmation   │
    │    Message      │
    └─────────────────┘
```

### Database Interaction Flow

```
┌──────────────┐     searchFlights()      ┌──────────────┐
│  User Input  │────────────────────────►│ Reservation  │
│ Date, Route  │                          │   System     │
└──────────────┘                          └──────┬───────┘
                                                 │
                                                 │ SQL Query
                                                 ▼
                                          ┌─────────────┐
                                          │   SQLite    │
                                          │  Database   │
                                          └──────┬──────┘
                                                 │
                        ┌────────────────────────┴────────┐
                        │                                 │
                   ┌────▼────┐                      ┌─────▼──────┐
                   │ flights │                      │booked_seats│
                   │  table  │                      │   table    │
                   └────┬────┘                      └─────┬──────┘
                        │                                 │
                        └────────────────┬────────────────┘
                                         │
                                         ▼
                              ┌──────────────────┐
                              │ Flight Objects   │
                              │ with Seat Status │
                              └────────┬─────────┘
                                       │
                                       ▼
                              ┌──────────────────┐
                              │  Display in GUI  │
                              └──────────────────┘
```

---

## 💾 Database Schema

```sql
┌─────────────────────────────────────────────────────────────┐
│                      flights TABLE                          │
├──────────────────┬──────────────────────────────────────────┤
│ flight_number    │ TEXT    (e.g., "SP1001")                │
│ flight_name      │ TEXT    (e.g., "Sky Express")           │
│ source           │ TEXT    (e.g., "Mumbai")                │
│ destination      │ TEXT    (e.g., "Delhi")                 │
│ date             │ TEXT    (YYYY-MM-DD)                    │
│ departure_time   │ TEXT    (YYYY-MM-DD HH:MM)              │
│ base_price       │ REAL    (Base fare in INR)              │
├──────────────────┴──────────────────────────────────────────┤
│ PRIMARY KEY: (flight_number, date)                          │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                     bookings TABLE                          │
├──────────────────┬──────────────────────────────────────────┤
│ id               │ INTEGER PRIMARY KEY                     │
│ passenger_name   │ TEXT                                    │
│ passenger_email  │ TEXT                                    │
│ passenger_phone  │ TEXT                                    │
│ flight_number    │ TEXT                                    │
│ flight_date      │ TEXT                                    │
│ seat_number      │ INTEGER (1-100)                         │
│ seat_class       │ TEXT    (Economy/Business/First)        │
│ price            │ REAL    (Final price paid)              │
│ booking_time     │ INTEGER (UNIX timestamp)                │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                   booked_seats TABLE                        │
├──────────────────┬──────────────────────────────────────────┤
│ flight_number    │ TEXT                                    │
│ flight_date      │ TEXT                                    │
│ seat_number      │ INTEGER                                 │
│ passenger_name   │ TEXT                                    │
├──────────────────┴──────────────────────────────────────────┤
│ PRIMARY KEY: (flight_number, flight_date, seat_number)      │
└─────────────────────────────────────────────────────────────┘
```

### Seat Distribution (100 seats per flight)
```
First Class:    Seats  1-10   (10 seats)  ██████████
Business Class: Seats 11-30   (20 seats)  ████████████████████
Economy Class:  Seats 31-100  (70 seats)  ██████████████████████████████████████████
```

---

## 🎨 OOP Design Patterns

### 1. **Encapsulation**
```cpp
class Seat {
private:
    int seatNumber;
    string seatClass;
    bool isBooked;
    string passengerName;
public:
    // Controlled access through getters
    int getSeatNumber() const;
    bool getIsBooked() const;
    // Controlled modification through methods
    void bookSeat(string name);
    void cancelBooking();
};
```

### 2. **Composition**
```
Flight HAS-A vector<Seat*>
  │
  ├─► Seat 1 (First Class)
  ├─► Seat 2 (First Class)
  ├─► ...
  └─► Seat 100 (Economy)

ReservationSystem HAS-A
  ├─► vector<Flight*>
  └─► vector<Booking*>
```

### 3. **Separation of Concerns**
- **flight_system.h/cpp**: Pure business logic (no GUI dependencies)
- **main_gui.cpp**: Qt GUI implementation (uses business logic)
- Clear interface between layers

### 4. **Single Responsibility Principle**
- `Seat`: Manages individual seat state
- `Flight`: Manages flight details and 100 seats
- `Booking`: Stores booking information
- `ReservationSystem`: Coordinates operations and database

---

## 🛠️ Technologies Used

### Programming Language
- **C++17**: Modern C++ with STL containers, lambda expressions, auto type deduction

### GUI Framework
- **Qt6 Widgets**: Cross-platform GUI framework
  - QMainWindow, QDialog, QLabel, QPushButton
  - QComboBox, QLineEdit, QDateEdit
  - QScrollArea, QGridLayout, QVBoxLayout
  - Custom widgets (FlightCard, SeatButton)

### Database
- **SQLite3**: Lightweight embedded database
  - Zero-configuration
  - File-based storage
  - ACID transactions
  - SQL query support

### Build System
- **CMake 3.16+**: Cross-platform build configuration
  - Automatic MOC (Meta-Object Compiler) for Qt
  - Dependency management
  - Build artifact organization

### Development Tools
- **Git**: Version control
- **g++**: GNU C++ compiler with C++17 support
- **Qt Designer** concepts: Programmatic UI creation

---

## 📦 Installation

### Prerequisites

#### Linux

**Arch Linux**
```bash
sudo pacman -S qt6-base cmake gcc sqlite
```

**Ubuntu/Debian**
```bash
sudo apt update
sudo apt install qt6-base-dev cmake build-essential libsqlite3-dev
```

**Fedora**
```bash
sudo dnf install qt6-qtbase-devel cmake gcc-c++ sqlite-devel
```

#### Windows

**Option 1: Using MSYS2 (Recommended)**

1. **Install MSYS2** from https://www.msys2.org/
2. **Open MSYS2 MinGW 64-bit terminal** and run:
```bash
pacman -Syu
pacman -S mingw-w64-x86_64-qt6-base mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc mingw-w64-x86_64-sqlite3
```

**Option 2: Using Qt Installer**

1. **Download Qt Online Installer** from https://www.qt.io/download-qt-installer
2. **Install Qt 6.x** with following components:
   - Qt 6.x for MinGW or MSVC
   - CMake
   - MinGW compiler (or use Visual Studio)
3. **Install SQLite3**:
   - Download precompiled binaries from https://www.sqlite.org/download.html
   - Extract to `C:\sqlite3`
   - Add to PATH: `C:\sqlite3`

#### macOS

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install qt@6 cmake sqlite3
```

---

### Build Instructions

#### Linux / macOS

```bash
# Clone repository
git clone <repository-url>
cd spaazm

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build .

# Run application
./bin/FlightReservation
```

#### Windows (MSYS2)

```bash
# Clone repository (in MSYS2 MinGW 64-bit terminal)
git clone <repository-url>
cd spaazm

# Create build directory
mkdir build
cd build

# Configure and build
cmake .. -G "MinGW Makefiles"
cmake --build .

# Run application
./bin/FlightReservation.exe
```

#### Windows (Visual Studio)

```powershell
# Clone repository (in Command Prompt or PowerShell)
git clone <repository-url>
cd spaazm

# Create build directory
mkdir build
cd build

# Configure (replace paths with your Qt installation)
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64"

# Build
cmake --build . --config Release

# Run application
.\bin\Release\FlightReservation.exe
```

#### Windows (Qt Creator IDE)

1. **Open Qt Creator**
2. **File → Open File or Project**
3. **Select** `CMakeLists.txt` from the spaazm folder
4. **Configure Project** with your Qt kit
5. **Build** (Ctrl+B)
6. **Run** (Ctrl+R)

---

### First Run

On first launch, the application will:
1. Create `spaazm_flights.db` in the build directory (or current working directory)
2. Populate with 5,400 flights (36 routes × 5 flights × 30 days)
3. This takes ~2 seconds and only happens once

**Note for Windows**: The database will be created where the executable is run from. For best results, run from the build directory.

---

## 🚀 Usage Flow

### Booking a Flight

```
Step 1: Select Travel Date
   │
   ▼
Step 2: Choose Source City (dropdown with 10 cities)
   │
   ▼
Step 3: Choose Destination City
   │
   ▼
Step 4: Click "Search Flights"
   │
   ▼
Step 5: View Available Flights
   ├─ Flight details (number, name, route, time)
   ├─ Base price displayed
   └─ Click "Book Now" on desired flight
       │
       ▼
Step 6: Booking Dialog Opens
   ├─ Enter passenger name
   ├─ Enter email address (validated)
   ├─ Enter phone number
   ├─ Select class (Economy/Business/First)
   ├─ Choose seat from visual map
   └─ See dynamic price update in real-time
       │
       ▼
Step 7: Confirm Booking
   ├─ Validation checks all fields
   ├─ Saves to database
   ├─ Updates seat availability
   └─ Shows confirmation dialog
```

### Managing Bookings

```
View Bookings Tab
   │
   ├─► List of all bookings with details
   │   ├─ Booking ID
   │   ├─ Passenger name
   │   ├─ Flight details
   │   ├─ Seat information
   │   └─ Amount paid
   │
   └─► Cancel Booking
       ├─ Select booking from list
       ├─ Click "Cancel Booking"
       ├─ 10% cancellation fee applied
       └─ Seat becomes available again
```

---

## 📁 Code Structure

```
spaazm/
├── CMakeLists.txt              # Build configuration
├── README.md                   # This file
├── flight_system.h             # Backend class declarations
├── flight_system.cpp           # Backend implementation + SQLite
├── main_gui.cpp                # Qt GUI implementation
└── build/
    ├── bin/
    │   └── FlightReservation   # Executable
    └── spaazm_flights.db       # Database (auto-generated)
```

### Class Overview

#### Backend Classes (flight_system.h/cpp)

**`class Seat`**
- **Purpose**: Represents a single seat on a flight
- **Attributes**: 
  - `seatNumber` (1-100)
  - `seatClass` (First/Business/Economy)
  - `isBooked` (availability status)
  - `passengerName` (if booked)
- **Methods**:
  - `bookSeat(name)`: Marks seat as booked
  - `cancelBooking()`: Frees up the seat

**`class Flight`**
- **Purpose**: Manages a complete flight with 100 seats
- **Attributes**:
  - Flight details (number, name, route, time)
  - `basePrice` (starting fare)
  - `vector<Seat*> seats` (100 seat objects)
  - `departureTimestamp` (for pricing calculations)
- **Key Methods**:
  - `calculatePrice(seatClass, bookingTime)`: Dynamic pricing algorithm
  - `bookSeat(seatNumber, name)`: Books a specific seat
  - `getAvailableSeatsByClass(class)`: Filters available seats
  - `getBookedSeatsCount()`: Returns occupancy for demand pricing

**`class Booking`**
- **Purpose**: Records a confirmed reservation
- **Attributes**:
  - Unique `bookingId` (auto-incremented from 1001)
  - Passenger info (name, email, phone)
  - Flight details (number, date)
  - Seat info (number, class)
  - `price` (amount paid)
  - `bookingTime` (timestamp)
- **Static**: `bookingCounter` for unique IDs

**`class ReservationSystem`**
- **Purpose**: Main controller coordinating all operations
- **Attributes**:
  - `vector<Flight*> flights` (currently loaded)
  - `vector<Booking*> bookings` (in-memory cache)
  - `sqlite3* db` (database connection)
- **Key Methods**:
  - `initDatabase()`: Creates tables on first run
  - `populateFlights()`: Generates 30 days of flights
  - `searchFlights(date, source, dest)`: Queries database
  - `loadBookedSeats(flight)`: Restores seat status
  - `addBooking(...)`: Creates and persists booking
  - `cancelBooking(id)`: Removes booking and frees seat

#### GUI Classes (main_gui.cpp)

**`class FlightCard : public QFrame`**
- **Purpose**: Custom widget to display flight information
- **Features**:
  - Styled card layout with hover effects
  - Shows flight number, name, route, time, price
  - "Book Now" button
  - Custom CSS styling

**`class SeatButton : public QPushButton`**
- **Purpose**: Interactive seat selection button
- **Features**:
  - 50×50px button with seat number
  - Color-coded by availability:
    - 🟢 Green: Available
    - 🔴 Red: Already booked
    - 🔵 Blue: Currently selected
  - Emits `seatSelected` signal on click
  - Hover effects

**`class MainWindow : public QMainWindow`**
- **Purpose**: Main application window
- **Components**:
  - Tab widget (Search Flights / My Bookings)
  - Search form (date, source, destination dropdowns)
  - Flight results area with FlightCard widgets
  - Booking list with cancel functionality
- **Key Methods**:
  - `initUI()`: Sets up the interface
  - `searchFlights()`: Handles search button click
  - `showBookingDialog(flight)`: Opens booking form
  - `updateBookingsList()`: Refreshes booking tab

---

## 🧮 Dynamic Pricing Formula

```cpp
finalPrice = basePrice × classMultiplier × demandFactor 
             × advanceBookingFactor × timeOfDayFactor
```

### Factors Breakdown

**1. Class Multiplier**
```
Economy:  1.0x  (base price)
Business: 2.0x  (double)
First:    3.0x  (triple)
```

**2. Demand Factor** (based on occupancy)
```
occupancyRate = bookedSeats / totalSeats
demandFactor = 1.0 + (occupancyRate × 0.5)

Examples:
- 0% full   → 1.0x (no change)
- 50% full  → 1.25x (+25%)
- 100% full → 1.5x (+50%)
```

**3. Advance Booking Factor** (days until departure)
```
daysUntilFlight = (departureTime - bookingTime) / 86400

if (days > 30)    → 0.50x  (-50% early bird)
if (days > 14)    → 0.70x  (-30% discount)
if (days > 7)     → 0.85x  (-15% discount)
if (days > 3)     → 1.0x   (regular price)
if (days <= 3)    → 1.15x  (+15% last-minute)
```

**4. Time of Day Factor** (departure hour)
```
Hour     | Factor | Description
---------|--------|-------------
00-05    | 0.90x  | Late night (-10%)
06-08    | 1.00x  | Morning
09-11    | 1.20x  | Peak morning (+20%)
12-14    | 1.10x  | Afternoon (+10%)
15-17    | 1.30x  | Peak evening (+30%)
18-20    | 1.15x  | Evening (+15%)
21-23    | 0.95x  | Night (-5%)
```

### Example Calculation

```
Base Price: ₹5,000 (Mumbai → Delhi)
Class: Business (2.0x)
Occupancy: 60% (1.3x)
Booked 10 days ahead: (0.85x)
Departure: 15:00 (1.30x)

Final Price = 5000 × 2.0 × 1.3 × 0.85 × 1.30
           = ₹14,365
```

---

## 🗄️ Database Operations

### Initialization Flow
```
App Launch
    │
    ├─► Open/Create spaazm_flights.db
    │
    ├─► Check if flights table exists
    │   │
    │   ├─► NO  → Create all tables
    │   │         └─► Run populateFlights()
    │   │
    │   └─► YES → Check row count
    │             │
    │             ├─► 0 rows  → Run populateFlights()
    │             └─► >0 rows → Skip (already populated)
    │
    └─► Ready for use
```

### Flight Search Query
```sql
SELECT flight_number, flight_name, source, destination, 
       departure_time, base_price
FROM flights
WHERE date = '2025-11-26'
  AND source = 'Mumbai'
  AND destination = 'Delhi';
```

### Booking Insert
```sql
-- Insert booking record
INSERT INTO bookings VALUES (
    1001, 'John Doe', 'john@email.com', '+91 9876543210',
    'SP1001', '2025-11-26', 42, 'Business', 14365.50, 1732607400
);

-- Mark seat as occupied
INSERT INTO booked_seats VALUES (
    'SP1001', '2025-11-26', 42, 'John Doe'
);
```

### Load Booked Seats
```sql
SELECT seat_number, passenger_name
FROM booked_seats
WHERE flight_number = 'SP1001'
  AND flight_date = '2025-11-26';
```

---

## 🎓 Learning Outcomes

This project demonstrates:

### OOP Concepts
- ✅ **Encapsulation**: Private data with public interfaces
- ✅ **Composition**: Objects containing other objects
- ✅ **Abstraction**: Complex operations hidden behind simple methods
- ✅ **Single Responsibility**: Each class has one clear purpose

### Software Engineering
- ✅ **Separation of Concerns**: GUI separate from business logic
- ✅ **Layered Architecture**: Presentation → Logic → Data
- ✅ **Database Integration**: Persistent storage with SQLite
- ✅ **Input Validation**: Email format, required fields
- ✅ **Error Handling**: Database connection checks, SQL error handling

### Advanced C++ Features
- ✅ STL Containers: `vector`, `string`, `stringstream`
- ✅ Smart Memory Management: Manual `new`/`delete` with cleanup
- ✅ Lambda Functions: Inline event handlers
- ✅ Time Manipulation: `time_t`, `struct tm`, `mktime`, `localtime`
- ✅ String Formatting: `stringstream`, `setw`, `setfill`

### Qt Framework
- ✅ Widget Hierarchy: Windows, dialogs, layouts
- ✅ Signal-Slot Mechanism: Event-driven programming
- ✅ Custom Widgets: `FlightCard`, `SeatButton`
- ✅ Styling: CSS-like stylesheets
- ✅ Dynamic UI Updates: Real-time price changes

---

## 🐛 Troubleshooting

### Build Issues

**Qt6 not found**
```bash
# Ensure Qt6 is installed and CMake can find it
export CMAKE_PREFIX_PATH=/usr/lib/qt6

# Or specify in cmake command
cmake -DCMAKE_PREFIX_PATH=/usr/lib/qt6 ..
```

**SQLite3 not found**
```bash
# Install SQLite development files
sudo apt install libsqlite3-dev  # Debian/Ubuntu
sudo pacman -S sqlite            # Arch Linux
```

### Runtime Issues

**Database not created**
- Ensure write permissions in build directory
- Database is created in current working directory when app runs
- Run from build directory: `cd build && ./bin/FlightReservation`
- **Windows**: Run from MSYS2 terminal to see debug output

**No cities in dropdown**
- **Fixed in latest version**: Dropdowns now use fallback city list if database fails
- If still empty: Delete `spaazm_flights.db` and restart
- Check console output for error messages
- Database will repopulate automatically

**No flights generated (Windows MSYS2)**
- **Fixed in latest version**: Added comprehensive error handling
- Console now shows database initialization progress
- Run from terminal to see debug messages:
  - "Database opened successfully"
  - "Tables created successfully"
  - "Successfully populated X flights"
- If no output, check file permissions in build directory

**Seats not updating**
- Booking saves to database immediately
- Search again to reload flight with updated seats

**Windows-specific issues**
- See `WINDOWS_TESTING.md` for detailed Windows troubleshooting
- Ensure SQLite3 DLL is in PATH or same directory as executable
- Run from MSYS2 terminal for proper console output

---

## 📈 Future Enhancements

Potential features for expansion:
- [ ] User authentication system
- [ ] Payment gateway integration
- [ ] PDF ticket generation
- [ ] Email confirmation sending
- [ ] Multi-language support
- [ ] Flight status updates
- [ ] Loyalty program points
- [ ] Refund processing
- [ ] Admin dashboard
- [ ] Analytics and reporting

---

## 📝 License

This project is created for educational purposes as part of an Object-Oriented Programming course.

---

## 👨‍💻 Author

Built with ❤️ using modern C++, Qt6, and SQLite3

**Spaazm Flights** - Your journey, our priority! ✈️
```

### macOS
```bash
# Install Qt6 using Homebrew
brew install qt@6 cmake

# Build the project
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6) ..
cmake --build .

# Run the application
./bin/FlightReservation
```

### Windows
```bash
# Install Qt6 from https://www.qt.io/download
# Add Qt to PATH or use Qt Creator

# Build with CMake
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64" ..
cmake --build . --config Release

# Run the application
.\bin\Release\FlightReservation.exe
```

## Usage

### Booking a Flight
1. Navigate to the "Flights" page
2. Browse available flights with live pricing
3. Click "Book Flight" on your desired flight
4. Enter passenger name
5. Select seat class (Economy/Business/First)
6. Choose your seat from the visual seat map
7. Review the dynamic price and confirm

### Managing Bookings
1. Navigate to "My Bookings"
2. View all your confirmed reservations
3. Cancel bookings if needed (10% cancellation fee applies)

## Dynamic Pricing Algorithm

The system calculates prices using:
- **Base Price**: Starting price for economy class
- **Class Multiplier**: 
  - Economy: 1x
  - Business: 2x
  - First: 3x
- **Demand Factor**: Up to 50% increase based on seat occupancy
- **Time Factor**:
  - 30+ days early: 15% discount
  - 7-30 days: Standard price
  - 3-7 days: 15% increase
  - 1-3 days: 30% increase
  - < 1 day: 50% increase

## Project Structure

```
spaazm/
├── main.cpp              # Complete application (backend + GUI)
├── flight_reservation.cpp # Console-only version (reference)
├── CMakeLists.txt        # Build configuration
└── README.md            # This file
```

## OOP Concepts Demonstrated

- **Encapsulation**: Private data members with public getters/setters
- **Inheritance**: Qt widget inheritance for custom UI components
- **Polymorphism**: Virtual functions in Qt framework
- **Abstraction**: Clean interfaces between backend and GUI
- **Composition**: Complex objects built from simpler ones

## License

MIT License - Free for educational use

## Author

Created for OOP course project
