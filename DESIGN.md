# Spaazm Flights - Design Documentation

## System Architecture

### High-Level Architecture Diagram

```
┌───────────────────────────────────────────────────────────────────┐
│                          USER INTERFACE                           │
│                                                                   │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐      │
│  │   Search     │    │   Booking    │    │   Bookings   │      │
│  │   Flights    │    │   Dialog     │    │   Manager    │      │
│  └──────┬───────┘    └──────┬───────┘    └──────┬───────┘      │
│         │                   │                   │               │
└─────────┼───────────────────┼───────────────────┼───────────────┘
          │                   │                   │
          ▼                   ▼                   ▼
┌───────────────────────────────────────────────────────────────────┐
│                      BUSINESS LOGIC LAYER                         │
│                                                                   │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │               ReservationSystem (Controller)               │  │
│  │                                                            │  │
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐   │  │
│  │  │   Flight     │  │   Booking    │  │    Seat      │   │  │
│  │  │   Manager    │  │   Manager    │  │   Manager    │   │  │
│  │  └──────────────┘  └──────────────┘  └──────────────┘   │  │
│  │                                                            │  │
│  │  • Search flights         • Create bookings               │  │
│  │  • Load seat status       • Cancel bookings               │  │
│  │  • Calculate prices       • Track reservations            │  │
│  └───────────────────────────────────────────────────────────┘  │
│                                                                   │
└─────────────────────────────┬─────────────────────────────────────┘
                              │
                              ▼
┌───────────────────────────────────────────────────────────────────┐
│                      DATA PERSISTENCE LAYER                       │
│                                                                   │
│                         SQLite3 Database                          │
│                                                                   │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐          │
│  │   flights    │  │   bookings   │  │ booked_seats │          │
│  │              │  │              │  │              │          │
│  │  5400 rows   │  │  Dynamic     │  │  Dynamic     │          │
│  │  (permanent) │  │  (grows)     │  │  (grows)     │          │
│  └──────────────┘  └──────────────┘  └──────────────┘          │
│                                                                   │
└───────────────────────────────────────────────────────────────────┘
```

---

## Data Flow Diagrams

### 1. Flight Search Flow

```
User
  │
  │ 1. Selects Date + Route
  ▼
┌─────────────┐
│  MainWindow │
│   (GUI)     │
└──────┬──────┘
       │ 2. searchFlights(date, src, dest)
       ▼
┌──────────────────┐
│ ReservationSystem│
└──────┬───────────┘
       │ 3. SQL Query
       │    SELECT * FROM flights
       │    WHERE date=? AND source=? AND dest=?
       ▼
┌──────────────┐
│  SQLite DB   │
└──────┬───────┘
       │ 4. Return rows
       ▼
┌──────────────────┐
│ ReservationSystem│──────► 5. Create Flight objects
└──────┬───────────┘
       │ 6. For each flight:
       │    loadBookedSeats(flight)
       ▼
┌──────────────┐
│  SQLite DB   │
│ booked_seats │
└──────┬───────┘
       │ 7. Return booked seat numbers
       ▼
┌──────────────────┐
│ Flight objects   │──────► 8. Mark seats as booked
│ (in memory)      │
└──────┬───────────┘
       │ 9. Return flights vector
       ▼
┌─────────────┐
│  MainWindow │──────► 10. Display FlightCard widgets
│   (GUI)     │
└─────────────┘
```

### 2. Booking Flow

```
User
  │
  │ 1. Clicks "Book Now"
  ▼
┌─────────────────┐
│ Booking Dialog  │
│                 │
│ • Name input    │
│ • Email input   │
│ • Phone input   │
│ • Class select  │
│ • Seat map      │
└────────┬────────┘
         │ 2. User fills form & selects seat
         │ 3. Clicks "Confirm"
         ▼
┌────────────────────┐
│  Input Validation  │
│  • Name not empty  │
│  • Email has @     │
│  • Phone not empty │
│  • Seat selected   │
└────────┬───────────┘
         │ 4. If valid
         ▼
┌─────────────────────┐
│ Flight::bookSeat()  │──────► Updates Seat object
└─────────┬───────────┘
          │ 5. If successful
          ▼
┌──────────────────────────┐
│ ReservationSystem::      │
│ addBooking(...)          │
└──────────┬───────────────┘
           │ 6. Create Booking object
           ▼
┌────────────────────┐
│  saveBooking()     │
└─────────┬──────────┘
          │ 7. INSERT INTO bookings
          │ 8. INSERT INTO booked_seats
          ▼
┌─────────────────┐
│   SQLite DB     │──────► Persisted!
└─────────────────┘
          │
          │ 9. Success confirmation
          ▼
┌─────────────────┐
│     User        │──────► Sees confirmation dialog
└─────────────────┘
```

### 3. Dynamic Pricing Flow

```
User selects seat
  │
  ▼
┌──────────────────────┐
│ Flight::             │
│ calculatePrice(      │
│   seatClass,         │
│   bookingTime        │
│ )                    │
└──────┬───────────────┘
       │
       │ Step 1: Base Price
       │ ────────────────────
       ├──► basePrice = flight.basePrice
       │    (e.g., ₹5000)
       │
       │ Step 2: Class Multiplier
       │ ────────────────────
       ├──► if (Economy)  → × 1.0
       │    if (Business) → × 2.0
       │    if (First)    → × 3.0
       │
       │ Step 3: Demand Factor
       │ ────────────────────
       ├──► occupancy = bookedSeats / 100
       │    demandFactor = 1.0 + (occupancy × 0.5)
       │    (0% = 1.0x, 100% = 1.5x)
       │
       │ Step 4: Advance Booking Factor
       │ ────────────────────
       ├──► daysAhead = (departure - booking) / 86400
       │    if (> 30 days) → × 0.50 (-50%)
       │    if (> 14 days) → × 0.70 (-30%)
       │    if (> 7 days)  → × 0.85 (-15%)
       │    if (> 3 days)  → × 1.00 (base)
       │    if (≤ 3 days)  → × 1.15 (+15%)
       │
       │ Step 5: Time of Day Factor
       │ ────────────────────
       └──► Extract hour from departure time
            00-05: × 0.90 (night)
            06-08: × 1.00 (morning)
            09-11: × 1.20 (peak morning)
            12-14: × 1.10 (afternoon)
            15-17: × 1.30 (peak evening)
            18-20: × 1.15 (evening)
            21-23: × 0.95 (late)
            │
            │ Step 6: Final Calculation
            ▼
     price = basePrice × class × demand × advance × timeOfDay
            │
            ▼
     Display to user (live updates as they change selections)
```

---

## Class Relationships

### UML Class Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                    ReservationSystem                        │
├─────────────────────────────────────────────────────────────┤
│ - flights: vector<Flight*>                                  │
│ - bookings: vector<Booking*>                                │
│ - db: sqlite3*                                              │
├─────────────────────────────────────────────────────────────┤
│ + searchFlights(date, src, dest): void                      │
│ + addBooking(...): Booking*                                 │
│ + cancelBooking(id): bool                                   │
│ + getUniqueCities(): vector<string>                         │
│ - initDatabase(): void                                      │
│ - populateFlights(): void                                   │
│ - loadBookedSeats(flight): void                             │
└──────────┬────────────────────────────────┬─────────────────┘
           │ 1                              │ 1
           │ owns *                         │ owns *
           │                                │
     ┌─────▼──────┐                  ┌──────▼──────┐
     │   Flight   │                  │   Booking   │
     ├────────────┤                  ├─────────────┤
     │ - number   │                  │ - id        │
     │ - name     │                  │ - name      │
     │ - source   │                  │ - email     │
     │ - dest     │                  │ - phone     │
     │ - time     │                  │ - flight    │
     │ - price    │                  │ - seat      │
     │ - seats    │◄─────────────────│ - price     │
     ├────────────┤                  └─────────────┘
     │ + calculatePrice()             (references)
     │ + bookSeat()
     │ + getSeats()
     └─────┬──────┘
           │ 1
           │ owns 100
           │
     ┌─────▼──────┐
     │    Seat    │
     ├────────────┤
     │ - number   │
     │ - class    │
     │ - isBooked │
     │ - passenger│
     ├────────────┤
     │ + bookSeat()
     │ + cancel()
     └────────────┘
```

### Composition Relationships

**ReservationSystem HAS-MANY**
- Flight objects (dynamically loaded)
- Booking objects (all bookings)

**Flight HAS-MANY**
- 100 Seat objects (fixed composition)

**Ownership Rules:**
- ReservationSystem owns and manages Flight* pointers
- ReservationSystem owns and manages Booking* pointers
- Flight owns and manages Seat* pointers
- All use manual memory management (new/delete)

---

## Database Entity-Relationship Diagram

```
┌─────────────────────────┐
│       flights           │
│─────────────────────────│
│ PK (flight_number,date) │◄──────────┐
│    flight_name          │           │
│    source               │           │ Foreign Key
│    destination          │           │ Reference
│    departure_time       │           │
│    base_price           │           │
└─────────────────────────┘           │
                                      │
                                      │
┌─────────────────────────┐           │
│       bookings          │           │
│─────────────────────────│           │
│ PK id                   │           │
│    passenger_name       │           │
│    passenger_email      │           │
│    passenger_phone      │           │
│ FK flight_number        │───────────┤
│    flight_date          │───────────┘
│    seat_number          │
│    seat_class           │
│    price                │
│    booking_time         │
└─────────────────────────┘
          │
          │ Links to
          │
          ▼
┌─────────────────────────┐
│     booked_seats        │
│─────────────────────────│
│ PK (flight_number,      │
│     flight_date,        │
│     seat_number)        │
│    passenger_name       │
└─────────────────────────┘
```

### Table Relationships

1. **flights**: Master table (permanent data)
   - Pre-populated with all available flights
   - Never modified after initial population

2. **bookings**: Transaction records
   - One row per confirmed booking
   - Contains all passenger and payment info
   - References flights via (flight_number, flight_date)

3. **booked_seats**: Quick lookup table
   - Optimized for checking seat availability
   - One row per occupied seat
   - Used by `loadBookedSeats()` to restore state

---

## State Diagrams

### Seat State Machine

```
┌─────────────┐
│   CREATED   │
└──────┬──────┘
       │ Constructor called
       ▼
┌─────────────┐
│  AVAILABLE  │◄─────────────────┐
│             │                  │
│ isBooked=   │                  │ cancelBooking()
│   false     │                  │
└──────┬──────┘                  │
       │                         │
       │ bookSeat(name)          │
       ▼                         │
┌─────────────┐                  │
│   BOOKED    │──────────────────┘
│             │
│ isBooked=   │
│   true      │
│ passenger=  │
│   <name>    │
└─────────────┘
```

### Booking Lifecycle

```
    ┌───────────┐
    │  Created  │
    └─────┬─────┘
          │ User submits form
          ▼
    ┌───────────┐
    │Validating │
    └─────┬─────┘
          │
      ┌───┴───┐
      │       │
Valid │       │ Invalid
      │       │
      ▼       ▼
┌──────────┐ ┌────────────┐
│Confirmed │ │  Rejected  │
│          │ └────────────┘
│ Saved to │
│ database │
└─────┬────┘
      │
      │ User cancels
      ▼
┌──────────┐
│Cancelled │
│          │
│ Removed  │
│ from DB  │
└──────────┘
```

---

## Sequence Diagrams

### Complete Booking Sequence

```
User    MainWindow   Dialog   ReservationSystem   Flight   Database
 │          │          │              │             │         │
 │ Select   │          │              │             │         │
 │ Flight   │          │              │             │         │
 ├─────────►│          │              │             │         │
 │          │ Create   │              │             │         │
 │          │ Dialog   │              │             │         │
 │          ├─────────►│              │             │         │
 │          │          │              │             │         │
 │ Enter    │          │              │             │         │
 │ Details  │          │              │             │         │
 ├──────────┼─────────►│              │             │         │
 │          │          │              │             │         │
 │ Select   │          │              │             │         │
 │ Seat     │          │              │             │         │
 ├──────────┼─────────►│              │             │         │
 │          │          │ Validate     │             │         │
 │          │          │ Inputs       │             │         │
 │          │          │              │             │         │
 │ Confirm  │          │              │             │         │
 ├──────────┼─────────►│              │             │         │
 │          │          │ bookSeat()   │             │         │
 │          │          ├─────────────►│             │         │
 │          │          │              │ getSeat()   │         │
 │          │          │              ├────────────►│         │
 │          │          │              │             │ Update  │
 │          │          │              │◄────────────┤ Status  │
 │          │          │              │             │         │
 │          │          │ addBooking() │             │         │
 │          │          ├─────────────►│             │         │
 │          │          │              │ new Booking │         │
 │          │          │              │ saveBooking()         │
 │          │          │              ├────────────────────►  │
 │          │          │              │        INSERT bookings│
 │          │          │              │        INSERT seats   │
 │          │          │              │◄────────────────────  │
 │          │          │◄─────────────┤             │         │
 │          │          │ Success      │             │         │
 │          │          │              │             │         │
 │          │ Show     │              │             │         │
 │          │ Confirm  │              │             │         │
 │◄─────────┼──────────┤              │             │         │
 │          │          │              │             │         │
```

---

## Design Patterns Used

### 1. **Model-View-Controller (MVC)**
```
Model:      Flight, Seat, Booking (business logic)
View:       MainWindow, FlightCard, SeatButton (Qt GUI)
Controller: ReservationSystem (coordinates between model and view)
```

### 2. **Composition Over Inheritance**
- Flight composes 100 Seat objects
- ReservationSystem composes Flights and Bookings
- No inheritance hierarchies (kept simple for OOP course)

### 3. **Factory Pattern (Implicit)**
- `ReservationSystem::populateFlights()` generates Flight objects
- `Flight::initializeSeats()` creates Seat objects

### 4. **Repository Pattern**
- ReservationSystem acts as repository for Flights and Bookings
- Abstracts database operations from business logic

### 5. **Observer Pattern (Qt Signals/Slots)**
- User actions emit signals
- Connected slots update UI and trigger operations

---

## Performance Considerations

### Memory Management
```
Estimated Memory Usage:
─────────────────────────
Flight object:     ~200 bytes
  + 100 Seats:     ~4000 bytes per flight
                   = ~4.2 KB per flight

Max loaded flights: ~5-10 flights per search
Memory for flights: ~42 KB typical

Booking object:    ~150 bytes
100 bookings:      ~15 KB

Total typical:     < 100 KB (minimal)
```

### Database Performance
```
flights table:     5,400 rows    (~250 KB)
bookings table:    Growing       (< 1 MB typically)
booked_seats:      Growing       (< 500 KB typically)

Total DB size:     < 2 MB typical

Query Performance:
─────────────────
Flight search:     O(log n) with composite index
Seat lookup:       O(1) with primary key
City list:         O(n) distinct scan (cached in practice)
```

### Optimization Strategies
1. **Indexed Searches**: Primary keys on composite fields
2. **Batch Transactions**: Single transaction for flight population
3. **Lazy Loading**: Flights loaded only when searched
4. **In-Memory Cache**: Loaded flights kept until next search

---

## Security Considerations

### Input Validation
- ✅ Email format validation (contains @)
- ✅ Required field checks (name, email, phone)
- ✅ Seat availability verification
- ⚠️ SQL Injection: Currently vulnerable (uses string concatenation)
- ⚠️ No passenger data encryption

### Recommended Improvements
```cpp
// CURRENT (vulnerable):
sql << "INSERT INTO bookings VALUES ('" << name << "', ...

// RECOMMENDED (use prepared statements):
sqlite3_prepare_v2(db, "INSERT INTO bookings VALUES (?, ?, ...
sqlite3_bind_text(stmt, 1, name.c_str(), ...
```

---

## Future Scalability

### Current Limitations
- **Single-user**: No authentication or multi-user support
- **In-memory state**: No session persistence
- **Fixed routes**: Routes hardcoded in populateFlights()
- **30-day window**: Flights only for next month

### Scalability Path
```
Phase 1 (Current):
  Single user, local database, 10 cities, 30 days

Phase 2 (Medium Scale):
  Multi-user, server database, 50 cities, 90 days
  Add: User accounts, payment processing

Phase 3 (Production):
  Cloud deployment, PostgreSQL, 200+ cities, 1 year ahead
  Add: Load balancing, caching, real-time updates

Phase 4 (Enterprise):
  Microservices, distributed database, global scale
  Add: ML pricing, fraud detection, analytics
```

---

## Testing Strategy

### Unit Tests (Recommended)
```cpp
Test Cases:
───────────
✓ Seat::bookSeat() marks seat as booked
✓ Seat::cancelBooking() frees seat
✓ Flight::calculatePrice() returns correct values
✓ Flight::bookSeat() fails if already booked
✓ ReservationSystem::searchFlights() loads correct flights
✓ ReservationSystem::addBooking() creates booking
✓ Database tables created correctly
```

### Integration Tests
```
✓ Book seat → Check database → Verify persisted
✓ Cancel booking → Check seat status → Verify freed
✓ Search flights → Book seat → Search again → Verify unavailable
✓ Populate database → Count rows → Verify 5400 flights
```

### GUI Tests (Manual)
```
✓ Search shows correct flights
✓ Dropdown shows all 10 cities
✓ Seat map displays correctly
✓ Booked seats appear red
✓ Price updates dynamically
✓ Validation shows error messages
✓ Confirmation dialog appears
```

---

## Conclusion

This design balances:
- **Simplicity**: Easy to understand for OOP course
- **Functionality**: Real-world features (database, pricing)
- **Architecture**: Proper separation of concerns
- **Extensibility**: Can be enhanced in future iterations

The system demonstrates core OOP principles while providing a complete, working flight reservation application.
