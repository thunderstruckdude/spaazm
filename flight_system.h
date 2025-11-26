/**
 * @file flight_system.h
 * @brief Backend class declarations for Spaazm Flights Reservation System
 * 
 * This file contains all business logic classes for managing flights, seats,
 * bookings, and the reservation system. Completely independent of GUI code.
 * 
 * @author Spaazm Flights Development Team
 * @date November 2025
 */

#ifndef FLIGHT_SYSTEM_H
#define FLIGHT_SYSTEM_H

#include <vector>
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

struct sqlite3;  // Forward declaration for SQLite database handle

using namespace std;

// ==================== BACKEND CLASSES ====================

/**
 * @class Seat
 * @brief Represents a single seat on a flight
 * 
 * Manages seat number, class, booking status, and passenger information.
 * Each flight has 100 seats: 10 First, 20 Business, 70 Economy.
 */
class Seat {
private:
    int seatNumber;        ///< Seat number (1-100)
    string seatClass;      ///< "First", "Business", or "Economy"
    bool isBooked;         ///< Availability status
    string passengerName;  ///< Name of passenger if booked, empty otherwise

public:
    /**
     * @brief Constructs a new Seat object
     * @param number Seat number (1-100)
     * @param sClass Seat class ("First", "Business", or "Economy")
     */
    Seat(int number, string sClass);
    
    // Getters
    int getSeatNumber() const { return seatNumber; }
    string getSeatClass() const { return seatClass; }
    bool getIsBooked() const { return isBooked; }
    string getPassengerName() const { return passengerName; }

    /**
     * @brief Books this seat for a passenger
     * @param name Passenger's full name
     */
    void bookSeat(string name);
    
    /**
     * @brief Cancels booking and frees this seat
     */
    void cancelBooking();
};

/**
 * @class Flight
 * @brief Represents a complete flight with 100 seats
 * 
 * Manages flight details, seat inventory, and dynamic pricing calculations.
 * Seats are divided into: 10 First (1-10), 20 Business (11-30), 70 Economy (31-100)
 */
class Flight {
private:
    string flightNumber;        ///< Unique identifier (e.g., "SP1001")
    string flightName;          ///< Display name (e.g., "Sky Express")
    string source;              ///< Departure city
    string destination;         ///< Arrival city
    string departureTime;       ///< Full datetime string (YYYY-MM-DD HH:MM)
    double basePrice;           ///< Starting price in INR
    int totalSeats;             ///< Always 100
    vector<Seat*> seats;        ///< Composition: Flight owns 100 Seat objects
    time_t departureTimestamp;  ///< Unix timestamp for time calculations

    /**
     * @brief Initializes 100 seat objects with appropriate classes
     */
    void initializeSeats();

public:
    /**
     * @brief Constructs a new Flight object
     * @param fNumber Flight number
     * @param fName Display name
     * @param src Source city
     * @param dest Destination city
     * @param depTime Departure datetime string
     * @param price Base price in INR
     * @param depTimestamp Departure as Unix timestamp
     */
    Flight(string fNumber, string fName, string src, string dest, string depTime, double price, time_t depTimestamp);
    
    /**
     * @brief Destructor - frees all 100 Seat objects
     */
    ~Flight();

    // Getters
    string getFlightNumber() const { return flightNumber; }
    string getFlightName() const { return flightName; }
    string getSource() const { return source; }
    string getDestination() const { return destination; }
    string getDepartureTime() const { return departureTime; }
    double getBasePrice() const { return basePrice; }
    time_t getDepartureTimestamp() const { return departureTimestamp; }

    /**
     * @brief Calculates dynamic price based on multiple factors
     * @param seatClass "Economy", "Business", or "First"
     * @param bookingTime Current time as Unix timestamp
     * @return Final price in INR after applying all multipliers
     * 
     * Factors: Class (1x-3x), Demand (1x-1.5x), Advance Booking (0.5x-1.15x), Time of Day (0.9x-1.3x)
     */
    double calculatePrice(string seatClass, time_t bookingTime) const;
    
    /**
     * @brief Returns number of booked seats for demand pricing
     */
    int getBookedSeatsCount() const;
    
    /**
     * @brief Returns number of available seats
     */
    int getAvailableSeatsCount() const;
    
    /**
     * @brief Filters available seats by class
     * @param seatClass Target seat class
     * @return Vector of available Seat pointers
     */
    vector<Seat*> getAvailableSeatsByClass(string seatClass) const;
    
    /**
     * @brief Finds seat by number
     * @param seatNumber Seat number (1-100)
     * @return Pointer to Seat or nullptr if not found
     */
    Seat* getSeatByNumber(int seatNumber);
    
    /**
     * @brief Books a seat for a passenger
     * @param seatNumber Seat to book (1-100)
     * @param passengerName Passenger's name
     * @return true if successful, false if seat unavailable
     */
    bool bookSeat(int seatNumber, string passengerName);
    
    /**
     * @brief Cancels a seat booking
     * @param seatNumber Seat to cancel (1-100)
     * @return true if successful
     */
    bool cancelSeat(int seatNumber);
    
    const vector<Seat*>& getAllSeats() const { return seats; }
};

/**
 * @class Booking
 * @brief Represents a confirmed flight reservation
 * 
 * Stores complete booking information including passenger details,
 * flight information, seat assignment, and price paid.
 */
class Booking {
private:
    static int bookingCounter;  ///< Static counter for unique IDs (starts at 1000)
    int bookingId;              ///< Unique booking identifier
    string passengerName;       ///< Full name of passenger
    string email;               ///< Email address (validated)
    string phone;               ///< Phone number
    string flightNumber;        ///< Associated flight number
    string flightDate;          ///< Flight date (YYYY-MM-DD)
    int seatNumber;             ///< Assigned seat (1-100)
    double price;               ///< Final price paid (after dynamic pricing)
    time_t bookingTime;         ///< When booking was made (Unix timestamp)
    string seatClass;           ///< "Economy", "Business", or "First"

public:
    /**
     * @brief Creates a new booking record
     * @param name Passenger's full name
     * @param mail Email address
     * @param ph Phone number
     * @param fNumber Flight number
     * @param fDate Flight date
     * @param seat Seat number
     * @param p Price paid
     * @param sClass Seat class
     */
    Booking(string name, string mail, string ph, string fNumber, string fDate, int seat, double p, string sClass);

    int getBookingId() const { return bookingId; }
    string getPassengerName() const { return passengerName; }
    string getEmail() const { return email; }
    string getPhone() const { return phone; }
    string getFlightNumber() const { return flightNumber; }
    string getFlightDate() const { return flightDate; }
    int getSeatNumber() const { return seatNumber; }
    double getPrice() const { return price; }
    string getSeatClass() const { return seatClass; }
    time_t getBookingTime() const { return bookingTime; }
};

/**
 * @class ReservationSystem
 * @brief Main controller for the flight reservation system
 * 
 * Coordinates all operations between GUI, business logic, and database.
 * Manages flight search, booking operations, and data persistence.
 */
class ReservationSystem {
private:
    vector<Flight*> flights;    ///< Currently loaded flights (from search)
    vector<Booking*> bookings;  ///< All bookings (in-memory cache)
    sqlite3* db;                ///< Database connection handle
    
    /**
     * @brief Clears currently loaded flights from memory
     */
    void clearFlights();
    
    /**
     * @brief Initializes database schema on first run
     * Creates flights, bookings, and booked_seats tables
     */
    void initDatabase();
    
    /**
     * @brief Populates database with flight schedules
     * Generates 5,400 flights (36 routes × 5 flights × 30 days)
     * Only runs if database is empty
     */
    void populateFlights();
    
    /**
     * @brief Placeholder for on-demand flight loading
     */
    void loadFlights();
    
    /**
     * @brief Loads booked seats from database for a flight
     * @param flight Flight object to update with booking status
     */
    void loadBookedSeats(Flight* flight);
    
    /**
     * @brief Persists booking to database
     * @param booking Booking object to save
     */
    void saveBooking(Booking* booking);

public:
    /**
     * @brief Constructs ReservationSystem and initializes database
     */
    ReservationSystem();
    
    /**
     * @brief Destructor - closes database and frees all objects
     */
    ~ReservationSystem();

    /**
     * @brief Searches for flights matching criteria
     * @param dateStr Date in YYYY-MM-DD format
     * @param source Departure city
     * @param destination Arrival city
     * 
     * Queries database, creates Flight objects, and loads booked seats
     */
    void searchFlights(const string& dateStr, const string& source, const string& destination);
    
    /**
     * @brief Gets all unique cities from database
     * @return Sorted list of city names for dropdown population
     */
    vector<string> getUniqueCities() const;
    
    /**
     * @brief Gets currently loaded flights (from last search)
     */
    const vector<Flight*>& getFlights() const { return flights; }
    
    /**
     * @brief Gets all bookings
     */
    const vector<Booking*>& getBookings() const { return bookings; }

    /**
     * @brief Finds a flight by number in currently loaded flights
     * @param flightNumber Flight number to search for
     * @return Pointer to Flight or nullptr if not found
     */
    Flight* findFlight(string flightNumber);
    
    /**
     * @brief Creates a new booking
     * @param passengerName Passenger's full name
     * @param email Email address
     * @param phone Phone number
     * @param flightNumber Flight number
     * @param flightDate Flight date
     * @param seatNumber Seat number (1-100)
     * @param price Final price paid
     * @param seatClass Seat class
     * @return Pointer to created Booking or nullptr on failure
     */
    Booking* addBooking(string passengerName, string email, string phone, string flightNumber, string flightDate, int seatNumber, double price, string seatClass);
    
    /**
     * @brief Cancels a booking
     * @param bookingId Unique booking ID
     * @return true if successful, false if booking not found
     * 
     * Removes from database, frees seat, and deletes booking object
     */
    bool cancelBooking(int bookingId);
};

#endif // FLIGHT_SYSTEM_H
