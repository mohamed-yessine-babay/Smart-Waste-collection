#include "recyclage.h"
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QSqlDatabase>

Recyclage::Recyclage() {
    id_conteneur = 0; id_machine = 0; quantite = 0.0;
    etat = "En cours";
}
// Constructeur pour l'enregistrement initial
Recyclage::Recyclage(int id_c, int id_m, double qte) {
    this->id_conteneur = id_c;
    this->id_machine = id_m;
    this->quantite = qte;

    // Valeurs par défaut fixées par ton métier
    this->etat = "En cours";
    this->date_debut = QDateTime::currentDateTime();

    // Initialisation des estimations à 0 (avant calcul par tes fonctions)
    this->rendement = 0.0;
    this->quantite_sortie = 0.0;
    this->date_fin = QDateTime(); // Sera calculé après
}
bool Recyclage::ajouterRecyclage(){
    QSqlQuery query;

    // Utilise CURRENT_TIMESTAMP au lieu de :date_d
    // Cela garantit une précision à la microseconde, impossible d'avoir un doublon
    query.prepare("INSERT INTO RECYCLAGE (ID_CONTENEUR_SOURCE, ID_MACHINE, QUANTITE, ETAT, DATE_DEBUT, RENDEMENT, QUANTITE_SORTIE) "
                  "VALUES (:id_c, :id_m, :qte, :etat, CURRENT_TIMESTAMP, 0, 0)");

    query.bindValue(":id_c", this->id_conteneur);
    query.bindValue(":id_m", this->id_machine);
    query.bindValue(":qte", this->quantite);
    query.bindValue(":etat", this->etat);

    // On ne bind plus la date ici, Oracle s'en occupe tout seul avec CURRENT_TIMESTAMP

    if(!query.exec()){
        qDebug() << "ERREUR INSERTION :" << query.lastError().text();
        return false;
    }
    return true;
}

QStandardItemModel* Recyclage::afficherHistorique() {
    QStandardItemModel *model = new QStandardItemModel();
    model->setColumnCount(8);

    // En-têtes (Noms propres pour l'affichage utilisateur)
    model->setHeaderData(0, Qt::Horizontal, "Conteneur Source");
    model->setHeaderData(1, Qt::Horizontal, "Machine");
    model->setHeaderData(2, Qt::Horizontal, "Quantité (kg)");
    model->setHeaderData(3, Qt::Horizontal, "État");
    model->setHeaderData(4, Qt::Horizontal, "Début");
    model->setHeaderData(5, Qt::Horizontal, "Rendement");
    model->setHeaderData(6, Qt::Horizontal, "Quantité Sortie");
    model->setHeaderData(7, Qt::Horizontal, "Fin");

    // REQUÊTE CORRIGÉE : Jointure entre RECYCLAGE et MACHINES
    QSqlQuery query;
    query.prepare("SELECT R.ID_CONTENEUR_SOURCE, R.ID_MACHINE, R.QUANTITE, R.ETAT, "
                  "R.DATE_DEBUT, R.RENDEMENT, R.QUANTITE_SORTIE, R.DATE_FIN, M.TYPE_DECHET "
                  "FROM RECYCLAGE R "
                  "JOIN MACHINES M ON R.ID_MACHINE = M.ID_MACHINE "
                  "ORDER BY R.DATE_DEBUT DESC");

    if (!query.exec()) {
        qDebug() << "Erreur SQL Historique:" << query.lastError().text();
        return model;
    }

    int row = 0;
    while (query.next()) {
        // Extraction (Attention à l'ordre des index SQL)
        QString idC   = query.value(0).toString();
        QString idM   = query.value(1).toString();
        float qte     = query.value(2).toFloat();
        QString etat  = query.value(3).toString();
        QDateTime dtD = query.value(4).toDateTime();
        float rend    = query.value(5).toFloat();
        float qteS    = query.value(6).toFloat();
        QDateTime dtF = query.value(7).toDateTime();
        QString typeD = query.value(8).toString();

        // Colonnes standards
        model->setItem(row, 0, new QStandardItem(idC));
        model->setItem(row, 1, new QStandardItem(idM));
        model->setItem(row, 2, new QStandardItem(QString::number(qte, 'f', 2)));
        model->setItem(row, 3, new QStandardItem(etat));
        model->setItem(row, 4, new QStandardItem(dtD.toString("dd/MM/yyyy HH:mm")));

        // LOGIQUE D'ESTIMATION
        if (dtF.isNull()) {
            // Appel de ta fonction d'estimation
            EstimationResult est = calculerEstimation(qte, dtD, typeD);

            // État spécifique
            model->item(row, 3)->setText("En cours...");
            model->item(row, 3)->setForeground(QBrush(Qt::darkGreen));

            // Rendement (Tiret ou estimation)
            float pourcentageRendement = (est.quantiteSortie / qte) * 100;
            QStandardItem *itemRendEstime = new QStandardItem(QString::number(pourcentageRendement, 'f', 1) + " % (est.)");
            itemRendEstime->setForeground(QBrush(Qt::darkGreen));
            model->setItem(row, 5, itemRendEstime);

            // Date de fin estimée en VERT
            QStandardItem *itemFin = new QStandardItem(est.dateFin.toString("dd/MM/yyyy HH:mm"));
            itemFin->setForeground(QBrush(Qt::darkGreen));
            itemFin->setFont(QFont("Arial", -1, QFont::Bold));
            model->setItem(row, 7, itemFin);
        }
        else {
            // CAS RÉEL (TERMINÉ)
            model->setItem(row, 5, new QStandardItem(QString::number(rend, 'f', 2)));
            model->setItem(row, 6, new QStandardItem(QString::number(qteS, 'f', 2)));
            model->setItem(row, 7, new QStandardItem(dtF.toString("dd/MM/yyyy HH:mm")));
        }

        row++;
    }

    return model;
}

bool Recyclage::supprimerRecyclage(int id_c, int id_m, QString date_d){
    QSqlQuery query;
    // On transforme la date de la base en texte (HEURES:MINUTES) pour comparer avec le tableau
    query.prepare("DELETE FROM RECYCLAGE "
                  "WHERE ID_CONTENEUR_SOURCE = :id_c "
                  "AND ID_MACHINE = :id_m "
                  "AND TO_CHAR(DATE_DEBUT, 'DD/MM/YYYY HH24:MI') = :date_d");

    query.bindValue(":id_c", id_c);
    query.bindValue(":id_m", id_m);
    // On s'assure que date_d ne contient pas de secondes (on prend les 16 premiers caractères)
    query.bindValue(":date_d", date_d.left(16));

    if (!query.exec()) {
        qDebug() << "Erreur SQL suppression :" << query.lastError().text();
        return false;
    }

    // Vérifie si une ligne a vraiment été touchée
    return (query.numRowsAffected() > 0);
}

int Recyclage::verifierTout(int id_c, int id_m, double qte_saisie) {
    QSqlQuery query;
    QString typeC, typeM;
    double stockC, capaciteM;

    // 1. On récupère TOUT sur le conteneur
    query.prepare("SELECT TYPE, QUANTITE_ACTUELLE FROM CONTENEURS WHERE ID_CONTENEUR = :id");
    query.bindValue(":id", id_c);
    if(query.exec() && query.next()) {
        typeC = query.value(0).toString().trimmed();
        stockC = query.value(1).toDouble();
    } else return 0; // Code 0 : ID Conteneur n'existe pas

    // 2. On récupère TOUT sur la machine
    query.prepare("SELECT TYPE_DECHET, CAPACITE FROM MACHINES WHERE ID_MACHINE = :id");
    query.bindValue(":id", id_m);
    if(query.exec() && query.next()) {
        typeM = query.value(0).toString().trimmed();
        capaciteM = query.value(1).toDouble();
    } else return -1; // Code -1 : ID Machine n'existe pas

    // 3. LES COMPARAISONS (L'ordre est important)

    // A. Compatibilité des types
    if(typeC.toLower() != typeM.toLower()) return 2;

    // B. Capacité de la machine
    if(qte_saisie > capaciteM) return 3;

    // C. Stock du conteneur
    if(qte_saisie > stockC) return 4;

    return 1; // 1 = Tout est parfait !
}

EstimationResult Recyclage::calculerEstimation(float quantiteEntree, QDateTime dateDebut, QString typeDechet) {
    EstimationResult res;

    // Valeurs par défaut
    float vitesse = 1.2;
    float coefRendement = 0.90;

    // Nettoyage de la chaîne pour éviter les erreurs de casse
    QString type = typeDechet.toUpper().trimmed();

    // Application des règles selon le type
    if (type == "PLASTIQUE") {
        vitesse = 2.0;       // 2 min par kg
        coefRendement = 0.92; // 8% de perte
    }
    else if (type == "VERRE") {
        vitesse = 1.5;
        coefRendement = 0.98;
    }
    else if (type == "PAPIER") {
        vitesse = 1.0;
        coefRendement = 0.85;
    }
    else if (type == "METAL") {
        vitesse = 3.0;
        coefRendement = 0.95;
    }

    // Calculs
    res.dureeMinutes = static_cast<int>(quantiteEntree * vitesse);
    res.dateFin = dateDebut.addSecs(res.dureeMinutes * 60);
    res.quantiteSortie = quantiteEntree * coefRendement;

    return res;
}

bool Recyclage::terminerRecyclage(int id_c, int id_m, QString date_d, float qte_sortie, float qte_entree) {
    // 1. Calcul du rendement
    float rend = (qte_entree > 0) ? (qte_sortie / qte_entree) * 100 : 0;

    QSqlQuery query;
    // 2. La requête UPDATE
    // Utilisation de LIKE pour la date si jamais il y a un décalage de quelques secondes
    query.prepare("UPDATE RECYCLAGE SET "
                  "QUANTITE_SORTIE = :qs, "
                  "RENDEMENT = :rend, "
                  "DATE_FIN = CURRENT_TIMESTAMP, "
                  "ETAT = 'Terminé' "
                  "WHERE ID_CONTENEUR_SOURCE = :id_c "
                  "AND ID_MACHINE = :id_m "
                  "AND TO_CHAR(DATE_DEBUT, 'DD/MM/YYYY HH24:MI') LIKE :date_d");

    query.bindValue(":qs", qte_sortie);
    query.bindValue(":rend", rend);
    query.bindValue(":id_c", id_c);
    query.bindValue(":id_m", id_m);
    // On s'assure que la date envoyée ne contient que Minutes et Heures pour matcher le TO_CHAR
    query.bindValue(":date_d", date_d.left(16) + "%");

    if(!query.exec()) {
        qDebug() << "ERREUR UPDATE SQL :" << query.lastError().text();
        return false;
    }

    // 3. Vérification : est-ce qu'une ligne a vraiment été modifiée ?
    if(query.numRowsAffected() == 0) {
        qDebug() << "AUCUNE LIGNE TROUVÉE pour ID_C:" << id_c << " ID_M:" << id_m << " Date:" << date_d;
        return false;
    }

    return true;
}
