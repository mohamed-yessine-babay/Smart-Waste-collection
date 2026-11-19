#ifndef POINTCOLLECTE_H
#define POINTCOLLECTE_H
#include <QMessageBox>
#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QObject> //utile pour certains outils Qt comme tr()
#include <QMap> //stocker des statistiques clé/valeur
#include <QVector> //pour manipuler une collection de zones classées
#include <QColor> //pour stocker la couleur associée au niveau d’urgence dans la structure ZoneUrgence
#include <QSqlError> //pour afficher l’erreur SQL si une requête échoue
#include <QDebug> //pour faire du debug et afficher les erreurs.


class PointCollecte
{
private:
    int id_pc;
    QString zone;
    QString adresse;
    int niveau;
    QString derniere_collecte;
    QString type_zone;


public:

    //// Structure contenant le résultat du métier de prédiction de saturation
    struct PredictionResult
    {
        double tauxParJour; //vitesse estimée de remplissage
        int joursRestants;
        QString dateEstimee;
        bool fiable; //si la prédiction est jugée fiable ou non
    };

    ////// Structure contenant le résultat du métier de classification d’urgence par zone
    struct ZoneUrgence
    {
        QString zone;
        int niveauMax; //Le niveau maximum trouvé dans cette zone.
        int urgenceValeur; //Valeur numérique de l’urgence 1 = faible 2 = moyenne 3 = élevée 4 = critique
        QString urgenceTexte;
        QString interpretation; //Phrase qui explique la situation.
        QString action; //Action recommandée.
        QColor couleur;
    };

    //Cette fonction classe chaque zone selon son niveau d’urgence Elle retourne un QVector d’objets ZoneUrgence
    QVector<ZoneUrgence> classerZonesParUrgence();

    // Constructeurs de la classe PointCollecte
    PointCollecte();
    PointCollecte(int, QString, QString, int, QString, QString);

    // CRUD
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(int);
    bool modifier(int id);

    // Recherche des points de collecte par texte (zone/adresse)
    QSqlQueryModel* rechercher(QString texte);

    // Tri des points de collecte selon leur priorité de remplissage
    QSqlQueryModel* trierParPriorite(QString priorite);

    // statistiques
    int totalPoints();
    int totalUrgents();
    double remplissageMoyen();
    int collectesCetteSemaine();
    QMap<QString, double> moyenneParZone();
    QMap<QString, int> repartitionParPriorite();

    // Prédit la date de saturation d’un point de collecte à partir de son historique
    PredictionResult predireSaturation(int id_pc);
    //modifier le niveau mesuré par Arduino
    bool modifierNiveauArduino(int id, int niveau);

};

#endif // POINTCOLLECTE_H
