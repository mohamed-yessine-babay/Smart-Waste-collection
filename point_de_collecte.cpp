#include "point_de_collecte.h"
#include <QDate> //pour comparer les dates
#include <QVector> //pour stocker l’historique dans la prédiction et la liste des zones dans la classification.
#include <cmath> //pour utiliser ceil()
#include <QColor>
#include <QSqlError>
#include <QDebug>

PointCollecte::PointCollecte()
{
    id_pc = 0;
    niveau = 0;
}

PointCollecte::PointCollecte(int id, QString z, QString a, int n, QString d, QString t)
{
    id_pc = id;
    zone = z;
    adresse = a;
    niveau = n;
    derniere_collecte = d;
    type_zone = t;
}

bool PointCollecte::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO POINTS_DE_COLLECTE "
                  "(ZONE, ADRESSE, NIVEAU, DERNIERE_COLLECTE, TYPE_ZONE) "
                  "VALUES (:zone, :adresse, :niveau, TRUNC(TO_DATE(:date_collecte, 'YYYY-MM-DD')), :type_zone)");

    query.bindValue(":zone", zone);
    query.bindValue(":adresse", adresse);
    query.bindValue(":niveau", niveau);
    query.bindValue(":date_collecte", derniere_collecte);
    query.bindValue(":type_zone", type_zone);
    if(adresse.isEmpty()){
        QMessageBox::warning(nullptr, "Erreur", "tous les champs doivent être complétés !");
        return false;
    }if(adresse.length()<4){
        QMessageBox::warning(nullptr, "Erreur", "l'adresse est courte !");
        return false;
    }
    QDate dateSaisie = QDate::fromString(derniere_collecte, "yyyy-MM-dd");
    // Obtenir la date actuelle
    QDate dateActuelle = QDate::currentDate();

    if(dateSaisie > dateActuelle){
        QMessageBox::warning(nullptr, "Erreur", "La date ne peut pas être postérieure à aujourd'hui !\n"
                                                "Date maximum autorisée : " + dateActuelle.toString("yyyy-MM-dd"));
        return false;

    }else{
        return query.exec();
    }

}

QSqlQueryModel* PointCollecte::afficher()
{
    QSqlQueryModel* model = new QSqlQueryModel();

    model->setQuery(
        "SELECT ID_PC, ZONE, ADRESSE, NIVEAU, "
        "TO_CHAR(DERNIERE_COLLECTE, 'DD/MM/YYYY') AS DERNIERE_COLLECTE, "
        "TYPE_ZONE "
        "FROM POINTS_DE_COLLECTE"
        );

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Zone"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Adresse"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Niveau"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Dernière Collecte"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Type Zone"));

    return model;
}

bool PointCollecte::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM POINTS_DE_COLLECTE WHERE ID_PC = :id");
    query.bindValue(":id", id);

    return query.exec();
}

bool PointCollecte::modifier(int id)
{
    QSqlQuery query;

    query.prepare("UPDATE POINTS_DE_COLLECTE SET "
                  "ZONE = :zone, "
                  "ADRESSE = :adresse, "
                  "NIVEAU = :niveau, "
                  "DERNIERE_COLLECTE = TRUNC(TO_DATE(:date_collecte, 'YYYY-MM-DD')), "
                  "TYPE_ZONE = :type_zone "
                  "WHERE ID_PC = :id");

    query.bindValue(":zone", zone);
    query.bindValue(":adresse", adresse);
    query.bindValue(":niveau", niveau);
    query.bindValue(":date_collecte", derniere_collecte);
    query.bindValue(":type_zone", type_zone);
    query.bindValue(":id", id);
    if(adresse.isEmpty()){
        QMessageBox::warning(nullptr, "Erreur", "tous les champs doivent être complétés !");
        return false;
    }if(adresse.length()<4){
        QMessageBox::warning(nullptr, "Erreur", "l'adresse est courte !");
        return false;
    }
    QDate dateSaisie = QDate::fromString(derniere_collecte, "yyyy-MM-dd");
    // Obtenir la date actuelle
    QDate dateActuelle = QDate::currentDate();

    if(dateSaisie > dateActuelle){
        QMessageBox::warning(nullptr, "Erreur", "La date ne peut pas être postérieure à aujourd'hui !\n"
                                                "Date maximum autorisée : " + dateActuelle.toString("yyyy-MM-dd"));
        return false;

    }else{
        return query.exec();
    }
}


// Recherche dynamique des points de collecte par zone ou adresse

QSqlQueryModel* PointCollecte::rechercher(QString texte)
{
    QSqlQueryModel* model = new QSqlQueryModel();

    QSqlQuery query;
    query.prepare(
        "SELECT ID_PC, ZONE, ADRESSE, NIVEAU, "
        "TO_CHAR(DERNIERE_COLLECTE, 'DD/MM/YYYY') AS DERNIERE_COLLECTE, "
        "TYPE_ZONE "
        "FROM POINTS_DE_COLLECTE "
        "WHERE LOWER(ZONE) LIKE LOWER(:texte) "
        "OR LOWER(ADRESSE) LIKE LOWER(:texte)"
        );

    query.bindValue(":texte", "%" + texte + "%");
    query.exec();

    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Zone"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Adresse"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Niveau"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Dernière Collecte"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Type Zone"));

    return model;
}


// Trie les points de collecte selon leur priorité de remplissage

QSqlQueryModel* PointCollecte::trierParPriorite(QString priorite)
{
    QSqlQueryModel* model = new QSqlQueryModel();
    QSqlQuery query;

    QString baseQuery =
        "SELECT ID_PC, ZONE, ADRESSE, NIVEAU, "
        "TO_CHAR(DERNIERE_COLLECTE, 'DD/MM/YYYY') AS DERNIERE_COLLECTE, "
        "TYPE_ZONE "
        "FROM POINTS_DE_COLLECTE ";

    if (priorite == "Priorité: Urgente")
    {
        // Tous les points, mais les urgents d'abord
        query.prepare(baseQuery +
                      "ORDER BY "
                      "CASE "
                      "WHEN NIVEAU >= 80 THEN 1 "
                      "ELSE 2 "
                      "END, "
                      "NIVEAU DESC");
    }
    else if (priorite == "Priorité: Basse")
    {
        // Tous les points, mais les basses d'abord
        query.prepare(baseQuery +
                      "ORDER BY "
                      "CASE "
                      "WHEN NIVEAU < 50 THEN 1 "
                      "ELSE 2 "
                      "END, "
                      "NIVEAU ASC");
    }
    else
    {
        // Tous les points, tri normal
        query.prepare(baseQuery +
                      "ORDER BY NIVEAU DESC");
    }

    query.exec();
    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Zone"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Adresse"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Niveau"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Dernière Collecte"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Type Zone"));

    return model;
}

int PointCollecte::totalPoints()
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM POINTS_DE_COLLECTE");

    if (query.exec() && query.next())
        return query.value(0).toInt();

    return 0;
}

//Cette fonction permet d’identifier combien de points nécessitent une attention rapide.

int PointCollecte::totalUrgents()
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM POINTS_DE_COLLECTE WHERE NIVEAU >= 80");

    if (query.exec() && query.next())
        return query.value(0).toInt();

    return 0;
}

// Elle calcule le taux moyen de remplissage global.

double PointCollecte::remplissageMoyen()
{
    QSqlQuery query;
    query.prepare("SELECT NVL(AVG(NIVEAU), 0) FROM POINTS_DE_COLLECTE");

    if (query.exec() && query.next())
        return query.value(0).toDouble();

    return 0;
}

int PointCollecte::collectesCetteSemaine()
{
    QSqlQuery query;
    query.prepare(
        "SELECT COUNT(*) "
        "FROM POINTS_DE_COLLECTE "
        "WHERE TRUNC(DERNIERE_COLLECTE) >= TRUNC(SYSDATE, 'IW') "
        "AND TRUNC(DERNIERE_COLLECTE) < TRUNC(SYSDATE, 'IW') + 7"
        );

    if (query.exec() && query.next())
        return query.value(0).toInt();

    return 0;
}

//Retourner une moyenne de remplissage pour chaque zone

QMap<QString, double> PointCollecte::moyenneParZone()
{
    QMap<QString, double> data;
    QSqlQuery query;

    query.prepare(
        "SELECT ZONE, NVL(AVG(NIVEAU), 0) AS MOYENNE "
        "FROM POINTS_DE_COLLECTE "
        "GROUP BY ZONE "
        "ORDER BY ZONE"
        );

    if (query.exec())
    {
        while (query.next())
        {
            QString zone = query.value(0).toString();
            double moyenne = query.value(1).toDouble();
            data[zone] = moyenne;
        }
    }

    return data;
}

//Donner le nombre de points par catégorie de priorité.

QMap<QString, int> PointCollecte::repartitionParPriorite()
{
    QMap<QString, int> data;
    data["Basse"] = 0;
    data["Normale"] = 0;
    data["Urgente"] = 0;

    QSqlQuery query;
    query.prepare(
        "SELECT "
        "CASE "
        "WHEN NIVEAU < 50 THEN 'Basse' "
        "WHEN NIVEAU >= 50 AND NIVEAU < 80 THEN 'Normale' "
        "ELSE 'Urgente' "
        "END AS PRIORITE, "
        "COUNT(*) "
        "FROM POINTS_DE_COLLECTE "
        "GROUP BY "
        "CASE "
        "WHEN NIVEAU < 50 THEN 'Basse' "
        "WHEN NIVEAU >= 50 AND NIVEAU < 80 THEN 'Normale' "
        "ELSE 'Urgente' "
        "END"
        );

    if (query.exec())
    {
        while (query.next())
        {
            QString priorite = query.value(0).toString();
            int nb = query.value(1).toInt();
            data[priorite] = nb;
        }
    }

    return data;
}

// ==================== Prédiction ====================
// Estimation du taux journalier et de la date probable de saturation

PointCollecte::PredictionResult PointCollecte::predireSaturation(int id_pc)
{

    //initialiser le résultat
    PredictionResult result;
    result.tauxParJour = 0;
    result.joursRestants = 0;
    result.dateEstimee = "--";
    result.fiable = false;
    QSqlQuery query;


    // 1) récupérer le niveau actuel
    query.prepare("SELECT NIVEAU FROM TEAM17.POINTS_DE_COLLECTE WHERE ID_PC = :id");
    query.bindValue(":id", id_pc);

    double niveauActuel = 0;

    if (query.exec() && query.next())
    {
        niveauActuel = query.value(0).toDouble();
    }
    else
    {
        return result;
    }

    // 2) récupérer historique RECUPERER trié par date
    query.prepare("SELECT DATE_REMP, QUANTITE "
                  "FROM TEAM17.RECUPERER "
                  "WHERE ID_PC = :id "
                  "ORDER BY DATE_REMP ASC");
    query.bindValue(":id", id_pc);

    QVector<QDate> dates;
    QVector<double> quantites;

    if (query.exec())
    {
        while (query.next())
        {
            dates.append(query.value(0).toDate());
            quantites.append(query.value(1).toDouble());
        }
    }
    // 3) vérifier si assez de données
    if (dates.size() < 2)
    {
        result.tauxParJour = 5; // fallback
        result.fiable = false;
    }
    else
    {
        double totalRate = 0.0;
        int nbIntervallesValides = 0;

        for (int i = 1; i < dates.size(); i++)
        {
            double deltaQ = quantites[i] - quantites[i - 1];
            int deltaDays = dates[i - 1].daysTo(dates[i]);

            // On garde seulement les progressions positives et les dates valides
            if (deltaQ > 0 && deltaDays > 0)
            {
                totalRate += (deltaQ / deltaDays);
                nbIntervallesValides++;
            }
        }

        if (nbIntervallesValides > 0)
        {
            double taux = totalRate / nbIntervallesValides;

            // bornes de sécurité
            if (taux < 1) taux = 1;
            if (taux > 20) taux = 20;

            result.tauxParJour = taux;
            result.fiable = true;
        }
        else
        {
            result.tauxParJour = 5; // fallback si données incohérentes
            result.fiable = false;
        }
    }

    // 4) calcul jours restants
    double reste = 100 - niveauActuel;

    if (result.tauxParJour > 0)
        result.joursRestants = ceil(reste / result.tauxParJour);
    else
        result.joursRestants = 0;

    // 5) date estimée
    QDate dateEstimee = QDate::currentDate().addDays(result.joursRestants);
    result.dateEstimee = dateEstimee.toString("dd/MM/yyyy");
    return result;
}

// ==================== Classification urgence ====================
// Classement des zones selon le niveau maximal de remplissage

QVector<PointCollecte::ZoneUrgence> PointCollecte::classerZonesParUrgence()
{
    QVector<ZoneUrgence> zones;
    QSqlQuery query;

    query.prepare(
        "SELECT ZONE, MAX(NIVEAU) AS MAX_NIVEAU " //Pour chaque zone, tu prends le niveau maximum.
        "FROM POINTS_DE_COLLECTE "
        "GROUP BY ZONE "
        "ORDER BY ZONE"
        );

    if (!query.exec())
    {
        qDebug() << "Erreur classerZonesParUrgence :" << query.lastError().text();
        return zones;
    }

    while (query.next())
    {
        ZoneUrgence z;
        z.zone = query.value("ZONE").toString();
        z.niveauMax = query.value("MAX_NIVEAU").toInt();

        if (z.niveauMax >= 90)
        {
            z.urgenceValeur = 4;
            z.urgenceTexte = "4 - Critique";
            z.interpretation = "Risque élevé de débordement";
            z.action = "Intervention immédiate";
            z.couleur = QColor(220, 53, 69);
        }
        else if (z.niveauMax >= 70)
        {
            z.urgenceValeur = 3;
            z.urgenceTexte = "3 - Élevée";
            z.interpretation = "Plusieurs points à risque";
            z.action = "Collecte recommandée aujourd'hui";
            z.couleur = QColor(242, 140, 40);
        }
        else if (z.niveauMax >= 50)
        {
            z.urgenceValeur = 2;
            z.urgenceTexte = "2 - Moyenne";
            z.interpretation = "Certains points nécessitent une attention";
            z.action = "Collecte planifiée";
            z.couleur = QColor(241, 181, 0);
        }
        else
        {
            z.urgenceValeur = 1;
            z.urgenceTexte = "1 - Faible";
            z.interpretation = "Niveau acceptable";
            z.action = "Surveillance normale";
            z.couleur = QColor(76, 175, 80);
        }

        zones.append(z);
    }

    return zones;
}

bool PointCollecte::modifierNiveauArduino(int id, int niveau)
{
    QSqlQuery query;

    query.prepare("UPDATE POINTS_DE_COLLECTE "
                  "SET NIVEAU = :niveau "
                  "WHERE ID_PC = :id");

    query.bindValue(":niveau", niveau);
    query.bindValue(":id", id);

    if (!query.exec())
    {
        qDebug() << "Erreur update niveau Arduino :" << query.lastError().text();
        return false;
    }

    return true;
}
