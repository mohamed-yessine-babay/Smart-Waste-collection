#ifndef RECYCLAGE_H
#define RECYCLAGE_H

#include<QString>
#include<QSqlQuery>
#include<QSqlQueryModel>
#include <QDateTime>
#include <QStandardItemModel>


struct EstimationResult {
    QDateTime dateFin;
    float quantiteSortie;
    int dureeMinutes;

};

struct modifier_sortie {
    int id_c_source;      // Clé 1
    int id_m_cible;       // Clé 2
    QString date_debut;   // Clé 3 (pour identifier la ligne exacte)
    float quantite_entree; // Pour le calcul du rendement
};

class Recyclage {
private:
    int id_conteneur;
    int id_machine;
    double quantite;
    double rendement;       // Sera estimé puis modifié
    QString etat;           // "En cours" par défaut
    QDateTime date_debut;   // CurrentTime par défaut
    QDateTime date_fin;     // Estimée puis modifiée
    double quantite_sortie; // Estimée puis modifiée


public:
    // Constructeur par défaut (vide)
    Recyclage();

    // Constructeur pour l'enregistrement initial (ce que tu as décrit)
    Recyclage(int id_c, int id_m, double qte);

    // Getters pour les requêtes Oracle
    int getIdConteneur() const { return id_conteneur; }
    int getIdMachine() const { return id_machine; }
    double getQuantite() const { return quantite; }
    QString getEtat() const { return etat; }
    QDateTime getDateDebut() const { return date_debut; }

    // Setters (très importants pour tes modifications d'estimations plus tard)
    void setRendement(double r) { rendement = r; }
    void setDateFin(QDateTime df) { date_fin = df; }
    void setQuantiteSortie(double qs) { quantite_sortie = qs; }
    void setEtat(QString e) { etat = e; }

    // Méthodes CRUD

    bool ajouterRecyclage();
    bool supprimerRecyclage(int id_c, int id_m, QString date_d);
    QStandardItemModel* afficherHistorique();

    bool verifierCompatibilite(int id_c, int id_m);
    int verifierLimites(int id_c, int id_m, double qte_saisie);
    int verifierTout(int id_c, int id_m, double qte_saisie);
    EstimationResult calculerEstimation(float quantiteEntree, QDateTime dateDebut, QString typeDechet);
    bool terminerRecyclage(int id_c, int id_m, QString date_d, float qte_sortie, float qte_entree);
};

#endif // RECYCLAGE_H
