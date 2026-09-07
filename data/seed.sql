-- 1. Create the Schema (Tables)
CREATE TABLE IF NOT EXISTS Locations (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    type TEXT NOT NULL CHECK(type IN ('WAREHOUSE', 'RESIDENTIAL', 'COMMERCIAL'))
);

CREATE TABLE IF NOT EXISTS Roads (
    source_id INTEGER NOT NULL,
    dest_id INTEGER NOT NULL,
    distance_km REAL NOT NULL,
    traffic_multiplier REAL DEFAULT 1.0,
    PRIMARY KEY (source_id, dest_id),
    FOREIGN KEY (source_id) REFERENCES Locations(id),
    FOREIGN KEY (dest_id) REFERENCES Locations(id)
);

CREATE TABLE IF NOT EXISTS Packages (
    id INTEGER PRIMARY KEY,
    dest_location_id INTEGER NOT NULL,
    priority_level INTEGER NOT NULL CHECK(priority_level BETWEEN 1 AND 5),
    deadline_timestamp INTEGER NOT NULL,
    status TEXT NOT NULL CHECK(status IN ('PENDING', 'DISPATCHED', 'DELIVERED', 'CANCELLED')),
    FOREIGN KEY (dest_location_id) REFERENCES Locations(id)
);

CREATE TABLE IF NOT EXISTS Delivery_Logs (
    log_id INTEGER PRIMARY KEY AUTOINCREMENT,
    driver_id INTEGER NOT NULL,
    route_path TEXT NOT NULL,
    delivered_package_ids TEXT NOT NULL,
    total_distance_km REAL NOT NULL,
    dispatch_timestamp INTEGER NOT NULL
);

-- 2. Insert the Map Data
INSERT INTO Locations (id, name, type) VALUES 
(1, 'Central Warehouse', 'WAREHOUSE'),
(2, 'Downtown', 'COMMERCIAL'),
(3, 'North Suburb', 'RESIDENTIAL'),
(4, 'West End', 'RESIDENTIAL'),
(5, 'East Tech Park', 'COMMERCIAL');

-- Connect the map with roads (Source, Dest, Distance, Traffic Multiplier)
INSERT INTO Roads VALUES 
(1, 2, 5.0, 1.0), (2, 1, 5.0, 1.0),
(2, 3, 3.5, 1.5), (3, 2, 3.5, 1.5),
(1, 4, 8.0, 1.0), (4, 1, 8.0, 1.0),
(2, 5, 4.0, 1.0), (5, 2, 4.0, 1.0),
(3, 5, 6.0, 1.0), (5, 3, 6.0, 1.0);