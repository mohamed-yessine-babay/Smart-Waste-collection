#ifndef CONTENEUR_H
#define CONTENEUR_H

#include <QString>
#include <QDate>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QDebug>
#include "connection.h"

class Conteneur
{
private:
    int id_conteneur;
    QString type;
    int capacite_max;
    int quantite_actuelle;
    int seuil_alerte;
    QDate date_maj;

public:
    // Constructors
    Conteneur();
    Conteneur(int id, QString t, int cap, int qte, int seuil, QDate date);

    // Add this to your Conteneur class in conteneur.h

    // New function to query container by ID and format for Arduino
    QString getFormattedDataForArduino(int id);

    // Getters
    int getId() const { return id_conteneur; }
    QString getType() const { return type; }
    int getCapaciteMax() const { return capacite_max; }
    int getQuantiteActuelle() const { return quantite_actuelle; }
    int getSeuilAlerte() const { return seuil_alerte; }
    QDate getDateMaj() const { return date_maj; }

    // Setters
    void setId(int id) { id_conteneur = id; }
    void setType(const QString& t) { type = t; }
    void setCapaciteMax(int cap) { capacite_max = cap; }
    void setQuantiteActuelle(int qte) { quantite_actuelle = qte; }
    void setSeuilAlerte(int seuil) { seuil_alerte = seuil; }
    void setDateMaj(const QDate& date) { date_maj = date; }

    // Calculated fields
    int getPourcentageRemplissage() const;
    QString getNiveauAlerte() const;

    // CRUD Methods (following PDF guidelines)
    bool ajouter();                          // CREATE
    QSqlQueryModel* afficher();               // READ
    bool modifier(int id);                    // UPDATE
    bool supprimer(int id);                    // DELETE

    // Static method to check if ID exists
    static bool idExiste(int id);              // Vérifier si un ID existe

    // Prediction methods
    double getFillRatePerDay() const;           // Calculate daily fill rate
    int getDaysUntilFull() const;               // Calculate days until container is full
    QString getTimeUntilFull() const;           // Get human-readable time until full

    // Database query methods
    static QVector<Conteneur> getAllConteneurs();           // Get all containers
    static Conteneur getConteneurById(int id);              // Get container by ID
    static QVector<Conteneur> rechercherConteneurs(const QString& searchText);  // Search containers
    static QVector<Conteneur> filtrerParType(const QString& type);              // Filter by type

    // NEW: Statistics methods
    struct ContainerStats {
        int total;
        int empty;      // 0% fill
        int quarterFull; // 1-25% fill
        int halfFull;    // 26-50% fill
        int threeQuarterFull; // 51-75% fill
        int almostFull;  // 76-99% fill
        int full;        // 100% fill
        double emptyPercent;
        double quarterFullPercent;
        double halfFullPercent;
        double threeQuarterFullPercent;
        double almostFullPercent;
        double fullPercent;
    };

    static ContainerStats getStatistics();

    // Helper methods
    void clear();
    void handleEmail(QSqlDatabase db);
};

#endif // CONTENEUR_H
