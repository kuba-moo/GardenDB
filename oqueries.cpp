#include <QApplication>
#include <QSize>
#include <QString>

#include "oqueries.h"

const unsigned int numCreates = 8;
const QString creates[numCreates] = {
    "CREATE TABLE IF NOT EXISTS Types ("
        "id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
        "name TEXT"
    ")",
    "CREATE TABLE IF NOT EXISTS Flavour ("
        "id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
        "name TEXT"
    ")",
    "CREATE TABLE IF NOT EXISTS Flowering ("
        "id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
        "name TEXT"
    ")",
    "CREATE TABLE IF NOT EXISTS Frost ("
        "id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
        "name TEXT"
    ")",
    "CREATE TABLE IF NOT EXISTS Species ("
        "id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
        "name TEXT,"
        "flowers TEXT,"
        "size TEXT,"
        "desc TEXT,"
        "main_photo INTEGER,"
        "fl_id INTEGER,"
        "fw_id INTEGER,"
        "fr_id INTEGER,"
        "tp_id INTEGER,"
        "FOREIGN KEY(fl_id) REFERENCES Flavour(id),"
        "FOREIGN KEY(fw_id) REFERENCES Flowering(id),"
        "FOREIGN KEY(fr_id) REFERENCES Frost(id),"
        "FOREIGN KEY(tp_id) REFERENCES Types(id)"
    ")",
    "CREATE TABLE IF NOT EXISTS Images ("
        "id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
        "data BLOB,"
        "sp_id INTEGER,"
        "FOREIGN KEY(sp_id) REFERENCES Species(id) ON DELETE CASCADE"
    ")",
    "CREATE TABLE IF NOT EXISTS ImagesCache ("
        "id INTEGER PRIMARY KEY,"
        "data BLOB,"
        "FOREIGN KEY(id) REFERENCES Images(id) ON DELETE CASCADE"
    ")",
};

/* TODO: translate those */
const unsigned int numInserts = 22;
const QString inserts[numInserts] = {
    "INSERT INTO Types VALUES(1, 'wielkokwiatowe')",
    "INSERT INTO Types VALUES(2, 'wielokwiatowe')",
    "INSERT INTO Types VALUES(3, '"+QApplication::trUtf8("pnące")+"')",
    "INSERT INTO Types VALUES(4, 'parkowe')",
    "INSERT INTO Types VALUES(5, 'okrywowe')",
    "INSERT INTO Types VALUES(6, 'historyczne')",
    "INSERT INTO Types VALUES(7, 'dzikie')",
    "INSERT INTO Flavour VALUES(1, 'bardzo silny')",
    "INSERT INTO Flavour VALUES(2, 'silny')",
    "INSERT INTO Flavour VALUES(3, '"+QApplication::trUtf8("średni")+"')",
    "INSERT INTO Flavour VALUES(4, '"+QApplication::trUtf8("słaby")+"')",
    "INSERT INTO Flowering VALUES(1, 'maj-czerwiec')",
    "INSERT INTO Flowering VALUES(2, 'czerwiec-lipiec')",
    "INSERT INTO Flowering VALUES(3, 'czerwiec-lipiec powtarza')",
    "INSERT INTO Flowering VALUES(4, 'czerwiec-lipiec "+QApplication::trUtf8("słabo powtarza")+"')",
    "INSERT INTO Frost VALUES(1, '5a')",
    "INSERT INTO Frost VALUES(2, '5b')",
    "INSERT INTO Frost VALUES(3, '6a')",
    "INSERT INTO Frost VALUES(4, '6b')",
    "INSERT INTO Frost VALUES(5, '7a')",
    "INSERT INTO Frost VALUES(6, '7b')",

    /* null - image */
    "INSERT INTO Images VALUES(1, '', 0)"
};

const QString getTypes = "SELECT * FROM Types";
const QString getFlavours = "SELECT * FROM Flavour";
const QString getFrost = "SELECT * FROM Frost";
const QString getFlowering = "SELECT * FROM Flowering";
