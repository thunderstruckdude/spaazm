#include "flight_system.h"
#include <sqlite3.h>
#include <iostream>

using namespace std;

// ==================== SEAT IMPLEMENTATION ====================

Seat::Seat(int number, string sClass) : seatNumber(number), seatClass(sClass), isBooked(false), passengerName("") {}

void Seat::bookSeat(string name) {
    isBooked = true;
    passengerName = name;
}

void Seat::cancelBooking() {
    isBooked = false;
    passengerName = "";
}

// ==================== FLIGHT IMPLEMENTATION ====================

void Flight::initializeSeats() {
    for (int i = 1; i <= 10; i++) {
        seats.push_back(new Seat(i, "First"));
    }
    for (int i = 11; i <= 30; i++) {
        seats.push_back(new Seat(i, "Business"));
    }
    for (int i = 31; i <= 100; i++) {
        seats.push_back(new Seat(i, "Economy"));
    }
}

Flight::Flight(string fNumber, string fName, string src, string dest, string depTime, double price, time_t depTimestamp)
    : flightNumber(fNumber), flightName(fName), source(src), destination(dest), 
      departureTime(depTime), basePrice(price), totalSeats(100), departureTimestamp(depTimestamp) {
    initializeSeats();
}

Flight::~Flight() {
    for (auto seat : seats) {
        delete seat;
    }
}

double Flight::calculatePrice(string seatClass, time_t bookingTime) const {
    double price = basePrice;
    
    // Class multiplier
    if (seatClass == "First") {
        price *= 3.0;
    } else if (seatClass == "Business") {
        price *= 2.0;
    }
    
    // Demand-based pricing
    int bookedSeats = getBookedSeatsCount();
    double occupancyRate = (double)bookedSeats / totalSeats;
    price *= (1.0 + occupancyRate * 0.5);
    
    // Days until departure pricing
    double daysUntilDeparture = difftime(departureTimestamp, bookingTime) / (60 * 60 * 24);
    if (daysUntilDeparture < 1) {
        price *= 1.5;
    } else if (daysUntilDeparture < 3) {
        price *= 1.3;
    } else if (daysUntilDeparture < 7) {
        price *= 1.15;
    } else if (daysUntilDeparture > 30) {
        price *= 0.85;
    }
    
    // Time-of-day pricing (extract hour from departure time)
    struct tm* timeinfo = localtime(&departureTimestamp);
    int hour = timeinfo->tm_hour;
    
    if (hour >= 6 && hour < 9) {
        // Early morning peak (6 AM - 9 AM)
        price *= 1.25;
    } else if (hour >= 9 && hour < 12) {
        // Mid-morning (9 AM - 12 PM)
        price *= 1.10;
    } else if (hour >= 12 && hour < 15) {
        // Afternoon off-peak (12 PM - 3 PM)
        price *= 0.95;
    } else if (hour >= 15 && hour < 18) {
        // Late afternoon (3 PM - 6 PM)
        price *= 1.05;
    } else if (hour >= 18 && hour < 21) {
        // Evening peak (6 PM - 9 PM)
        price *= 1.30;
    } else {
        // Night/Late night (9 PM - 6 AM)
        price *= 0.90;
    }
    
    return price;
}

int Flight::getBookedSeatsCount() const {
    int count = 0;
    for (const auto& seat : seats) {
        if (seat->getIsBooked()) count++;
    }
    return count;
}

int Flight::getAvailableSeatsCount() const {
    return totalSeats - getBookedSeatsCount();
}

vector<Seat*> Flight::getAvailableSeatsByClass(string seatClass) const {
    vector<Seat*> available;
    for (auto seat : seats) {
        if (!seat->getIsBooked() && seat->getSeatClass() == seatClass) {
            available.push_back(seat);
        }
    }
    return available;
}

Seat* Flight::getSeatByNumber(int seatNumber) {
    if (seatNumber >= 1 && seatNumber <= totalSeats) {
        return seats[seatNumber - 1];
    }
    return nullptr;
}

bool Flight::bookSeat(int seatNumber, string passengerName) {
    Seat* seat = getSeatByNumber(seatNumber);
    if (seat && !seat->getIsBooked()) {
        seat->bookSeat(passengerName);
        return true;
    }
    return false;
}

bool Flight::cancelSeat(int seatNumber) {
    Seat* seat = getSeatByNumber(seatNumber);
    if (seat && seat->getIsBooked()) {
        seat->cancelBooking();
        return true;
    }
    return false;
}

// ==================== BOOKING IMPLEMENTATION ====================

int Booking::bookingCounter = 1000;

Booking::Booking(string name, string mail, string ph, string fNumber, string fDate, int seat, double p, string sClass)
    : passengerName(name), email(mail), phone(ph), flightNumber(fNumber), flightDate(fDate), seatNumber(seat), price(p), seatClass(sClass) {
    bookingId = ++bookingCounter;
    bookingTime = time(nullptr);
}

// ==================== RESERVATION SYSTEM IMPLEMENTATION ====================

ReservationSystem::ReservationSystem() : db(nullptr) {
    initDatabase();
    loadFlights();
}

ReservationSystem::~ReservationSystem() {
    for (auto flight : flights) delete flight;
    for (auto booking : bookings) delete booking;
    if (db) sqlite3_close(db);
}

void ReservationSystem::searchFlights(const string& dateStr, const string& source, const string& destination) {
    clearFlights();
    
    if (!db) return;
    
    stringstream ss;
    ss << "SELECT flight_number, flight_name, source, destination, departure_time, base_price "
       << "FROM flights WHERE date = '" << dateStr << "' "
       << "AND source = '" << source << "' AND destination = '" << destination << "';";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, ss.str().c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            string flightNum = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            string flightName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            string src = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            string dest = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            string depTime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            double basePrice = sqlite3_column_double(stmt, 5);
            
            struct tm tm = {};
            int y, mon, d, h, m;
            if (sscanf(depTime.c_str(), "%d-%d-%d %d:%d", &y, &mon, &d, &h, &m) == 5) {
                tm.tm_year = y - 1900;
                tm.tm_mon = mon - 1;
                tm.tm_mday = d;
                tm.tm_hour = h;
                tm.tm_min = m;
                tm.tm_isdst = -1;
                time_t timestamp = mktime(&tm);
                
                Flight* flight = new Flight(flightNum, flightName, src, dest, depTime, basePrice, timestamp);
                loadBookedSeats(flight);
                flights.push_back(flight);
            }
        }
        sqlite3_finalize(stmt);
    }
}

vector<string> ReservationSystem::getUniqueCities() const {
    vector<string> cities;
    if (!db) {
        // Fallback to hardcoded cities if database is not available
        return {"Mumbai", "Delhi", "Bangalore", "Chennai", "Kolkata",
                "Hyderabad", "Pune", "Goa", "Jaipur", "Kochi"};
    }
    
    sqlite3_stmt* stmt;
    const char* sql = "SELECT DISTINCT source FROM flights UNION SELECT DISTINCT destination FROM flights ORDER BY 1;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            cities.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        }
        sqlite3_finalize(stmt);
    }
    
    // If no cities found in database, return hardcoded list
    if (cities.empty()) {
        return {"Mumbai", "Delhi", "Bangalore", "Chennai", "Kolkata",
                "Hyderabad", "Pune", "Goa", "Jaipur", "Kochi"};
    }
    
    return cities;
}

void ReservationSystem::clearFlights() {
    for (auto flight : flights) {
        delete flight;
    }
    flights.clear();
}

Flight* ReservationSystem::findFlight(string flightNumber) {
    for (auto flight : flights) {
        if (flight->getFlightNumber() == flightNumber) {
            return flight;
        }
    }
    return nullptr;
}

Booking* ReservationSystem::addBooking(string passengerName, string email, string phone, string flightNumber, string flightDate, int seatNumber, double price, string seatClass) {
    Booking* booking = new Booking(passengerName, email, phone, flightNumber, flightDate, seatNumber, price, seatClass);
    bookings.push_back(booking);
    saveBooking(booking);
    return booking;
}

bool ReservationSystem::cancelBooking(int bookingId) {
    for (size_t i = 0; i < bookings.size(); i++) {
        if (bookings[i]->getBookingId() == bookingId) {
            Flight* flight = findFlight(bookings[i]->getFlightNumber());
            if (flight) {
                flight->cancelSeat(bookings[i]->getSeatNumber());
            }
            
            if (db) {
                string sql = "DELETE FROM bookings WHERE id = " + to_string(bookingId) + ";";
                sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
                
                stringstream ss;
                ss << "DELETE FROM booked_seats WHERE flight_number = '" 
                   << bookings[i]->getFlightNumber() << "' AND flight_date = '" 
                   << bookings[i]->getFlightDate() << "' AND seat_number = " 
                   << bookings[i]->getSeatNumber() << ";";
                sqlite3_exec(db, ss.str().c_str(), nullptr, nullptr, nullptr);
            }
            
            delete bookings[i];
            bookings.erase(bookings.begin() + i);
            return true;
        }
    }
    return false;
}

void ReservationSystem::initDatabase() {
    int rc = sqlite3_open("spaazm_flights.db", &db);
    if (rc) {
        cerr << "Failed to open database: " << sqlite3_errmsg(db) << endl;
        db = nullptr;
        return;
    }
    
    cout << "Database opened successfully" << endl;
    
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS flights ("
        "flight_number TEXT,"
        "flight_name TEXT,"
        "source TEXT,"
        "destination TEXT,"
        "date TEXT,"
        "departure_time TEXT,"
        "base_price REAL,"
        "PRIMARY KEY (flight_number, date));"
        
        "CREATE TABLE IF NOT EXISTS bookings ("
        "id INTEGER PRIMARY KEY,"
        "passenger_name TEXT,"
        "passenger_email TEXT,"
        "passenger_phone TEXT,"
        "flight_number TEXT,"
        "flight_date TEXT,"
        "seat_number INTEGER,"
        "seat_class TEXT,"
        "price REAL,"
        "booking_time INTEGER);"
        
        "CREATE TABLE IF NOT EXISTS booked_seats ("
        "flight_number TEXT,"
        "flight_date TEXT,"
        "seat_number INTEGER,"
        "passenger_name TEXT,"
        "PRIMARY KEY (flight_number, flight_date, seat_number));";
    
    char* errMsg = nullptr;
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
    } else {
        cout << "Tables created successfully" << endl;
    }
    
    populateFlights();
}

void ReservationSystem::populateFlights() {
    if (!db) {
        cerr << "Database not initialized, cannot populate flights" << endl;
        return;
    }
    
    // Check if already populated
    sqlite3_stmt* check;
    int rc = sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM flights;", -1, &check, nullptr);
    if (rc == SQLITE_OK) {
        if (sqlite3_step(check) == SQLITE_ROW) {
            int count = sqlite3_column_int(check, 0);
            if (count > 0) {
                cout << "Database already has " << count << " flights" << endl;
                sqlite3_finalize(check);
                return;
            }
        }
        sqlite3_finalize(check);
    }
    
    // All 10 cities
    vector<string> cities = {
        "Mumbai", "Delhi", "Bangalore", "Chennai", "Kolkata",
        "Hyderabad", "Pune", "Goa", "Jaipur", "Kochi"
    };
    
    // Generate ALL possible city pairs (both directions)
    // 10 cities × 9 other cities = 90 directional routes
    vector<pair<string, string>> routes;
    for (size_t i = 0; i < cities.size(); i++) {
        for (size_t j = 0; j < cities.size(); j++) {
            if (i != j) {  // Don't create routes from city to itself
                routes.push_back({cities[i], cities[j]});
            }
        }
    }
    
    string names[] = {"Sky Express", "Cloud Nine", "Wind Jet", "Star Flight", "Thunder Express"};
    string times[] = {"06:00", "10:00", "14:00", "18:00", "21:00"};
    
    // Generate flights for next 30 days
    time_t now = time(nullptr);
    
    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    
    int flightCounter = 1001;
    
    for (int day = 0; day < 30; day++) {
        time_t futureTime = now + (day * 86400);
        struct tm* tm = localtime(&futureTime);
        char dateStr[11];
        strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", tm);
        
        for (size_t r = 0; r < routes.size(); r++) {
            string source = routes[r].first;
            string destination = routes[r].second;
            
            // Generate base price based on distance (pseudo-calculation)
            int basePrice = 2500 + ((flightCounter * 37) % 4500);
            
            for (int i = 0; i < 5; i++) {
                stringstream flightNum;
                flightNum << "SP" << flightCounter;
                
                stringstream sql;
                sql << "INSERT OR IGNORE INTO flights VALUES ('"
                    << flightNum.str() << "','"
                    << names[i] << "','"
                    << source << "','"
                    << destination << "','"
                    << dateStr << "','"
                    << dateStr << " " << times[i] << "',"
                    << basePrice << ");";
                sqlite3_exec(db, sql.str().c_str(), nullptr, nullptr, nullptr);
                
                flightCounter++;
            }
        }
    }
    
    char* errMsg = nullptr;
    rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        cerr << "Failed to commit transaction: " << errMsg << endl;
        sqlite3_free(errMsg);
    } else {
        cout << "Successfully populated " << (flightCounter - 1001) << " flights" << endl;
        cout << "Routes: " << routes.size() << " (all city pairs)" << endl;
    }
}

void ReservationSystem::loadFlights() {
    // Flights loaded on-demand via searchFlights
}

void ReservationSystem::loadBookedSeats(Flight* flight) {
    if (!db) return;
    
    stringstream ss;
    ss << "SELECT seat_number, passenger_name FROM booked_seats "
       << "WHERE flight_number = '" << flight->getFlightNumber() << "' "
       << "AND flight_date = '" << flight->getDepartureTime().substr(0, 10) << "';";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, ss.str().c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int seatNum = sqlite3_column_int(stmt, 0);
            string passengerName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            flight->bookSeat(seatNum, passengerName);
        }
        sqlite3_finalize(stmt);
    }
}

void ReservationSystem::saveBooking(Booking* booking) {
    if (!db) return;
    
    stringstream ss;
    ss << "INSERT INTO bookings VALUES (" 
       << booking->getBookingId() << ",'"
       << booking->getPassengerName() << "','"
       << booking->getEmail() << "','"
       << booking->getPhone() << "','"
       << booking->getFlightNumber() << "','"
       << booking->getFlightDate() << "',"
       << booking->getSeatNumber() << ",'" 
       << booking->getSeatClass() << "',"
       << booking->getPrice() << ","
       << booking->getBookingTime() << ");";
    
    sqlite3_exec(db, ss.str().c_str(), nullptr, nullptr, nullptr);
    
    // Save to booked_seats
    stringstream ss2;
    ss2 << "INSERT INTO booked_seats VALUES ('"
        << booking->getFlightNumber() << "','"
        << booking->getFlightDate() << "',"
        << booking->getSeatNumber() << ",'" 
        << booking->getPassengerName() << "');";
    
    sqlite3_exec(db, ss2.str().c_str(), nullptr, nullptr, nullptr);
}
