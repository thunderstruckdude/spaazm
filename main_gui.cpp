#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QListView>
#include <QPalette>
#include <QMessageBox>
#include <QStackedWidget>
#include <QScrollArea>
#include <QFrame>
#include <QDateEdit>
#include <QDate>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QEasingCurve>
#include <iostream>
#include "flight_system.h"

using namespace std;

// ==================== GUI CLASSES ====================

class FlightCard : public QFrame {
    Q_OBJECT
public:
    FlightCard(Flight* flight, QWidget* parent = nullptr) : QFrame(parent), flight(flight) {
        setFrameShape(QFrame::StyledPanel);
        setStyleSheet(
            "FlightCard { background: white; border-radius: 12px; border: 1px solid #e5e7eb; }"
            "FlightCard:hover { border-color: #a5b4fc; }"
        );

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setSpacing(12);
        layout->setContentsMargins(20, 20, 20, 20);

        QHBoxLayout* headerLayout = new QHBoxLayout();
        
        QLabel* nameLabel = new QLabel(QString::fromStdString(flight->getFlightName()));
        nameLabel->setStyleSheet("font-size: 16px; font-weight: 700; color: #111827;");
        headerLayout->addWidget(nameLabel);
        
        headerLayout->addStretch();
        
        QLabel* flightLabel = new QLabel(QString::fromStdString(flight->getFlightNumber()));
        flightLabel->setStyleSheet("font-size: 12px; color: #6b7280;");
        headerLayout->addWidget(flightLabel);
        
        layout->addLayout(headerLayout);

        // Separator line
        QFrame* separator = new QFrame();
        separator->setFrameShape(QFrame::HLine);
        separator->setStyleSheet("background: #e5e7eb; max-height: 1px;");
        layout->addWidget(separator);

        // Route with visual connection
        QHBoxLayout* routeLayout = new QHBoxLayout();
        
        QString cityStyle = "font-size: 16px; font-weight: 700; color: #1f2937;";
        
        QLabel* sourceLabel = new QLabel(QString::fromStdString(flight->getSource()));
        sourceLabel->setStyleSheet(cityStyle);
        routeLayout->addWidget(sourceLabel);
        
        QLabel* arrow = new QLabel("→");
        arrow->setStyleSheet("font-size: 20px; color: #6366f1;");
        routeLayout->addWidget(arrow);
        
        QLabel* destLabel = new QLabel(QString::fromStdString(flight->getDestination()));
        destLabel->setStyleSheet(cityStyle);
        routeLayout->addWidget(destLabel);
        
        routeLayout->addStretch();
        layout->addLayout(routeLayout);

        QLabel* timeLabel = new QLabel(QString::fromStdString(flight->getDepartureTime()));
        timeLabel->setStyleSheet("font-size: 13px; color: #6b7280; margin-top: 5px;");
        layout->addWidget(timeLabel);

        QHBoxLayout* priceLayout = new QHBoxLayout();
        priceLayout->setContentsMargins(0, 10, 0, 0);
        
        double economyPrice = flight->calculatePrice("Economy", time(nullptr));
        QLabel* priceLabel = new QLabel(QString("From ₹%1").arg(economyPrice, 0, 'f', 0));
        priceLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #059669;");
        priceLayout->addWidget(priceLabel);
        
        priceLayout->addStretch();
        
        int availSeats = flight->getAvailableSeatsCount();
        QString availColor = availSeats > 50 ? "#10b981" : (availSeats > 20 ? "#f59e0b" : "#ef4444");
        QLabel* seatsLabel = new QLabel(QString("%1 seats").arg(availSeats));
        seatsLabel->setStyleSheet(QString("font-size: 12px; color: %1;").arg(availColor));
        priceLayout->addWidget(seatsLabel);
        
        layout->addLayout(priceLayout);

        QPushButton* bookBtn = new QPushButton("Book Flight");
        bookBtn->setCursor(Qt::PointingHandCursor);
        bookBtn->setStyleSheet(
            "QPushButton { background: #6366f1; color: white; border: none; border-radius: 6px; "
            "padding: 10px 20px; font-weight: 600; font-size: 14px; }"
            "QPushButton:hover { background: #4f46e5; }"
            "QPushButton:pressed { background: #4338ca; }"
        );
        connect(bookBtn, &QPushButton::clicked, this, &FlightCard::onBookClicked);
        layout->addWidget(bookBtn);
    }

signals:
    void bookClicked(Flight* flight);

private slots:
    void onBookClicked() {
        emit bookClicked(flight);
    }

private:
    Flight* flight;
};

class SeatButton : public QPushButton {
    Q_OBJECT
public:
    SeatButton(Seat* seat, QWidget* parent = nullptr) : QPushButton(parent), seat(seat) {
        setText(QString::number(seat->getSeatNumber()));
        setFixedSize(50, 50);
        updateStyle();
        
        if (!seat->getIsBooked()) {
            connect(this, &QPushButton::clicked, this, &SeatButton::onClicked);
        } else {
            setEnabled(false);
        }
    }

    Seat* getSeat() const { return seat; }
    bool isSelected() const { return selected; }

    void setSelected(bool sel) {
        selected = sel;
        updateStyle();
    }

signals:
    void seatSelected(Seat* seat);

private slots:
    void onClicked() {
        if (!seat->getIsBooked()) {
            emit seatSelected(seat);
        }
    }

private:
    void updateStyle() {
        QString style;
        if (seat->getIsBooked()) {
            style = "background: #e5e7eb; color: #9ca3af; border: 1px solid #d1d5db;";
        } else if (selected) {
            style = "background: #6366f1; color: white; border: 2px solid #4f46e5; font-weight: 600;";
        } else {
            style = "background: white; color: #1f2937; border: 1px solid #d1d5db;";
            style += "QPushButton:hover { background: #f3f4f6; border-color: #6366f1; }";
        }
        setStyleSheet("QPushButton { " + style + " border-radius: 6px; font-size: 12px; }");
    }

    Seat* seat;
    bool selected = false;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    QWidget* createHeader();
    QWidget* createFlightsPage();
    QWidget* createBookingsPage();
    void updateBookingsList();

private slots:
    void searchFlights();
    void showFlights();
    void showBookings();
    void showBookingDialog(Flight* flight);
    void cancelBooking();

private:
    ReservationSystem* system;
    QStackedWidget* stackedWidget;
    QWidget* flightsPage;
    QWidget* bookingsPage;
    QPushButton* flightsBtn;
    QPushButton* bookingsBtn;
    QGridLayout* flightsLayout;
    QVBoxLayout* bookingsMainLayout;
    QVBoxLayout* bookingsListLayout;
    QWidget* bookingsContainer;
    QDateEdit* dateSelector;
    QWidget* flightsScrollContent;
    QComboBox* sourceSelector;
    QComboBox* destSelector;
};

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("✈ Spaazm Flights - Premium Flight Booking");
    setMinimumSize(1280, 850);

    system = new ReservationSystem();

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QWidget* header = createHeader();
    mainLayout->addWidget(header);

    stackedWidget = new QStackedWidget();
    
    flightsPage = createFlightsPage();
    bookingsPage = createBookingsPage();
    
    stackedWidget->addWidget(flightsPage);
    stackedWidget->addWidget(bookingsPage);
    
    mainLayout->addWidget(stackedWidget);

    setStyleSheet(
        "QMainWindow { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f9fafb, stop:1 #f3f4f6); }"
        "QLabel { font-family: 'Segoe UI', 'Roboto', 'Helvetica', Arial, sans-serif; }"
        "QPushButton { font-family: 'Segoe UI', 'Roboto', 'Helvetica', Arial, sans-serif; }"
    );
}

MainWindow::~MainWindow() {
    delete system;
}

QWidget* MainWindow::createHeader() {
    QWidget* header = new QWidget();
    header->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ffffff, stop:1 #fefeff);"
        "border-bottom: 2px solid #e5e7eb;"
    );
    header->setFixedHeight(90);

    // Add shadow to header
    QGraphicsDropShadowEffect* headerShadow = new QGraphicsDropShadowEffect();
    headerShadow->setBlurRadius(15);
    headerShadow->setColor(QColor(0, 0, 0, 20));
    headerShadow->setOffset(0, 2);
    header->setGraphicsEffect(headerShadow);

    QHBoxLayout* layout = new QHBoxLayout(header);
    layout->setContentsMargins(40, 0, 40, 0);

    // Enhanced logo with gradient background
    QWidget* logoContainer = new QWidget();
    logoContainer->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #6366f1, stop:1 #8b5cf6);"
        "border-radius: 12px; padding: 8px 16px;"
    );
    QHBoxLayout* logoLayout = new QHBoxLayout(logoContainer);
    logoLayout->setContentsMargins(8, 4, 8, 4);
    
    QLabel* logoIcon = new QLabel("✈");
    logoIcon->setStyleSheet("font-size: 28px; color: white;");
    logoLayout->addWidget(logoIcon);
    
    QLabel* logoText = new QLabel("Spaazm Flights");
    logoText->setStyleSheet("font-size: 22px; font-weight: 800; color: white; letter-spacing: 1px;");
    logoLayout->addWidget(logoText);
    
    layout->addWidget(logoContainer);

    layout->addStretch();

    // Enhanced navigation buttons with icons
    flightsBtn = new QPushButton("🔍 Search Flights");
    bookingsBtn = new QPushButton("📋 My Bookings");

    flightsBtn->setCursor(Qt::PointingHandCursor);
    bookingsBtn->setCursor(Qt::PointingHandCursor);

    QString navBtnStyle = 
        "QPushButton {"
        "   background: transparent;"
        "   border: 2px solid transparent;"
        "   color: #6b7280;"
        "   font-size: 15px;"
        "   font-weight: 600;"
        "   padding: 12px 24px;"
        "   border-radius: 10px;"
        "}"
        "QPushButton:hover {"
        "   background: #f3f4f6;"
        "   color: #1f2937;"
        "   border: 2px solid #e5e7eb;"
        "}";

    QString activeBtnStyle = 
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #eef2ff, stop:1 #e0e7ff);"
        "   border: 2px solid #6366f1;"
        "   color: #6366f1;"
        "   font-size: 15px;"
        "   font-weight: 700;"
        "   padding: 12px 24px;"
        "   border-radius: 10px;"
        "}";

    flightsBtn->setStyleSheet(activeBtnStyle);
    bookingsBtn->setStyleSheet(navBtnStyle);

    connect(flightsBtn, &QPushButton::clicked, this, &MainWindow::showFlights);
    connect(bookingsBtn, &QPushButton::clicked, this, &MainWindow::showBookings);

    layout->addWidget(flightsBtn);
    layout->addWidget(bookingsBtn);

    // Add user profile icon
    QPushButton* profileBtn = new QPushButton("👤");
    profileBtn->setCursor(Qt::PointingHandCursor);
    profileBtn->setStyleSheet(
        "QPushButton {"
        "   background: #f3f4f6;"
        "   border: 2px solid #e5e7eb;"
        "   color: #1f2937;"
        "   font-size: 20px;"
        "   padding: 10px;"
        "   border-radius: 12px;"
        "   min-width: 48px;"
        "   min-height: 48px;"
        "}"
        "QPushButton:hover {"
        "   background: #e5e7eb;"
        "   border: 2px solid #d1d5db;"
        "}"
    );
    layout->addWidget(profileBtn);

    return header;
}

QWidget* MainWindow::createFlightsPage() {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(30, 20, 30, 20);
    layout->setSpacing(20);

    // Simple header
    QLabel* title = new QLabel("Search Flights");
    title->setStyleSheet("font-size: 28px; font-weight: 700; color: #1f2937;");
    layout->addWidget(title);

    QLabel* subtitle = new QLabel("Select your route and travel date");
    subtitle->setStyleSheet("font-size: 15px; color: #6b7280; margin-bottom: 10px;");
    layout->addWidget(subtitle);

    // Search panel
    QFrame* searchPanel = new QFrame();
    searchPanel->setStyleSheet(
        "QFrame { background: white; border-radius: 12px; border: 1px solid #e5e7eb; }"
    );
    
    QVBoxLayout* searchLayout = new QVBoxLayout(searchPanel);
    searchLayout->setContentsMargins(24, 24, 24, 24);
    searchLayout->setSpacing(20);

    // From/To row
    QHBoxLayout* routeLayout = new QHBoxLayout();
    routeLayout->setSpacing(15);
    
    QVBoxLayout* fromLayout = new QVBoxLayout();
    QLabel* fromLabel = new QLabel("From:");
    fromLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #374151;");
    fromLayout->addWidget(fromLabel);
    
    sourceSelector = new QComboBox();
    vector<string> cities = system->getUniqueCities();
    for (const auto& city : cities) {
        sourceSelector->addItem(QString::fromStdString(city));
    }
    
    // Set up the dropdown list view with explicit palette
    QListView* sourceView = new QListView();
    sourceView->setStyleSheet(
        "QListView { background-color: white; color: black; border: 1px solid #d1d5db; "
        "border-radius: 4px; padding: 4px; outline: none; }"
        "QListView::item { padding: 8px; min-height: 25px; background-color: white; color: black; }"
        "QListView::item:hover { background-color: #f3f4f6; }"
        "QListView::item:selected { background-color: #eef2ff; color: #4f46e5; }"
    );
    QPalette sourcePalette = sourceView->palette();
    sourcePalette.setColor(QPalette::Base, QColor(255, 255, 255));  // White background
    sourcePalette.setColor(QPalette::Text, QColor(0, 0, 0));  // Black text
    sourcePalette.setColor(QPalette::Window, QColor(255, 255, 255));
    sourcePalette.setColor(QPalette::WindowText, QColor(0, 0, 0));
    sourcePalette.setColor(QPalette::Highlight, QColor(238, 242, 255));  // Selection background
    sourcePalette.setColor(QPalette::HighlightedText, QColor(79, 70, 229));  // Selection text
    sourceView->setPalette(sourcePalette);
    sourceSelector->setView(sourceView);
    
    sourceSelector->setStyleSheet(
        "QComboBox { padding: 10px; border: 1px solid #d1d5db; border-radius: 8px; "
        "font-size: 14px; background-color: white; color: black; min-width: 200px; }"
        "QComboBox::drop-down { border: none; width: 30px; }"
        "QComboBox::down-arrow { image: url(none); border-left: 5px solid transparent; "
        "border-right: 5px solid transparent; border-top: 5px solid #6b7280; width: 0; height: 0; }"
    );
    fromLayout->addWidget(sourceSelector);
    routeLayout->addLayout(fromLayout);
    
    QLabel* arrow = new QLabel("→");
    arrow->setStyleSheet("font-size: 24px; color: #6366f1; margin: 0 10px;");
    arrow->setAlignment(Qt::AlignCenter);
    routeLayout->addWidget(arrow);
    
    QVBoxLayout* toLayout = new QVBoxLayout();
    QLabel* toLabel = new QLabel("To:");
    toLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #374151;");
    toLayout->addWidget(toLabel);
    
    destSelector = new QComboBox();
    for (const auto& city : cities) {
        destSelector->addItem(QString::fromStdString(city));
    }
    destSelector->setCurrentIndex(1);
    
    // Set up the dropdown list view with explicit palette
    QListView* destView = new QListView();
    destView->setStyleSheet(
        "QListView { background-color: white; color: black; border: 1px solid #d1d5db; "
        "border-radius: 4px; padding: 4px; outline: none; }"
        "QListView::item { padding: 8px; min-height: 25px; background-color: white; color: black; }"
        "QListView::item:hover { background-color: #f3f4f6; }"
        "QListView::item:selected { background-color: #eef2ff; color: #4f46e5; }"
    );
    QPalette destPalette = destView->palette();
    destPalette.setColor(QPalette::Base, QColor(255, 255, 255));  // White background
    destPalette.setColor(QPalette::Text, QColor(0, 0, 0));  // Black text
    destPalette.setColor(QPalette::Window, QColor(255, 255, 255));
    destPalette.setColor(QPalette::WindowText, QColor(0, 0, 0));
    destPalette.setColor(QPalette::Highlight, QColor(238, 242, 255));  // Selection background
    destPalette.setColor(QPalette::HighlightedText, QColor(79, 70, 229));  // Selection text
    destView->setPalette(destPalette);
    destSelector->setView(destView);
    
    destSelector->setStyleSheet(
        "QComboBox { padding: 10px; border: 1px solid #d1d5db; border-radius: 8px; "
        "font-size: 14px; background-color: white; color: black; min-width: 200px; }"
        "QComboBox::drop-down { border: none; width: 30px; }"
        "QComboBox::down-arrow { image: url(none); border-left: 5px solid transparent; "
        "border-right: 5px solid transparent; border-top: 5px solid #6b7280; width: 0; height: 0; }"
    );
    toLayout->addWidget(destSelector);
    routeLayout->addLayout(toLayout);
    
    routeLayout->addStretch();
    searchLayout->addLayout(routeLayout);

    QLabel* dateLabel = new QLabel("Date:");
    dateLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #374151; margin-top: 10px;");
    searchLayout->addWidget(dateLabel);

    dateSelector = new QDateEdit();
    dateSelector->setCalendarPopup(true);
    dateSelector->setMinimumDate(QDate::currentDate());
    dateSelector->setMaximumDate(QDate::currentDate().addDays(59));
    dateSelector->setDate(QDate::currentDate().addDays(7));
    dateSelector->setDisplayFormat("ddd, MMM dd yyyy");
    dateSelector->setStyleSheet(
        "QDateEdit { padding: 10px; border: 1px solid #d1d5db; border-radius: 8px; "
        "font-size: 14px; background: white; color: black; min-width: 240px; }"
        "QDateEdit::drop-down { border: none; width: 30px; }"
        "QDateEdit::down-arrow { image: url(none); border-left: 5px solid transparent; "
        "border-right: 5px solid transparent; border-top: 5px solid #6b7280; width: 0; height: 0; }"
    );
    searchLayout->addWidget(dateSelector);
    
    QPushButton* searchBtn = new QPushButton("Search Flights");
    searchBtn->setCursor(Qt::PointingHandCursor);
    searchBtn->setStyleSheet(
        "QPushButton { background: #6366f1; color: white; border: none; border-radius: 8px; "
        "padding: 12px 32px; font-weight: 600; font-size: 15px; margin-top: 15px; }"
        "QPushButton:hover { background: #4f46e5; }"
        "QPushButton:pressed { background: #4338ca; }"
    );
    connect(searchBtn, &QPushButton::clicked, this, &MainWindow::searchFlights);
    searchLayout->addWidget(searchBtn);

    layout->addWidget(searchPanel);

    QLabel* resultsHeader = new QLabel("Available Flights");
    resultsHeader->setStyleSheet("font-size: 20px; font-weight: 600; color: #1f2937; margin-top: 20px;");
    layout->addWidget(resultsHeader);

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    flightsScrollContent = new QWidget();
    flightsLayout = new QGridLayout(flightsScrollContent);
    flightsLayout->setSpacing(24);
    flightsLayout->setContentsMargins(0, 0, 0, 0);

    scrollArea->setWidget(flightsScrollContent);
    layout->addWidget(scrollArea);

    return page;
}

QWidget* MainWindow::createBookingsPage() {
    QWidget* page = new QWidget();
    bookingsMainLayout = new QVBoxLayout(page);
    bookingsMainLayout->setContentsMargins(40, 30, 40, 30);
    bookingsMainLayout->setSpacing(30);

    QLabel* title = new QLabel("Your Bookings");
    title->setStyleSheet("font-size: 32px; font-weight: 700; color: #1f2937;");
    bookingsMainLayout->addWidget(title);

    bookingsContainer = new QWidget();
    bookingsListLayout = new QVBoxLayout(bookingsContainer);
    bookingsListLayout->setSpacing(15);

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidget(bookingsContainer);

    bookingsMainLayout->addWidget(scrollArea);

    updateBookingsList();

    return page;
}

void MainWindow::updateBookingsList() {
    QLayoutItem* item;
    while ((item = bookingsListLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    const vector<Booking*>& bookings = system->getBookings();
    
    if (bookings.empty()) {
        QLabel* emptyLabel = new QLabel("No bookings yet. Book your first flight!");
        emptyLabel->setStyleSheet("font-size: 16px; color: #9ca3af; padding: 40px;");
        emptyLabel->setAlignment(Qt::AlignCenter);
        bookingsListLayout->addWidget(emptyLabel);
    } else {
        for (Booking* booking : bookings) {
            QFrame* bookingCard = new QFrame();
            bookingCard->setStyleSheet(
                "QFrame { background: white; border-radius: 12px; border: 1px solid #e5e7eb; padding: 20px; }"
            );

            QHBoxLayout* cardLayout = new QHBoxLayout(bookingCard);

            QVBoxLayout* infoLayout = new QVBoxLayout();
            
            QLabel* idLabel = new QLabel(QString("Booking #%1").arg(booking->getBookingId()));
            idLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #6366f1;");
            infoLayout->addWidget(idLabel);

            QLabel* nameLabel = new QLabel(QString::fromStdString(booking->getPassengerName()));
            nameLabel->setStyleSheet("font-size: 18px; font-weight: 600; color: #1f2937;");
            infoLayout->addWidget(nameLabel);

            Flight* flight = system->findFlight(booking->getFlightNumber());
            if (flight) {
                QLabel* routeLabel = new QLabel(QString::fromStdString(
                    flight->getSource() + " → " + flight->getDestination()
                ));
                routeLabel->setStyleSheet("font-size: 14px; color: #6b7280;");
                infoLayout->addWidget(routeLabel);
            }

            QLabel* detailsLabel = new QLabel(QString("Flight: %1 | Seat: %2 (%3)")
                .arg(QString::fromStdString(booking->getFlightNumber()))
                .arg(booking->getSeatNumber())
                .arg(QString::fromStdString(booking->getSeatClass())));
            detailsLabel->setStyleSheet("font-size: 13px; color: #6b7280;");
            infoLayout->addWidget(detailsLabel);

            QLabel* priceLabel = new QLabel(QString("₹%1").arg(booking->getPrice(), 0, 'f', 2));
            priceLabel->setStyleSheet("font-size: 20px; font-weight: 700; color: #059669; margin-top: 8px;");
            infoLayout->addWidget(priceLabel);

            cardLayout->addLayout(infoLayout);
            cardLayout->addStretch();

            QPushButton* cancelBtn = new QPushButton("Cancel Booking");
            cancelBtn->setStyleSheet(
                "QPushButton {"
                "   background: #ef4444;"
                "   color: white;"
                "   border: none;"
                "   border-radius: 8px;"
                "   padding: 10px 20px;"
                "   font-weight: 600;"
                "}"
                "QPushButton:hover { background: #dc2626; }"
            );
            cancelBtn->setProperty("bookingId", booking->getBookingId());
            connect(cancelBtn, &QPushButton::clicked, this, &MainWindow::cancelBooking);
            cardLayout->addWidget(cancelBtn);

            bookingsListLayout->addWidget(bookingCard);
        }
    }

    bookingsListLayout->addStretch();
}

void MainWindow::searchFlights() {
    QString source = sourceSelector->currentText();
    QString dest = destSelector->currentText();
    QString dateStr = dateSelector->date().toString("yyyy-MM-dd");
    
    if (source == dest) {
        QMessageBox::warning(this, "Invalid Route", "Source and destination cannot be the same!");
        return;
    }
    
    // Debug output
    cout << "Searching flights from " << source.toStdString() << " to " 
         << dest.toStdString() << " on " << dateStr.toStdString() << endl;
    
    system->searchFlights(dateStr.toStdString(), source.toStdString(), dest.toStdString());
    
    // Debug output
    cout << "Flights found: " << system->getFlights().size() << endl;
    if (system->getFlights().empty()) {
        cout << "No flights found for this route and date!" << endl;
    }
    
    QLayoutItem* item;
    while ((item = flightsLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    const vector<Flight*>& flights = system->getFlights();
    if (flights.empty()) {
        QLabel* noFlights = new QLabel(QString("No flights available from %1 to %2 on this date.")
            .arg(source).arg(dest));
        noFlights->setStyleSheet("font-size: 16px; color: #9ca3af; padding: 40px;");
        noFlights->setAlignment(Qt::AlignCenter);
        flightsLayout->addWidget(noFlights, 0, 0, 1, 2);
    } else {
        int row = 0, col = 0;
        for (Flight* flight : flights) {
            FlightCard* card = new FlightCard(flight);
            connect(card, &FlightCard::bookClicked, this, &MainWindow::showBookingDialog);
            flightsLayout->addWidget(card, row, col);
            
            col++;
            if (col >= 2) {
                col = 0;
                row++;
            }
        }
    }
}

void MainWindow::showFlights() {
    stackedWidget->setCurrentIndex(0);
    flightsBtn->setStyleSheet(
        "QPushButton { background: #eef2ff; color: #6366f1; font-weight: 600; "
        "border: none; padding: 10px 20px; border-radius: 8px; font-size: 15px; }"
    );
    bookingsBtn->setStyleSheet(
        "QPushButton { background: transparent; color: #6b7280; font-weight: 500; "
        "border: none; padding: 10px 20px; border-radius: 8px; font-size: 15px; }"
        "QPushButton:hover { background: #f3f4f6; color: #1f2937; }"
    );
}

void MainWindow::showBookings() {
    updateBookingsList();
    stackedWidget->setCurrentIndex(1);
    bookingsBtn->setStyleSheet(
        "QPushButton { background: #eef2ff; color: #6366f1; font-weight: 600; "
        "border: none; padding: 10px 20px; border-radius: 8px; font-size: 15px; }"
    );
    flightsBtn->setStyleSheet(
        "QPushButton { background: transparent; color: #6b7280; font-weight: 500; "
        "border: none; padding: 10px 20px; border-radius: 8px; font-size: 15px; }"
        "QPushButton:hover { background: #f3f4f6; color: #1f2937; }"
    );
}

void MainWindow::showBookingDialog(Flight* flight) {
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Book Flight");
    dialog->setMinimumSize(1000, 800);
    dialog->setStyleSheet("QDialog { background: #f9fafb; }");

    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->setSpacing(8);
    layout->setContentsMargins(30, 30, 30, 30);

    QLabel* title = new QLabel(QString::fromStdString(flight->getFlightName()));
    title->setStyleSheet("font-size: 24px; font-weight: 700; color: #1f2937;");
    layout->addWidget(title);

    QLabel* flightNum = new QLabel(QString("Flight %1").arg(QString::fromStdString(flight->getFlightNumber())));
    flightNum->setStyleSheet("font-size: 14px; color: #6366f1; font-weight: 600;");
    layout->addWidget(flightNum);

    QLabel* route = new QLabel(QString::fromStdString(flight->getSource() + " → " + flight->getDestination()));
    route->setStyleSheet("font-size: 16px; color: #6b7280;");
    layout->addWidget(route);

    QLabel* nameLabel = new QLabel("Passenger Name:");
    nameLabel->setStyleSheet("font-weight: 600; color: #374151; margin-top: 5px;");
    layout->addWidget(nameLabel);

    QLineEdit* nameInput = new QLineEdit();
    nameInput->setPlaceholderText("Enter your full name");
    nameInput->setStyleSheet(
        "QLineEdit { padding: 12px; border: 1px solid #d1d5db; border-radius: 8px; "
        "font-size: 14px; background: white; }"
        "QLineEdit:focus { border-color: #6366f1; }"
    );
    layout->addWidget(nameInput);

    QLabel* emailLabel = new QLabel("Email:");
    emailLabel->setStyleSheet("font-weight: 600; color: #374151; margin-top: 5px;");
    layout->addWidget(emailLabel);

    QLineEdit* emailInput = new QLineEdit();
    emailInput->setPlaceholderText("your@email.com");
    emailInput->setStyleSheet(
        "QLineEdit { padding: 12px; border: 1px solid #d1d5db; border-radius: 8px; "
        "font-size: 14px; background: white; }"
        "QLineEdit:focus { border-color: #6366f1; }"
    );
    layout->addWidget(emailInput);

    QLabel* phoneLabel = new QLabel("Phone Number:");
    phoneLabel->setStyleSheet("font-weight: 600; color: #374151; margin-top: 5px;");
    layout->addWidget(phoneLabel);

    QLineEdit* phoneInput = new QLineEdit();
    phoneInput->setPlaceholderText("+91 XXXXXXXXXX");
    phoneInput->setStyleSheet(
        "QLineEdit { padding: 12px; border: 1px solid #d1d5db; border-radius: 8px; "
        "font-size: 14px; background: white; }"
        "QLineEdit:focus { border-color: #6366f1; }"
    );
    layout->addWidget(phoneInput);

    QLabel* classLabel = new QLabel("Select Class:");
    classLabel->setStyleSheet("font-weight: 600; color: #374151; margin-top: 5px;");
    layout->addWidget(classLabel);

    QComboBox* classCombo = new QComboBox();
    classCombo->addItems({"Economy", "Business", "First"});
    classCombo->setStyleSheet(
        "QComboBox { padding: 12px; border: 1px solid #d1d5db; border-radius: 8px; "
        "font-size: 14px; background-color: #ffffff; color: #1f2937; }"
        "QComboBox::drop-down { border: none; width: 30px; background-color: transparent; }"
        "QComboBox::down-arrow { image: url(none); border-left: 5px solid transparent; "
        "border-right: 5px solid transparent; border-top: 5px solid #6b7280; width: 0; height: 0; }"
        "QComboBox QAbstractItemView { background-color: #ffffff; color: #1f2937; border: 1px solid #d1d5db; "
        "border-radius: 4px; padding: 4px; outline: none; selection-background-color: #eef2ff; selection-color: #4f46e5; }"
        "QComboBox QAbstractItemView::item { padding: 8px; min-height: 25px; background-color: #ffffff; color: #1f2937; }"
        "QComboBox QAbstractItemView::item:hover { background-color: #f3f4f6; }"
        "QComboBox QAbstractItemView::item:selected { background-color: #eef2ff; color: #4f46e5; }"
    );
    layout->addWidget(classCombo);

    QLabel* seatLabel = new QLabel("Select Your Seat:");
    seatLabel->setStyleSheet("font-weight: 600; color: #374151; margin-top: 5px;");
    layout->addWidget(seatLabel);

    QScrollArea* seatScroll = new QScrollArea();
    seatScroll->setWidgetResizable(true);
    seatScroll->setFrameShape(QFrame::StyledPanel);
    seatScroll->setStyleSheet("QScrollArea { background: white; border-radius: 8px; border: 1px solid #e5e7eb; }");

    // Store selected seat in dialog property
    dialog->setProperty("selectedSeat", QVariant::fromValue<void*>(nullptr));

    layout->addWidget(seatScroll, 1);

    QLabel* priceLabel = new QLabel();
    priceLabel->setStyleSheet("font-size: 20px; font-weight: 700; color: #059669; margin-top: 10px;");
    layout->addWidget(priceLabel);

    auto updatePrice = [=]() {
        Seat* selectedSeat = static_cast<Seat*>(dialog->property("selectedSeat").value<void*>());
        if (selectedSeat) {
            double price = flight->calculatePrice(
                classCombo->currentText().toStdString(),
                time(nullptr)
            );
            priceLabel->setText(QString("Total Price: ₹%1").arg(price, 0, 'f', 2));
        } else {
            priceLabel->setText("Select a seat to see the price");
        }
    };

    // Now define updateSeats with access to updatePrice
    auto updateSeats = [dialog, flight, seatScroll, updatePrice](const QString& seatClass) {
        // Clear the seat selection
        dialog->setProperty("selectedSeat", QVariant::fromValue<void*>(nullptr));
        
        // Update price when seat selection is cleared
        updatePrice();
        
        // Clear existing layout
        QWidget* oldWidget = seatScroll->takeWidget();
        if (oldWidget) {
            delete oldWidget;
        }
        
        // Create new container
        QWidget* newContainer = new QWidget();
        QVBoxLayout* newLayout = new QVBoxLayout(newContainer);
        newLayout->setSpacing(15);
        newLayout->setContentsMargins(20, 20, 20, 20);

        vector<Seat*> allSeats = flight->getSeatsByClass(seatClass.toStdString());
        
        if (allSeats.empty()) {
            QLabel* noSeats = new QLabel("No seats in this class");
            noSeats->setStyleSheet("color: #ef4444; padding: 20px; font-size: 14px;");
            noSeats->setAlignment(Qt::AlignCenter);
            newLayout->addWidget(noSeats);
            seatScroll->setWidget(newContainer);
            return;
        }

        // Count available seats
        int availableCount = 0;
        for (auto seat : allSeats) {
            if (!seat->getIsBooked()) availableCount++;
        }

        QLabel* legendLabel = new QLabel(QString("%1 Class - %2/%3 seats available")
            .arg(seatClass).arg(availableCount).arg(allSeats.size()));
        legendLabel->setStyleSheet("font-weight: 600; color: #1f2937; font-size: 14px;");
        newLayout->addWidget(legendLabel);

        // Add legend for seat colors
        QHBoxLayout* legendLayout = new QHBoxLayout();
        
        QLabel* availLegend = new QLabel("● Available");
        availLegend->setStyleSheet("color: #10b981; font-size: 12px;");
        legendLayout->addWidget(availLegend);
        
        QLabel* bookedLegend = new QLabel("● Booked");
        bookedLegend->setStyleSheet("color: #9ca3af; font-size: 12px;");
        legendLayout->addWidget(bookedLegend);
        
        QLabel* selectedLegend = new QLabel("● Selected");
        selectedLegend->setStyleSheet("color: #6366f1; font-size: 12px;");
        legendLayout->addWidget(selectedLegend);
        
        legendLayout->addStretch();
        newLayout->addLayout(legendLayout);

        QGridLayout* seatsGrid = new QGridLayout();
        seatsGrid->setSpacing(10);

        int seatsPerRow = (seatClass == "Economy") ? 10 : 5;
        
        // Store all buttons so we can update their selection state
        QList<SeatButton*> allButtons;
        
        for (size_t i = 0; i < allSeats.size(); i++) {
            SeatButton* btn = new SeatButton(allSeats[i]);
            allButtons.append(btn);
            
            QObject::connect(btn, &SeatButton::seatSelected, dialog, [dialog, allButtons, btn, updatePrice](Seat* seat) {
                // Deselect all buttons
                for (SeatButton* sb : allButtons) {
                    sb->setSelected(false);
                }
                // Select this button
                btn->setSelected(true);
                // Store the selected seat in dialog property
                dialog->setProperty("selectedSeat", QVariant::fromValue<void*>(seat));
                // Update the price display
                updatePrice();
            });
            
            seatsGrid->addWidget(btn, i / seatsPerRow, i % seatsPerRow);
        }

        newLayout->addLayout(seatsGrid);
        newLayout->addStretch();
        
        seatScroll->setWidget(newContainer);
    };

    updateSeats(classCombo->currentText());
    connect(classCombo, &QComboBox::currentTextChanged, updateSeats);
    
    connect(classCombo, &QComboBox::currentTextChanged, updatePrice);
    updatePrice();

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setStyleSheet(
        "QPushButton { background: #e5e7eb; color: #374151; border: none; "
        "border-radius: 8px; padding: 12px 24px; font-weight: 600; }"
        "QPushButton:hover { background: #d1d5db; }"
    );
    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);
    btnLayout->addWidget(cancelBtn);

    QPushButton* confirmBtn = new QPushButton("Confirm Booking");
    confirmBtn->setStyleSheet(
        "QPushButton { background: #6366f1; color: white; border: none; "
        "border-radius: 8px; padding: 12px 24px; font-weight: 600; }"
        "QPushButton:hover { background: #4f46e5; }"
    );
    connect(confirmBtn, &QPushButton::clicked, [=]() {
        if (nameInput->text().isEmpty()) {
            QMessageBox::warning(dialog, "Error", "Please enter passenger name");
            return;
        }
        if (emailInput->text().isEmpty() || !emailInput->text().contains("@")) {
            QMessageBox::warning(dialog, "Error", "Please enter a valid email");
            return;
        }
        if (phoneInput->text().isEmpty()) {
            QMessageBox::warning(dialog, "Error", "Please enter phone number");
            return;
        }
        
        Seat* selectedSeat = static_cast<Seat*>(dialog->property("selectedSeat").value<void*>());
        if (!selectedSeat) {
            QMessageBox::warning(dialog, "Error", "Please select a seat");
            return;
        }

        string passengerName = nameInput->text().toStdString();
        string email = emailInput->text().toStdString();
        string phone = phoneInput->text().toStdString();
        string seatClass = classCombo->currentText().toStdString();
        int seatNumber = selectedSeat->getSeatNumber();
        double price = flight->calculatePrice(seatClass, time(nullptr));
        string flightDate = flight->getDepartureTime().substr(0, 10);

        if (flight->bookSeat(seatNumber, passengerName)) {
            system->addBooking(passengerName, email, phone, flight->getFlightNumber(), flightDate, seatNumber, price, seatClass);
            
            QMessageBox::information(dialog, "Success", 
                QString("Booking confirmed!\n\nPassenger: %1\nFlight: %2 - %3\nSeat: %4 (%5)\nPrice: ₹%6")
                .arg(QString::fromStdString(passengerName))
                .arg(QString::fromStdString(flight->getFlightNumber()))
                .arg(QString::fromStdString(flight->getFlightName()))
                .arg(seatNumber)
                .arg(QString::fromStdString(seatClass))
                .arg(price, 0, 'f', 2));
            
            dialog->accept();
            updateBookingsList();
        } else {
            QMessageBox::warning(dialog, "Error", "Failed to book seat. Please try again.");
        }
    });
    btnLayout->addWidget(confirmBtn);

    layout->addLayout(btnLayout);

    dialog->exec();
    delete dialog;
}

void MainWindow::cancelBooking() {
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    int bookingId = btn->property("bookingId").toInt();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Cancel Booking",
        "Are you sure you want to cancel this booking?\n\n10% cancellation fee will be applied.",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        if (system->cancelBooking(bookingId)) {
            QMessageBox::information(this, "Success", "Booking cancelled successfully!\n\n10% cancellation fee applied.");
            updateBookingsList();
        } else {
            QMessageBox::warning(this, "Error", "Failed to cancel booking.");
        }
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    MainWindow window;
    window.show();
    
    return app.exec();
}

#include "main_gui.moc"
